

#ifndef UltraSolver_bfs_engine_h
#define UltraSolver_bfs_engine_h

#include "queued_engine.h"
#include <vector>
#include <functional>
#include <iostream>


using namespace std;

/*
Breadth first search.
*/

template<class E>
struct bfs_node_priority_cmp
{
	typedef E element_t;

	bool operator()(const element_t & lhs, const element_t & rhs) const
	{
		return get<1>(lhs) < get<1>(rhs);
	}
};

template<typename N>
class bfs_engine : public queued_search_engine<N, bool, bfs_node_priority_cmp>
{
	typedef queued_search_engine<N, bool, bfs_node_priority_cmp> _Base;

public:
	template<typename Gr>
	bfs_engine(Gr & graph)
		:_Base(graph)
	{}

	/*
	Returns true, if found.
	*/
	template<typename GraphT>
	bool operator()(GraphT & graph, state_t init_node, std::function<bool(state_t node)> goal_check_fun, std::vector<state_t> & solution_path)
	{
		enqueue(create_node(init_node, 0), 0);

		int step = 0;
		while((!m_searchQueue.empty()) && m_goalNodes.empty())
		{
			//Pop node for expansion and mark as expanded
			search_node_t cur_node = dequeue();

			forall_adj_vertices<true>(graph, get<2>(cur_node), [&](state_t state){
				search_node_t new_node = create_node(state, get<0>(cur_node));
				if(goal_check_fun(state))
					m_goalNodes.push_back(new_node);
				else
					enqueue(new_node, step);
			});
			++step;
		}

		if(!m_goalNodes.empty())
			solution_path = backtrace_path(m_goalNodes[0]);
		
		//Build solution path
		return !m_goalNodes.empty();
	}

};

#endif

