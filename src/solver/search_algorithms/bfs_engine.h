

#ifndef UltraSolver_bfs_engine_h
#define UltraSolver_bfs_engine_h

#include "serial_engine.h"


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
		return get<1>(lhs) > get<1>(rhs);
	}
};

template<typename Gr>
class bfs_engine : public blind_engine<Gr, bfs_node_priority_cmp>
{
	using _Base = blind_engine<Gr, bfs_node_priority_cmp>;
public:
	//template<typename Gr>
	bfs_engine(Gr & graph, const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}
};

/*
template<typename N>
class bfs_engine : public queued_search_engine<N, bool, bfs_node_priority_cmp>
{
	typedef queued_search_engine<N, bool, bfs_node_priority_cmp> _Base;
	using state_t = typename _Base::state_t;
	using comparison_t = typename _Base::comparison_t;
	using state_t = typename _Base::state_t;
	using comparison_t = typename _Base::comparison_t;
	
public:
	template<typename Gr>
	bfs_engine(Gr & graph)
		:_Base(graph)
	{}

	//Returns true, if found.
	template<typename GraphT>
	bool operator()(GraphT & graph, state_t init_node, std::function<bool(const state_t & node)> goal_check_fun, std::vector<state_t> & solution_path)
	{
		enqueue(goal_check_fun, create_node(init_node, 0), 0);

		int step = 0;
		while((!m_searchQueue.empty()) && m_goalNodes.empty())
		{
			//Pop node for expansion and mark as expanded
			search_node_t cur_node = dequeue();


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
*/

#endif

