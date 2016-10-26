
#ifndef UltraTest_edfd_cover_h
#define UltraTest_edfd_cover_h

#include <core/algorithm/math.h>
#include <core/io/streamer.h>
#include <core/utils/helpers.h>
#include <core/hash.h>
#include <core/compressed_stream.h>
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <random>
#include <queue>
#include <map>
#include "core/algorithm/graph.h"

#define STATE_COMPRESSION 1

using namespace std;

struct edfd_element
{
	typedef int id_t;
	enum class type_t
	{
		entity,
		process,
		nf_process //special type
	};

	id_t id;
	string name;
	type_t type;

	bool operator == (const edfd_element& rhs) const
	{
		return id == rhs.id && name == rhs.name && type == rhs.type;
	}
};

namespace std
{
	template<>
	struct hash<edfd_element>
	{
		std::size_t operator()(const edfd_element& vert) const
		{
			return id_hasher(vert.id) + name_hasher(vert.name) + type_hasher(vert.type);
		}

		std::hash<edfd_element::id_t> id_hasher;
		std::hash<string> name_hasher;
		std::hash<edfd_element::type_t> type_hasher;
	};
}

struct edfd_connection
{
	string name; //name is required for every connection in edfd
};

typedef explicit_graph<edfd_element, edfd_connection> edfd_graph;

struct edfd_cover_state
{
	edfd_graph src_graph;
	vector<size_t> sdp_instances; //map SDP instance id(index in vector) to SDP type(index in sdps vector in edfd_cover class)
	vector<size_t> cover; //what SDP instance is used to cover i-th edfd_element in source graph
};

class edfd_cover
{
public:
	typedef edfd_cover_state state_t;

	struct transition_t
	{
		std::unordered_map<edfd_element::id_t, size_t> cover_difference; //new values for edfd_cover_state::cover fields
		vector<size_t> new_sdp_instances; //values to append to edfd_cover_state::sdp_instances vector
	};

	edfd_cover()
	{

	}

	void apply(state_t & state, transition_t cover_diff) const
	{
		state.sdp_instances.insert(state.sdp_instances.end(), cover_diff.new_sdp_instances.begin(), cover_diff.new_sdp_instances.end());
		for (size_t i = 0; i < cover_diff.cover_difference.size(); ++i)
			if (cover_diff.cover_difference[i] != 0)
				state.cover[i] = cover_diff.cover_difference[i];
	}

	vector<transition_t> generate_transitions_from_state(const state_t & base_state) const
	{
		//for each sdp from available set try to find sdp-like subgraph in current state source graph
		vector<transition_t> result;
		const auto& source_vertices = base_state.src_graph.get_vertices();
		for (size_t i = 0; i < sdps.size(); ++i)
		{
			const edfd_graph& sdp = sdps[i];
			const auto& sdp_vertices = sdp.get_vertices();
			for (const edfd_element& sdp_vertex : sdp_vertices)
			{
				for (const edfd_element& source_vertex : source_vertices)
				{
					if (source_vertex.type == sdp_vertex.type)
					{
						transition_t transition;
						transition.new_sdp_instances = { i };

						queue<edfd_element> elements_to_check;
						unordered_map<edfd_element, edfd_element> used; //maps sdp vertices to src_graph vertices
						elements_to_check.push(sdp_vertex);
						bool fail = false;
						while (!elements_to_check.empty() && !fail)
						{
							auto curVertInSdp = elements_to_check.front();
							auto curVertInSrcGraph = used[curVertInSdp];
							
							sdp.forall_adj_verts(curVertInSdp, [&](const edfd_element& vert, const edfd_connection& edge)
							{
								bool found = false;
								base_state.src_graph.forall_adj_verts(curVertInSrcGraph, [&](const edfd_element& src_vert, const edfd_connection& src_edge)
								{
									if (vert.type == src_vert.type &&
										(used.find(vert) == used.end())) //checking only types for now
									{
										used[vert] = src_vert;
										elements_to_check.push(vert);
										found = true;
									}
								});
								if (!found)
									fail = true;
							});
						}
						if (!fail)
						{
							for (const auto& vert : used)
								transition.cover_difference[vert.second.id] = i;
							result.push_back(transition);
						}
					}
				}
			}
		}
		return result;
	}

	template<typename F>
	void forall_available_transitions(const state_t & base_state, F fun) const
	{
		vector<transition_t> available_transitions = generate_transitions_from_state(base_state);
		for (const auto& transition : available_transitions)
			F(transition);
	}
protected:
	vector<edfd_graph> sdps; //available set of SDPs to cover source graph
};

#endif