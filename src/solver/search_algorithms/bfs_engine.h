

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

#endif

