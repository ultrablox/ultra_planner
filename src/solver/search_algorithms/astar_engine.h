
#ifndef UltraSolver_astar_engine_h
#define UltraSolver_astar_engine_h

#include "serial_engine.h"

template<class E>
struct astar_node_priority_cmp
{
	typedef E element_t;

	bool operator()(const element_t & lhs, const element_t & rhs) const
	{
		float sum1 = get<1>(lhs) + get<0>(lhs),
			sum2 = get<1>(rhs) + get<0>(rhs);
		return sum1 < sum2;
	}
};

template<typename Gr, typename H, bool ExtMemory>
class astar_engine : public heuristic_engine<Gr, astar_node_priority_cmp, H, ExtMemory>
{
	using _Base = heuristic_engine<Gr, astar_node_priority_cmp, H, ExtMemory>;
public:
	//template<typename Gr>
	astar_engine(Gr & graph)
		:_Base(graph)
	{}
};

/*
template<typename N, typename H, bool ExtMemory>
class astar_engine : public queued_search_engine<N, float, astar_node_priority_cmp, ExtMemory>
{
	typedef float estimation_t;
	typedef queued_search_engine<N, float, astar_node_priority_cmp, ExtMemory> _Base;
	typedef H heuristic_t;
	using search_node_t = typename _Base::search_node_t;
	using state_t = typename _Base::state_t;
	using comparison_t = typename _Base::comparison_t;
public:
	template<typename Gr>
	astar_engine(Gr & graph)
		:_Base(graph)
	{}

	template<typename GraphT, typename IsGoalFun>
	bool operator()(GraphT & graph, const state_t & init_state, IsGoalFun is_goal_fun, std::vector<state_t> & solution_path)
	{
		heuristic_t h_fun(graph.transition_system());

		this->enqueue(is_goal_fun, this->create_node(init_state, 0), std::numeric_limits<float>::max());

		float best_data = std::numeric_limits<float>::max();
		comparison_t current_data;

		while((!_Base::m_searchQueue.empty()) && _Base::m_goalNodes.empty())
		{
			search_node_t cur_node = this->dequeue(&current_data);
			
			if(get<0>(current_data) < best_data)
			{
				best_data = get<0>(current_data);
				cout << "Best heuristic: " << best_data << std::endl;

			}

			graph.forall_adj_verts(get<2>(cur_node), [=](const state_t & state){
				//Check that node is not expanded or discovered by trying to add
				this->m_database.add(state, [=](const state_t & state){
					search_node_t new_node = this->m_database.create_node(state, get<0>(cur_node), get<3>(cur_node) +1);
					this->enqueue(is_goal_fun, new_node, h_fun(state));
				});
			});
		}

		if(!_Base::m_goalNodes.empty())
			solution_path = this->backtrace_path(_Base::m_goalNodes[0]);

		_Base::m_finished = true;

		return !_Base::m_goalNodes.empty();
	}
};*/

#endif
