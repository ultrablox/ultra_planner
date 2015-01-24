
#ifndef UltraSolver_serial_engine_h
#define UltraSolver_serial_engine_h

#include "queued_engine.h"

template<typename Gr, template<typename> class Cmp>
class blind_engine : public queued_search_engine<Gr, bool, Cmp>
{
	using graph_t = Gr;
	typedef queued_search_engine<Gr, bool, Cmp> _Base;
	using state_t = typename _Base::state_t;
	using comparison_t = typename _Base::comparison_t;
	using search_node_t = typename _Base::search_node_t;

public:
	//template<typename Gr>
	blind_engine(graph_t & graph, const typename graph_t::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}

	//Returns true, if found.
	//template<typename GraphT>
	bool operator()(graph_t & graph, state_t init_node, std::function<bool(const state_t & node)> goal_check_fun, std::vector<state_t> & solution_path)
	{
		enqueue(goal_check_fun, create_node(init_node, 0), 0);

		int step = 0;
		while ((!this->m_searchQueue.empty()) && this->m_goalNodes.empty())
		{
			//Pop node for expansion and mark as expanded
			search_node_t cur_node = this->dequeue();


			graph.forall_adj_verts(get<2>(cur_node), [=](const state_t & state){
				//Check that node is not expanded or discovered by trying to add
				this->m_database.add(state, [=](const state_t & state){
					search_node_t new_node = this->m_database.create_node(state, get<0>(cur_node), get<3>(cur_node) +1);
					enqueue(goal_check_fun, new_node, step);
				});
			});
			++step;
		}

		return this->finalize(solution_path);
	}

};

template<typename Gr, template<typename> class Cmp, typename H, bool ExtMemory>
class heuristic_engine : public queued_search_engine<Gr, float, Cmp, ExtMemory, true>
{
	//using priority_t = template<typename> Cmp;
	using graph_t = Gr;
	typedef float estimation_t;
	typedef queued_search_engine<Gr, float, Cmp, ExtMemory> _Base;
	typedef H heuristic_t;
	using search_node_t = typename _Base::search_node_t;
	using state_t = typename _Base::state_t;
	using comparison_t = typename _Base::comparison_t;
public:
	//template<typename Gr>
	heuristic_engine(graph_t & graph, const typename graph_t::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}

	template<typename IsGoalFun>
	bool operator()(graph_t & graph, const state_t & init_state, IsGoalFun is_goal_fun, std::vector<state_t> & solution_path)
	{
		heuristic_t h_fun(graph.transition_system());

		this->enqueue(is_goal_fun, this->create_node(init_state, 0), std::numeric_limits<float>::max());

		float best_data = std::numeric_limits<float>::max();
		comparison_t current_data;

		while ((!_Base::m_searchQueue.empty()) && _Base::m_goalNodes.empty())
		{
			search_node_t cur_node = this->dequeue(&current_data);

			graph.forall_adj_verts(get<2>(cur_node), [=](const state_t & state){
				//Check that node is not expanded or discovered by trying to add
				this->m_database.add(state, [=](const state_t & state){
					this->enqueue(is_goal_fun, this->m_database.create_node(state, get<0>(cur_node), get<3>(cur_node) + 1), h_fun(state));
				});
			});
		}

		return this->finalize(solution_path);
	}
};

template<typename Gr, template<typename> class Cmp, typename H, bool ExtMemory>
class buffered_heuristic_engine : public queued_search_engine<Gr, float, Cmp, ExtMemory, false>
{
	using _Base = queued_search_engine<Gr, float, Cmp, ExtMemory, false>;
	using graph_t = Gr;
	typedef float estimation_t;
	typedef H heuristic_t;
	using search_node_t = typename _Base::search_node_t;
	using state_t = typename _Base::state_t;
	using comparison_t = typename _Base::comparison_t;
public:
	buffered_heuristic_engine(graph_t & graph, const typename graph_t::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}

	template<typename IsGoalFun>
	bool operator()(graph_t & graph, const state_t & init_state, IsGoalFun is_goal_fun, std::vector<state_t> & solution_path)
	{
		heuristic_t h_fun(graph.transition_system());

		this->enqueue(is_goal_fun, this->create_node(init_state, 0), std::numeric_limits<float>::max());

		float best_data = std::numeric_limits<float>::max();
		comparison_t current_data;

		while ((!_Base::m_searchQueue.empty()) && _Base::m_goalNodes.empty())
		{
			search_node_t cur_node = this->dequeue(&current_data);

			graph.forall_adj_verts(get<2>(cur_node), [=](const state_t & state){
				//Check that node is not expanded or discovered by trying to add
				this->m_database.add(state, [=](const state_t & state){
					this->enqueue(is_goal_fun, this->m_database.create_node(state, get<0>(cur_node), get<3>(cur_node) +1), h_fun(state));
				});
			});
		}

		return this->finalize(solution_path);
	}
};

#endif
