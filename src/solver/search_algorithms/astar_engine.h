
#ifndef UltraSolver_astar_engine_h
#define UltraSolver_astar_engine_h

#include "serial_engine.h"

template<class E>
struct astar_node_priority_cmp
{
	typedef E element_t;

	bool operator()(const node_estimation_t & lhs, const node_estimation_t & rhs) const
	{
		return lhs.total_estimation() < rhs.total_estimation();
		/*float sum1 = get<1>(lhs) + get<0>(lhs),
			sum2 = get<1>(rhs) + get<0>(rhs);
		return sum1 < sum2;

		if (sum1 == sum2)
		{
			return get<1>(lhs) > get<1>(rhs);
		}
		else
			return sum1 < sum2;*/
	}
};

template<typename Gr, typename H, bool ExtMemory>
class astar_engine : public heuristic_engine<Gr, astar_node_priority_cmp, H, ExtMemory, hashset_t::Buffered>
{
	using _Base = heuristic_engine<Gr, astar_node_priority_cmp, H, ExtMemory, hashset_t::Buffered>;
public:
	astar_engine(const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(vstreamer)
	{}
};

template<typename Gr, typename H, bool ExtMemory>
class delayed_astar_engine : public buffered_heuristic_engine<Gr, astar_node_priority_cmp, H, ExtMemory, hashset_t::Delayed>
{
	using _Base = buffered_heuristic_engine<Gr, astar_node_priority_cmp, H, ExtMemory, hashset_t::Delayed>;
public:
	delayed_astar_engine(const Gr & graph, const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer)
	{}
};

#endif
