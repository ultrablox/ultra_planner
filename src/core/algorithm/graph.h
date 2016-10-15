#ifndef UltraCore_graph_h
#define UltraCore_graph_h

#include <unordered_set>
#include <unordered_map>
#include <core/io/streamer.h>
#include <list>

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
template<typename V, typename E>
class explicit_graph : public simple_graph<V>
{
public:
	using vertex_t = V;
	using edge_t = E;
	using vertex_streamer_t = base_type_streamer<vertex_t>;

	struct adjacent_vertex_t
	{
		edge_t edge;
		vertex_t vertex;
	};

	explicit_graph()
	{}

	explicit_graph(const std::initializer_list<vertex_t> & verts, const std::initializer_list<std::pair<vertex_t, vertex_t>> & adjacent_verts)
	{
		for (auto & v : verts)
			m_vertices.insert(v);

		for (auto & av : adjacent_verts)
		{
			adjacent_vertex_t forward_edge;
			forward_edge.vertex = av.second;
			m_adjacentLists[av.first].push_back(forward_edge);

			adjacent_vertex_t backward_edge;
			forward_edge.vertex = av.first;
			m_adjacentLists[av.second].push_back(backward_edge);
		}
	}

	template<typename F>
	void forall_adj_verts(const vertex_t & vertex, F fun) const
	{
		auto adj_it = m_adjacentLists.find(vertex);
#if _DEBUG
		assert(m_vertices.find(vertex) != m_vertices.end());
		assert(adj_it != m_adjacentLists.end());
#endif

		for(auto & edge : adj_it->second)
			fun(edge.vertex, edge.edge);
	}

	void add_vertex(const vertex_t & vert)
	{
		m_vertices.insert(vert);
	}

	void add_edge(const vertex_t & src, const vertex_t & dst, const edge_t & edg)
	{
#if _DEBUG
		assert(m_vertices.find(src) != m_vertices.end());
		assert(m_vertices.find(dst) != m_vertices.end());
#endif
		adjacent_vertex_t new_edge;
		new_edge.edge = edg;
		new_edge.vertex = dst;

		auto it = m_adjacentLists.find(src);
		if (it == m_adjacentLists.end())
			m_adjacentLists.insert(make_pair(src, std::list<adjacent_vertex_t>(1, new_edge)));
		else
		{
			it->second.push_back(new_edge);
		}
	}

	std::list<adjacent_vertex_t> & adjacent_list(const vertex_t & vertex)
	{
		auto it = m_adjacentLists.find(vertex);
		return it->second;
	}

	const std::unordered_set<vertex_t> get_vertices() const
	{
		return m_vertices;
	}
private:
	std::unordered_set<vertex_t> m_vertices;
	std::unordered_map<vertex_t, std::list<adjacent_vertex_t>> m_adjacentLists;
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
			fun(vert, 1);
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

	transition_system_graph(const transition_system_t & ts)
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
	const transition_system_t & m_system;
};

#endif
