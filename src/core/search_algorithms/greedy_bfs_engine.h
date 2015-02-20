
#ifndef UltraSolver_greedy_bfs_engine_h
#define UltraSolver_greedy_bfs_engine_h

#include "queued_engine.h"

template<class E>
struct greedy_bfs_node_priority_cmp
{
	typedef E element_t;

	bool operator()(const node_estimation_t & lhs, const node_estimation_t & rhs) const
	{
		return lhs.heuristic_estimation < rhs.heuristic_estimation;
	}
};

template<typename Gr, typename H, bool ExtMemory>
class greedy_bfs_engine : public heuristic_engine<Gr, greedy_bfs_node_priority_cmp, H, ExtMemory, hashset_t::Buffered>
{
	using _Base = heuristic_engine<Gr, greedy_bfs_node_priority_cmp, H, ExtMemory, hashset_t::Buffered>;
public:
	//template<typename Gr>
	greedy_bfs_engine(const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(vstreamer)
	{}
};

#endif
