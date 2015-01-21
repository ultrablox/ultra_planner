
#ifndef UltraSolver_search_engine_base_h
#define UltraSolver_search_engine_base_h

#include "../database/search_database.h"

//template<typename N, bool ExtMemory>
template<typename Gr, bool ExtMemory>
class search_engine_base
{
protected:
	//typedef N state_t;
	using graph_t = Gr;
	using state_t = typename Gr::vertex_t;
	using state_streamer_t = typename Gr::vertex_streamer_t;

	//typedef simple_search_database<state_t> search_database_t;
	typedef search_database<state_t, state_streamer_t, std::hash<state_t>, ExtMemory> search_database_t;

	//Id + parent search node id + state + init->current path length
	typedef typename search_database_t::search_node_t search_node_t;

	using node_streamer_t = typename search_database_t::node_streamer_t;

public:
	struct stats_t
	{
		size_t state_count;
		size_t database_block_count;
		float database_density;
	};

	//enum class state_state_t {Unknown, Discovered, Expanded};
	//template<typename Gr>
	search_engine_base(const graph_t & graph)
		:/*m_stateSerializeFun(
			[=](void * dest, const state_t & state){
					graph.serialize_state(dest, state);
				}),
			m_stateDeserializeFun(
				[=](const void * src, state_t & state){
					graph.deserialize_state(src, state);
			}),*/
			m_database(state_streamer_t(graph.transition_system())/*, graph.serialized_state_size(), m_stateSerializeFun, m_stateDeserializeFun*/), m_finished(false)
	{
	}
protected:
	search_node_t create_node(const state_t & state, size_t parent_id)
	{
		return m_database.create_node(state, parent_id);
	}

	std::vector<state_t> backtrace_path(search_node_t node)
	{
		std::vector<state_t> res;
		do
		{
			res.push_back(get<2>(node));
			node = m_database.parent_node(node);
		} while (get<0>(node) != get<1>(node));

		res.push_back(get<2>(node));

		return std::vector<state_t>(res.rbegin(), res.rend());
	}

	template<bool OnlyNew, typename Fun>
	void forall_adj_vertices(const graph_t & graph, state_t base_state, Fun fun) const
	{
		if (OnlyNew)
		{
			graph.forall_adj_verts(base_state, [&](const state_t & state){
				//Check that node is not expanded or discovered
				if (!m_database.contains(state))
					fun(state);
			});
		}
		else
		{
			graph.forall_adj_verts(base_state, [&](state_t state){
				fun(state);
			});
		}
	}
public:
	bool finished() const
	{
		return m_finished;
	}

	template<typename StatsT = stats_t>
	StatsT get_stats() const
	{
		StatsT res;
		res.state_count = m_database.state_count();
		res.database_block_count = m_database.block_count();
		res.database_density = m_database.density();
		return res;
	}

	friend std::ostream & operator<<(std::ostream & os, const stats_t & stats)
	{
		os << "node_count: " << stats.state_count << std::endl;
		os << "Database hashset block count: " << stats.database_block_count << std::endl;
		os << "Database density: " << stats.database_density << std::endl;
		return os;
	}
protected:
	std::vector<search_node_t> m_goalNodes;
	//std::vector<search_node_t> m_searchNodes;

	//std::function<void(void*, const state_t&)> m_stateSerializeFun;
	//std::function<void(const void*, state_t&)> m_stateDeserializeFun;

	search_database_t m_database;
	bool m_finished;	
};

#endif
