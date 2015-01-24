
#ifndef UltraSolver_greedy_bfs_engine_h
#define UltraSolver_greedy_bfs_engine_h

#include "queued_engine.h"

template<class E>
struct greedy_bfs_node_priority_cmp
{
	typedef E element_t;

	bool operator()(const element_t & lhs, const element_t & rhs) const
	{
		return get<0>(lhs) < get<0>(rhs);
	}
};

template<typename Gr, typename H, bool ExtMemory>
class greedy_bfs_engine : public heuristic_engine<Gr, greedy_bfs_node_priority_cmp, H, ExtMemory>
{
	using _Base = heuristic_engine<Gr, greedy_bfs_node_priority_cmp, H, ExtMemory>;
public:
	//template<typename Gr>
	greedy_bfs_engine(Gr & graph, const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}
};

#endif
