
#ifndef UltraSolver_queued_engine_h
#define UltraSolver_queued_engine_h

#include "search_engine_base.h"
#include "../search_queue.h"
#include <queue>

template<typename N, typename M, template<typename> class Cmp, bool ExtMemory = false>
class queued_search_engine : public search_engine_base<N, ExtMemory>
{
	typedef search_engine_base<N, ExtMemory> _Base;
	typedef M element_meta_t;
	using search_node_t = typename _Base::search_node_t;
	
public:
	typedef std::tuple<element_meta_t, int> comparison_t;	//any meta + length to initial
	typedef std::pair<comparison_t, search_node_t> open_list_el_t;

	template<typename Gr>
	queued_search_engine(Gr & graph)
		:_Base(graph), m_searchQueue(graph.serialized_state_size() + sizeof(size_t)* 2 + sizeof(int), m_database.m_nodeSerializeFun, m_database.m_nodeDeserializeFun)
	{}

	struct stats_t : public _Base::stats_t
	{
		int queue_layers_count;
		size_t secondary_nodes_count;
	};

	stats_t get_stats() const
	{
		stats_t res = _Base::get_stats<stats_t>();
		res.queue_layers_count = m_searchQueue.layer_count();
		res.secondary_nodes_count = m_searchQueue.secondary_nodes_count();
		return res;
	}

	friend std::ostream & operator<<(std::ostream & os, const stats_t & stats)
	{
		cout << static_cast<const typename _Base::stats_t&>(stats);
		cout << "Search queue layer count: " << stats.queue_layers_count << std::endl;
		cout << "Search queue secondary node count: " << stats.secondary_nodes_count << std::endl;
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
	typedef search_queue<open_list_el_t, Cmp<comparison_t>> open_list_t;

protected:
	template<typename IsGoalFun>
	std::pair<bool, comparison_t> enqueue(IsGoalFun is_goal, search_node_t node, element_meta_t meta_data)
	{
		if (is_goal(get<2>(node)))
		{
			m_goalNodes.push_back(node);
			return make_pair(false, comparison_t());
		}
		else
			return m_searchQueue.push(open_list_el_t(comparison_t(meta_data, get<3>(node)), node));	
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
};


#endif
