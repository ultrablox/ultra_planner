
#ifndef UltraSolver_dfs_engine_h
#define UltraSolver_dfs_engine_h

#include "queued_engine.h"

/*
Depth first search.
*/

template<class E>
struct dfs_node_priority_cmp
{
	typedef E element_t;

	bool operator()(const element_t & lhs, const element_t & rhs) const
	{
		return get<1>(lhs) > get<1>(rhs);
	}
};

template<typename N>
class dfs_engine : public queued_search_engine<N, bool, dfs_node_priority_cmp>
{
	typedef queued_search_engine<N, bool, dfs_node_priority_cmp> _Base;

public:
	template<typename Gr>
	dfs_engine(Gr & graph)
		:_Base(graph)
	{}

	/*
	Returns true, if found.
	*/
	template<typename GraphT>
	bool operator()(GraphT & graph, state_t init_node, std::function<bool(state_t node)> goal_check_fun, std::vector<state_t> & solution_path)
	{
		enqueue(goal_check_fun, create_node(init_node, 0), 0);

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
					enqueue(goal_check_fun, new_node, step);
				//cout << "Enqueueing:" << node << std::endl;
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
