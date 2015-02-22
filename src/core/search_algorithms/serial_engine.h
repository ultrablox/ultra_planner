
#ifndef UltraSolver_serial_engine_h
#define UltraSolver_serial_engine_h

#include "queued_engine.h"

template<typename Gr, template<typename> class Cmp, bool ExtMemory = false, hashset_t HT = hashset_t::Internal>
class blind_engine : public queued_search_engine<Gr, Cmp, false, HT>
{
	using graph_t = Gr;
	using _Base = queued_search_engine<Gr, Cmp, false, HT>;
	using state_t = typename _Base::state_t;
//	using comparison_t = typename _Base::comparison_t;
	using search_node_t = typename _Base::search_node_t;

public:
	//template<typename Gr>
	blind_engine(const typename graph_t::vertex_streamer_t & vstreamer)
		:_Base(vstreamer)
	{}

	//Returns true, if found.
	//template<typename GraphT>
	bool operator()(const graph_t & graph, const state_t & init_node, std::function<bool(const state_t & node)> goal_check_fun, std::vector<state_t> & solution_path)
	{
		_Base::enqueue(goal_check_fun, _Base::create_node(init_node, 0), node_estimation_t(0, 0));

		int step = 0;
		while ((!this->m_searchQueue.empty()) && this->m_goalNodes.empty())
		{
			//Pop node for expansion and mark as expanded
			search_node_t cur_node = this->dequeue();


			graph.forall_adj_verts(cur_node.state, [=](const state_t & state, float transition_cost){
				//Check that node is not expanded or discovered by trying to add
				if(this->m_database.add(state))
				{
					search_node_t new_node = this->m_database.create_node(state, cur_node.id, cur_node.length + transition_cost);
					this->enqueue(goal_check_fun, new_node, node_estimation_t(cur_node.length + transition_cost, 0));
				}
			});
			++step;
		}

		return this->finalize(solution_path);
	}

};

template<typename Gr, template<typename> class Cmp, typename H, bool ExtMemory, hashset_t StorageType>
class heuristic_engine : public queued_search_engine<Gr, Cmp, ExtMemory, StorageType>
{
	//using priority_t = template<typename> Cmp;
	using graph_t = Gr;
	typedef float estimation_t;
	typedef queued_search_engine<Gr, Cmp, ExtMemory, StorageType> _Base;
	typedef H heuristic_t;
	using search_node_t = typename _Base::search_node_t;
	using state_t = typename _Base::state_t;
	//using comparison_t = typename _Base::comparison_t;
public:
	//template<typename Gr>
	heuristic_engine(const typename graph_t::vertex_streamer_t & vstreamer)
		:_Base(vstreamer)
	{}

	template<typename IsGoalFun>
	bool operator()(const graph_t & graph, const state_t & init_state, IsGoalFun is_goal_fun, std::vector<state_t> & solution_path, float * p_plan_cost = nullptr)
	{
		heuristic_t h_fun(graph.transition_system());
		heuristic_t * p_fun = &h_fun;

		this->enqueue(is_goal_fun, this->create_node(init_state, 0), node_estimation_t(0, h_fun(init_state)));
		this->m_database.add(init_state, [=](const state_t & state){});

		float best_data = std::numeric_limits<float>::max();
		//comparison_t current_data;

		while ((!_Base::m_searchQueue.empty()) && _Base::m_goalNodes.empty())
		{
			search_node_t cur_node = this->dequeue(/*&current_data*/);

			graph.forall_adj_verts(cur_node.state, [=](const state_t & adjacent_state, float transition_cost){			
				if(this->m_database.add(adjacent_state))
				{
					float estimation = (*p_fun)(adjacent_state);
#if _DEBUG
					assert(estimation >= 0.0f);
#endif
					if (estimation != std::numeric_limits<float>::max())
						this->enqueue(is_goal_fun, this->m_database.create_node(adjacent_state, cur_node.id, cur_node.length + transition_cost), node_estimation_t(cur_node.length + transition_cost, estimation));
				}
			});
		}

		if (p_plan_cost && !_Base::m_goalNodes.empty())
			*p_plan_cost = _Base::m_goalNodes[0].length;

#if TRACE_SOLUTION
		int i = 0;
		return this->finalize(solution_path, [&](const search_node_t & node){
			cout << i++ << ". " << "cost: " << node.length << ", est: " << h_fun(node.state) << std::endl;
		});
#else
		return this->finalize(solution_path);
#endif
	}
};

template<typename Gr, template<typename> class Cmp, typename H, bool ExtMemory, hashset_t StorageType>
class buffered_heuristic_engine : public queued_search_engine<Gr, Cmp, ExtMemory, StorageType>
{
	using _Base = queued_search_engine<Gr, Cmp, ExtMemory, StorageType>;
	using graph_t = Gr;
	typedef H heuristic_t;
	using search_node_t = typename _Base::search_node_t;
	using state_t = typename _Base::state_t;
public:
	buffered_heuristic_engine(const typename graph_t::vertex_streamer_t & vstreamer)
		:_Base(vstreamer)
	{}

	template<typename IsGoalFun>
	bool operator()(const graph_t & graph, const state_t & init_state, IsGoalFun is_goal_fun, std::vector<state_t> & solution_path, float * p_plan_cost = nullptr)
	{
		heuristic_t h_fun(graph.transition_system());

		float initial_est = h_fun(init_state);
		cout << "Initial heuristic estimation " << initial_est << std::endl;

		this->enqueue(is_goal_fun, this->create_node(init_state, 0), node_estimation_t(0, initial_est));
		this->m_database.add_delayed(init_state, [=](const state_t & state){});

		float best_data = std::numeric_limits<float>::max();
		
		node_estimation_t current_data;

		bool db_flush_needed = false;
		bool * p_flush_flag = &db_flush_needed;

		while ((!_Base::m_searchQueue.empty()) && _Base::m_goalNodes.empty())
		{
			search_node_t cur_node = this->dequeue(&current_data);

			db_flush_needed = false;

			graph.forall_adj_verts(cur_node.state, [=](const state_t & state, float transition_cost){

				node_estimation_t new_est(cur_node.length + 1, h_fun(state));
				if (this->m_cmp(new_est, this->m_searchQueue.best_estimation()))
					*p_flush_flag = true;
				
				this->m_database.add_delayed(state, [=](const state_t & _state){
					if (this->enqueue(is_goal_fun, this->m_database.create_node(_state, cur_node.id, cur_node.length + transition_cost), new_est))	//If given node created the best layer or it is a goal
						*p_flush_flag = true;

					//
				});

				//if (cur_node.length + 1 + cur_est < get<1>(current_data) +get<0>(current_data))
				//	*p_flush_flag = true;
			});

			do
			{
				this->m_database.get_delayed_result();
			} while (_Base::m_searchQueue.empty());

			if (db_flush_needed)
				this->m_database.flush();
		}

#if TRACE_SOLUTION
		int i = 0;
		return this->finalize(solution_path, [&](const search_node_t & node){
			cout << i++ << ". " << "cost: " << node.length << ", est: " << h_fun(node.state) << std::endl;
		});
#else
		return this->finalize(solution_path);
#endif
	}
};

#endif
