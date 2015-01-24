
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
	astar_engine(Gr & graph, const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}
};

#endif
