#ifndef UltraCore_graph_h
#define UltraCore_graph_h

#include <unordered_set>
#include <unordered_map>
#include <core/streamer.h>

template<typename V>
class simple_graph
{
	typedef V vertex_t;
public:
	void serialize_state(void * dst, const vertex_t & vertex) const
	{
		*((vertex_t*)dst) = vertex;
	}

	void deserialize_state(const void * src, vertex_t & vertex) const
	{
		vertex = *((const vertex_t*)src);
	}

	int serialized_state_size() const
	{
		return sizeof(vertex_t);
	}
};

/*
Describes graph explicitely set by the
vertices and edges.
*/
template<typename V>
class explicit_graph : public simple_graph<V>
{
public:
	typedef V vertex_t;
	using vertex_streamer_t = base_type_streamer<vertex_t>;

	template<typename Verts, typename Edges>
	explicit_graph(const Verts & verts, const Edges & edgs)
	{

		const int v_len = sizeof(verts) / sizeof(verts[0]);
		for(int i = 0; i < v_len; ++i)
			m_vertices.insert(verts[i]);

		const int e_len = sizeof(edgs) / sizeof(edgs[0]);
		for(int i = 0; i < e_len; ++i)
		{
			m_adjacentLists[edgs[i][0]].push_back(edgs[i][1]);
			m_adjacentLists[edgs[i][1]].push_back(edgs[i][0]);
		}
	}

	template<typename F>
	void forall_adj_verts(const vertex_t & vertex, F fun) const
	{
		auto it = m_vertices.find(vertex);
		if(it == m_vertices.end())
			throw runtime_error("Vertex not found in graph");

		auto adj_it = m_adjacentLists.find(vertex);
		if(adj_it != m_adjacentLists.end())
		{
			for(auto & vert : adj_it->second)
				fun(vert);
		}
	}

private:
	std::unordered_set<vertex_t> m_vertices;
	std::unordered_map<vertex_t, std::list<vertex_t>> m_adjacentLists;
};

/*
Describes the graph, where vertices are not
known, before they were discovered.
*/
template<typename V>
class implicit_graph : public simple_graph<V>
{
public:	
	typedef V vertex_t;
	using vertex_streamer_t = base_type_streamer<vertex_t>;

	template<typename AdjFun>
	implicit_graph(AdjFun adj_fun)
		:m_adjGetter(adj_fun)
	{
	}

	template<typename F>
	void forall_adj_verts(const vertex_t & vertex, F fun) const
	{
		for(auto & vert : m_adjGetter(vertex))
			fun(vert);
	}

private:
	std::function<std::list<vertex_t>(vertex_t)> m_adjGetter;
};

template<typename TS>
class transition_system_graph
{
public:
	typedef TS transition_system_t;
	using vertex_t = typename transition_system_t::state_t;
	using vertex_streamer_t = typename transition_system_t::state_streamer_t;

	transition_system_graph(transition_system_t & ts)
		:m_system(ts)
	{
	}

	template<typename F>
	void forall_adj_verts(const vertex_t & vertex, F fun) const
	{
		m_system.forall_generated_states(vertex, fun);
	}

	const transition_system_t & transition_system() const
	{
		return m_system;
	}

	/*int serialized_state_size() const
	{
		return m_system.serialized_state_size();
	}

	void serialize_state(void * dst, const vertex_t & vertex) const
	{
		m_system.serialize_state(dst, vertex);
	}

	void deserialize_state(const void * src, vertex_t & vertex) const
	{
		m_system.deserialize_state(src, vertex);
	}*/
private:
	transition_system_t & m_system;
};

#endif
