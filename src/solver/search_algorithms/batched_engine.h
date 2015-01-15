
#ifndef UltraSolver_batched_engine_h
#define UltraSolver_batched_engine_h

#include "queued_engine.h"
#include <thread>
#include <future>

template<class E>
struct batched_priority_cmp
{
	typedef E element_t;

	bool operator()(const element_t & lhs, const element_t & rhs) const
	{
		return (get<1>(lhs) + get<0>(lhs)) < (get<1>(rhs) + get<0>(rhs));
	}
};

template<typename N, typename H, bool ExtMemory>
class batched_engine : public queued_search_engine<N, float, batched_priority_cmp, ExtMemory>
{
	typedef batched_engine<N, H, ExtMemory> _Self;
	typedef queued_search_engine<N, float, batched_priority_cmp, ExtMemory> _Base;
	typedef float estimation_t;
	typedef H heuristic_t;
	typedef typename _Base::state_t state_t;

	typedef std::pair<size_t, search_node_t> expanded_node_t;
	typedef std::vector<expanded_node_t> expanded_nodes_container_t;
public:
	template<typename Tr>
	batched_engine(const Tr & transition_system, size_t batch_size = 800)
		:_Base(transition_system), m_batchSize(batch_size)
	{}

	template<typename GraphT>
	bool operator()(GraphT & graph, state_t init_node, state_t goal_state, std::vector<state_t> & solution_path)
	{
		heuristic_t h_fun(graph.transition_system());

		enqueue(create_node(init_node, 0), std::numeric_limits<float>::max());

		float best_estimation = std::numeric_limits<float>::max();
		comparison_t current_data;

		while((!m_searchQueue.empty()) && m_goalNodes.empty())
		{
			//Extract batch of best states

			pick_best_nodes(m_inBuffer, m_batchSize, &current_data);

			if(get<0>(current_data) < best_estimation)
			{
				best_estimation = get<0>(current_data);
				cout << "Best heuristic: " << best_estimation << std::endl;
			}
			
			expand_best_nodes(graph);

			estimate_expand_buffer([&](const state_t & state){
				return h_fun(state);
			});

			merge(goal_state);
		}

		m_finished = true;

		if(!m_goalNodes.empty())
			solution_path = backtrace_path(m_goalNodes[0]);

		return !m_goalNodes.empty();
	}
private:
	
	void pick_best_nodes(std::vector<search_node_t> & res, size_t max_count, comparison_t * first_node_data = nullptr)
	{
		res.reserve(max_count);

		size_t picked_count = m_searchQueue.top(res, max_count, false);
		m_searchQueue.pop(picked_count);
	}

	template<typename GraphT, typename It>
	expanded_nodes_container_t expand_node_range(const GraphT & graph, It begin, It end)
	{
		expanded_nodes_container_t res;
	
		for(auto it = begin; it != end; ++it)
		{
			forall_adj_vertices<true>(graph, get<2>(*it), [&](const state_t & state){
				res.push_back(expanded_node_t(m_hasher(state), search_node_t(-1, get<0>(*it), state, get<3>(*it) + 1)));
			});
		}

		return std::move(res);
	}

	template<typename GraphT>
	void expand_best_nodes(GraphT & graph)
	{
		//cout << "(" << m_inBuffer.size() << " nodes)... " << std::endl;

		//If node count < 20 - do it single-thread to avoid ping-pong effect
		if(m_inBuffer.size() < 20)
			m_expandBuffer = std::move(expand_node_range(graph, m_inBuffer.begin(), m_inBuffer.end()));
		else
		{
			int grain_count = std::thread::hardware_concurrency();
			size_t grain_size = m_inBuffer.size() / grain_count;

			vector<future<expanded_nodes_container_t>> node_groups;

			for(int i = 0; i < grain_count - 1; ++i)
			{
				node_groups.push_back(async(launch::async, [=](){
						return expand_node_range(graph, m_inBuffer.begin() + i * grain_size, m_inBuffer.begin() + (i+1) * grain_size);
					}));
			}
			node_groups.push_back(async(launch::async, [=](){
					return expand_node_range(graph, m_inBuffer.begin() + (grain_count-1) * grain_size, m_inBuffer.end());
				}));

			for(auto & ng : node_groups)
			{
				auto gr = ng.get();
				m_expandBuffer.insert(m_expandBuffer.end(), gr.begin(), gr.end());
			}
		}

		m_inBuffer.clear();

		//Remove duplications

		//Sort (parallel) by hash
		std::sort(m_expandBuffer.begin(), m_expandBuffer.end(), [](const expanded_node_t & n1, const expanded_node_t & n2){
			return n1.first < n2.first;
		});
		
		//Apply internal merge
	}

	template<typename EstFun, typename NodeIt, typename EstIt>
	void estimate_node_range(EstFun e_fun, NodeIt n_begin, NodeIt n_end, EstIt e_begin, EstIt e_end)
	{
		auto e_it = e_begin;
		for(auto n_it = n_begin; n_it != n_end; ++n_it, ++e_it)
			*e_it = e_fun(get<2>((*n_it).second));
	}

	template<typename EstFun>
	void estimate_expand_buffer(EstFun e_fun)
	{
		m_estimationBuffer.resize(m_expandBuffer.size(), -1.0f);

		if(m_expandBuffer.size() < 20)
			estimate_node_range(e_fun, m_expandBuffer.begin(), m_expandBuffer.end(), m_estimationBuffer.begin(), m_estimationBuffer.end());
		else
		{
			int grain_count = std::thread::hardware_concurrency();
			size_t grain_size = m_expandBuffer.size() / grain_count;

			vector<future<void>> estimations;
			estimations.reserve(grain_count);

			for(int i = 0; i < grain_count - 1; ++i)
			{
				estimations.push_back(async(launch::async, [=](){
						//cout << "Estimating for " << std::distance(m_expandBuffer.begin(), node_it) << "-" << std::distance(m_expandBuffer.begin(), node_it + grain_size) << std::endl;
						return estimate_node_range(e_fun, m_expandBuffer.begin() + i * grain_size, m_expandBuffer.begin() + (i+1) * grain_size, m_estimationBuffer.begin() + i * grain_size, m_estimationBuffer.begin() + (i+1) * grain_size);
					}));
			}

			estimations.push_back(async(launch::async, [=](){
					return estimate_node_range(e_fun, m_expandBuffer.begin() + (grain_count - 1) * grain_size, m_expandBuffer.end(), m_estimationBuffer.begin() + (grain_count - 1) * grain_size, m_estimationBuffer.end());
				}));

			for(auto & est : estimations)
				est.get();
		}
	}

	void merge(const state_t & goal_state)
	{
		int len = m_expandBuffer.size();

		for(int i = 0; i < len; ++i)
		{
			auto res = m_database.add(get<2>(m_expandBuffer[i].second));
			if (res)
			{
				m_expandBuffer[i].second = create_node(get<2>(m_expandBuffer[i].second), get<1>(m_expandBuffer[i].second));

				if (get<2>(m_expandBuffer[i].second) == goal_state)
					m_goalNodes.push_back(m_expandBuffer[i].second);
				else
					enqueue(m_expandBuffer[i].second, m_estimationBuffer[i]);
			}
		}

		m_expandBuffer.clear();
		m_estimationBuffer.clear();
	}

private:
	size_t m_batchSize;
	std::vector<search_node_t> m_inBuffer;
	expanded_nodes_container_t m_expandBuffer; //Node + hash
	std::vector<float> m_estimationBuffer;

	std::hash<state_t> m_hasher;
};

#endif
