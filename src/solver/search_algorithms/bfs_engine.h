

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

	bool operator()(const node_estimation_t & lhs, const node_estimation_t & rhs) const
	{
		return lhs.path_cost > rhs.path_cost;
	}
};

template<typename Gr>
class bfs_engine : public blind_engine<Gr, bfs_node_priority_cmp>
{
	using _Base = blind_engine<Gr, bfs_node_priority_cmp>;
public:
	//template<typename Gr>
	bfs_engine(const Gr & graph, const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}
};

#endif

