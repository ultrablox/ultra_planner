
#ifndef UltraSolver_queued_engine_h
#define UltraSolver_queued_engine_h

#include "search_engine_base.h"
#include "../search_queue.h"
#include <queue>

template<typename Gr, typename M, template<typename> class Cmp, bool ExtMemory = false, bool RAMBuffered = true>
class queued_search_engine : public search_engine_base<Gr, ExtMemory, RAMBuffered>
{
protected:
	using graph_t = Gr;
	typedef search_engine_base<Gr, ExtMemory, RAMBuffered> _Base;
	typedef M element_meta_t;
	using search_node_t = typename _Base::search_node_t;
	
public:
	typedef std::tuple<element_meta_t, int> comparison_t;	//any meta + length to initial
	typedef std::pair<comparison_t, search_node_t> open_list_el_t;

	//template<typename Gr>
	queued_search_engine(graph_t & graph, const typename Gr::vertex_streamer_t & vstreamer)
		:_Base(graph, vstreamer), m_searchQueue(typename _Base::node_streamer_t(vstreamer)/*, graph.serialized_state_size() + sizeof(size_t)* 2 + sizeof(int), _Base::m_database.m_nodeSerializeFun, _Base::m_database.m_nodeDeserializeFun*/), m_firstNode(true)
	{}

	struct stats_t : public _Base::stats_t
	{
		int queue_layers_count;
		size_t secondary_nodes_count;
		float best_priority, max_distance_from_initial;
	};

	stats_t get_stats() const
	{
		stats_t res = _Base::template get_stats<stats_t>();
		res.queue_layers_count = m_searchQueue.layer_count();
		res.secondary_nodes_count = m_searchQueue.secondary_nodes_count();
		res.best_priority = get<0>(m_bestPriority) + get<1>(m_bestPriority);
		res.max_distance_from_initial = m_farestDistance;
		return res;
	}

	friend std::ostream & operator<<(std::ostream & os, const stats_t & stats)
	{
		os << static_cast<const typename _Base::stats_t&>(stats);
		os << "Search queue layer count: " << stats.queue_layers_count << std::endl;
		os << "Search queue secondary node count: " << stats.secondary_nodes_count << std::endl;
		os << "Best priority: " << stats.best_priority << std::endl;
		os << "Max distance from initial: " << stats.max_distance_from_initial << std::endl;
		return os;
	}
protected:
	


	//Open list structure
/*	class open_list_cmp
	{
		Cmp<comparison_t> m_cmp;
		bool reverse;
	public:
		open_list_cmp(const bool& revparam=false)
			:reverse(revparam)
		{}

		bool operator() (const open_list_el_t & lhs, const open_list_el_t &rhs) const
		{
			if(reverse)
				return m_cmp(lhs.first, rhs.first);
			else
				return m_cmp(rhs.first, lhs.first);
		}
	};*/

	//typedef std::priority_queue<open_list_el_t, std::vector<open_list_el_t>, open_list_cmp> open_list_t;
	typedef search_queue<open_list_el_t, Cmp<comparison_t>, typename _Base::node_streamer_t> open_list_t;

protected:
	template<typename IsGoalFun>
	std::pair<bool, comparison_t> enqueue(IsGoalFun is_goal, search_node_t node, element_meta_t meta_data)
	{
		if (is_goal(get<2>(node)))
		{
			_Base::m_goalNodes.push_back(node);
			return make_pair(false, comparison_t());
		}
		else
		{
			m_farestDistance = max(m_farestDistance, (float)get<3>(node));

			comparison_t new_prior(meta_data, get<3>(node));
			
			if (m_firstNode)
			{
				m_bestPriority = new_prior;
				m_firstNode = false;
			}
			else
			{
				if (m_cmp(new_prior, m_bestPriority))
					m_bestPriority = new_prior;
			}

			return m_searchQueue.push(open_list_el_t(new_prior, node));
		}
	}

	search_node_t dequeue(comparison_t * meta_data = nullptr)
	{
		if(meta_data)
			*meta_data = std::move(m_searchQueue.top().first);

		search_node_t cur_node = std::move(m_searchQueue.top().second);
		m_searchQueue.pop();

		return cur_node;
	}

	bool queue_empty() const
	{
		return m_searchQueue.empty();
	}

protected:
	open_list_t m_searchQueue;
	comparison_t m_bestPriority;
	Cmp<comparison_t> m_cmp;
	bool m_firstNode;
	float m_farestDistance;
};


#endif
