
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
		return get<1>(lhs) < get<1>(rhs);
	}
};

template<typename N>
class dfs_engine : public queued_search_engine<N, bool, dfs_node_priority_cmp>
{
	typedef queued_search_engine<N, bool, dfs_node_priority_cmp> _Base;
	using state_t = typename _Base::state_t;
	using comparison_t = typename _Base::comparison_t;
	using state_t = typename _Base::state_t;

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

			/*forall_adj_vertices<true>(graph, get<2>(cur_node), [&](state_t state){
				search_node_t new_node = create_node(state, get<0>(cur_node));
				enqueue(goal_check_fun, new_node, step);
				//cout << "Enqueueing:" << node << std::endl;
			});*/
			graph.forall_adj_verts(get<2>(cur_node), [=](const state_t & state){
				//Check that node is not expanded or discovered by trying to add
				m_database.add(state, [=](const state_t & state){
					search_node_t new_node = m_database.create_node(state, get<0>(cur_node), get<3>(cur_node) +1);
					enqueue(goal_check_fun, new_node, step);
				});
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
