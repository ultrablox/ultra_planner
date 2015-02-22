
#ifndef UltraSolver_batched_engine_h
#define UltraSolver_batched_engine_h

#include "queued_engine.h"
#include <core/algorithm/merge.h>
#include <thread>
#include <future>

template<class E>
struct batched_priority_cmp
{
	typedef E element_t;

	bool operator()(const node_estimation_t & lhs, const node_estimation_t & rhs) const
	{
		return lhs.total_estimation() < rhs.total_estimation();
	}
};

template<typename Gr, typename H, bool ExtMemory>
class batched_engine : public queued_search_engine<Gr, batched_priority_cmp, ExtMemory, hashset_t::Delayed>
{
	typedef batched_engine<Gr, H, ExtMemory> _Self;
	typedef queued_search_engine<Gr, batched_priority_cmp, ExtMemory, hashset_t::Delayed> _Base;
	typedef float estimation_t;
	typedef H heuristic_t;
	typedef typename _Base::state_t state_t;

	using search_node_t = typename _Base::search_node_t;
	//using comparison_t = typename _Base::comparison_t;
	typedef std::tuple<size_t, search_node_t/*, estimation_t*/> expanded_node_t;
	typedef std::vector<expanded_node_t> expanded_nodes_container_t;
public:
	//template<typename Tr>
	batched_engine(const typename Gr::vertex_streamer_t & vstreamer, size_t batch_size = 1000)
		:_Base(vstreamer), m_batchSize(batch_size)
	{}

	template<typename GraphT, typename IsGoalFun>
	bool operator()(GraphT & graph, const state_t & init_node, IsGoalFun is_goal_fun, std::vector<state_t> & solution_path, float * p_plan_cost = nullptr)
	{
		heuristic_t h_fun(graph.transition_system());

		this->enqueue(is_goal_fun, this->create_node(init_node, 0), node_estimation_t(0, h_fun(init_node)));

		float best_estimation = std::numeric_limits<float>::max();
		//comparison_t current_data;

		while((!_Base::m_searchQueue.empty()) && _Base::m_goalNodes.empty())
		{
			//Extract batch of best states

			pick_best_nodes(m_inBuffer, m_batchSize, 0/*&current_data*/);

			/*if(get<0>(current_data) < best_estimation)
			{
				best_estimation = get<0>(current_data);
				cout << "Best heuristic: " << best_estimation << std::endl;
			}*/
			
			expand_best_nodes(graph);

			/*estimate_expand_buffer([&](const state_t & state){
				return h_fun(state);
			});*/

			merge(is_goal_fun, [&](const state_t & state){
				return h_fun(state);
			});
		}

		_Base::m_finished = true;

		if(!_Base::m_goalNodes.empty())
			solution_path = this->backtrace_path(_Base::m_goalNodes[0]);

		return !_Base::m_goalNodes.empty();
	}
private:
	
	void pick_best_nodes(std::vector<search_node_t> & res, size_t max_count, node_estimation_t * first_node_data = nullptr)
	{
		res.reserve(max_count);

		/*size_t picked_count = _Base::m_searchQueue.top(res, max_count, false, first_node_data);
		_Base::m_searchQueue.pop(picked_count);*/
		_Base::m_searchQueue.pop_and_call([&](const node_estimation_t & prior, const search_node_t & node){
			res.push_back(node);
		}, max_count);
	}

	template<typename GraphT, typename It>
	expanded_nodes_container_t expand_node_range(const GraphT & graph, It begin, It end)
	{
		expanded_nodes_container_t res;
	
		for(auto it = begin; it != end; ++it)
		{
			/*forall_adj_vertices<true>(graph, get<2>(*it), [&](const state_t & state){
				res.push_back(expanded_node_t(m_hasher(state), search_node_t(-1, get<0>(*it), state, get<3>(*it) + 1), -1.0f));
			});*/

			graph.forall_adj_verts(it->state, [&](const state_t & state, float transition_cost){
				res.push_back(expanded_node_t(m_hasher(state), search_node_t(-1, it->id, state, it->length + transition_cost)/*, -1.0f*/));
			});
		}

		return std::move(res);
	}

	template<typename GraphT>
	void expand_best_nodes(GraphT & graph)
	{
		//If node count < 20 - do it single-thread to avoid ping-pong effect
		/*if(m_inBuffer.size() < 20)
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
		}*/
		m_expandBuffer = std::move(expand_node_range(graph, m_inBuffer.begin(), m_inBuffer.end()));

		m_inBuffer.clear();

		//Remove duplications

		//Sort (parallel) by hash
		sort_wrapper(m_expandBuffer.begin(), m_expandBuffer.end(), [](const expanded_node_t & n1, const expanded_node_t & n2){
			return get<0>(n1) < get<0>(n2);
		});
		
		//Apply internal merge
		auto last_it = UltraCore::unique(m_expandBuffer.begin(), m_expandBuffer.end(), [](const expanded_node_t & node){
			return get<1>(node);
		});
		m_expandBuffer.erase(last_it, m_expandBuffer.end());

	}

	template<typename EstFun, typename NodeIt>
	void estimate_node_range(EstFun e_fun, NodeIt n_begin, NodeIt n_end)
	{
		for(auto n_it = n_begin; n_it != n_end; ++n_it)
			get<2>(*n_it) = e_fun(get<2>(get<1>(*n_it)));
	}

	template<typename EstFun>
	void estimate_expand_buffer(EstFun e_fun)
	{
//		m_estimationBuffer.resize(m_expandBuffer.size(), -1.0f);

		/*if(m_expandBuffer.size() < 20)
			estimate_node_range(e_fun, m_expandBuffer.begin(), m_expandBuffer.end());
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
						return estimate_node_range(e_fun, m_expandBuffer.begin() + i * grain_size, m_expandBuffer.begin() + (i+1) * grain_size);
					}));
			}

			estimations.push_back(async(launch::async, [=](){
					return estimate_node_range(e_fun, m_expandBuffer.begin() + (grain_count - 1) * grain_size, m_expandBuffer.end());
				}));

			for(auto & est : estimations)
				est.get();
		}*/
		estimate_node_range(e_fun, m_expandBuffer.begin(), m_expandBuffer.end());
	}

	
		template<typename IsGoalFun, typename EstFun>
		void merge(IsGoalFun is_goal, EstFun est_fun)
		{
			_Base::m_database.add_range(m_expandBuffer.begin(), m_expandBuffer.end(), [](const expanded_node_t & exp_node){
				return get<0>(exp_node);
			}, [](const expanded_node_t & exp_node){
				return get<1>(exp_node).state;
			}, [=](const expanded_node_t & expanded_node){
				search_node_t new_node = this->create_node(get<1>(expanded_node).state, get<1>(expanded_node).parent_id);
				this->enqueue(is_goal, new_node, node_estimation_t(new_node.length, est_fun(get<1>(expanded_node).state)));
			});

			m_expandBuffer.clear();
		}

private:
	size_t m_batchSize;
	std::vector<search_node_t> m_inBuffer;
	expanded_nodes_container_t m_expandBuffer; //Node + hash

	std::hash<state_t> m_hasher;
};

#endif
