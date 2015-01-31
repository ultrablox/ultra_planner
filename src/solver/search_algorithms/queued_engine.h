
#ifndef UltraSolver_queued_engine_h
#define UltraSolver_queued_engine_h

#include "search_engine_base.h"
#include "../search_queue.h"
#include <queue>

template<typename Gr, /*typename M,*/ template<typename> class Cmp, bool ExtMemory = false, bool RAMBuffered = true>
class queued_search_engine : public search_engine_base<Gr, ExtMemory, RAMBuffered>
{
protected:
	typedef search_engine_base<Gr, ExtMemory, RAMBuffered> _Base;
	using graph_t = Gr;
	
	//typedef M element_meta_t;
	using search_node_t = typename _Base::search_node_t;
	
	
public:
	using open_list_t = search_queue<search_node_t, Cmp<node_estimation_t>, typename _Base::node_streamer_t>;
	using open_list_el_t = typename open_list_t::combined_value_t;

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
		res.best_priority = m_bestEstimation.total_estimation();//get<0>(m_bestPriority) + get<1>(m_bestPriority);
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
	template<typename IsGoalFun>
	bool enqueue(IsGoalFun is_goal, const search_node_t & node, const node_estimation_t & node_estimation)
	{
		if (is_goal(node.state))
		{
			if (m_cmp(m_searchQueue.best_estimation(), node_estimation))
			{
				cout << "Goal found, but it is not optimal (length " << node.length << ")" << std::endl;
				//_Base::m_goalNodes.push_back(node);
				return true;
			}
			else
			{
				_Base::m_goalNodes.push_back(node);
				return true;
			}			
		}
		else
		{
			m_farestDistance = max(m_farestDistance, (float)node.length);
			
			if (m_firstNode)
			{
				m_bestEstimation = node_estimation;
				m_firstNode = false;
			}
			else
			{
				if (m_cmp(node_estimation, m_bestEstimation))
					m_bestEstimation = node_estimation;
			}

			return m_searchQueue.push(open_list_el_t(node_estimation, node));
		}
	}

	search_node_t dequeue(node_estimation_t * meta_data = nullptr)
	{
		search_node_t cur_node;
		m_searchQueue.pop_and_call([&](const node_estimation_t & pri, const search_node_t & top_node){
			if (meta_data)
				*meta_data = pri;
			cur_node = top_node;
		});

		return cur_node;
	}

	bool queue_empty() const
	{
		return m_searchQueue.empty();
	}

protected:
	open_list_t m_searchQueue;
	node_estimation_t m_bestEstimation;
	Cmp<node_estimation_t> m_cmp;
	bool m_firstNode;
	float m_farestDistance;
};


#endif
