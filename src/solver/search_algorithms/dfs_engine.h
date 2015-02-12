
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

	bool operator()(const node_estimation_t & lhs, const node_estimation_t & rhs) const
	{
		return lhs.path_cost < rhs.path_cost;
	}
};

template<typename Gr>
class dfs_engine : public blind_engine<Gr, dfs_node_priority_cmp>
{
	using _Base = blind_engine<Gr, dfs_node_priority_cmp>;
public:
	//template<typename Gr>
	dfs_engine(const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(vstreamer)
	{}
};

#endif
