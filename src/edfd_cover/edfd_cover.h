
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
#include <set>
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

	friend bool operator == (const edfd_element& lhs, const edfd_element& rhs)
	{
		return lhs.id == rhs.id && lhs.name == rhs.name && lhs.type == rhs.type;
	}
};

struct edfd_connection
{
	string name; //name is required for every connection in edfd
};

namespace std
{
	template<>
	struct hash <edfd_element>
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

typedef explicit_graph<edfd_element, edfd_connection> edfd_graph;

struct edfd_cover_state
{
	edfd_graph src_graph;
	vector<size_t> sdp_instances; //map SDP instance id(index in vector) to SDP type(index in sdps vector in edfd_cover class)
	vector<size_t> cover; //what SDP instance is used to cover i-th edfd_element in source graph
};

struct edfd_cover_transition
{
	std::set<edfd_element::id_t> cover_difference; //what vertices are covered by this transition
	size_t new_sdp_instance; //value to append to edfd_cover_state::sdp_instances vector

	friend bool operator < (const edfd_cover_transition& lhs, const edfd_cover_transition& rhs)
	{
		if (lhs.new_sdp_instance < rhs.new_sdp_instance ||
			lhs.new_sdp_instance == rhs.new_sdp_instance && lhs.cover_difference.size() < rhs.cover_difference.size())
			return true;
		else if (lhs.new_sdp_instance == rhs.new_sdp_instance && lhs.cover_difference.size() == rhs.cover_difference.size())
			for (auto it1 = lhs.cover_difference.cbegin(), it2 = rhs.cover_difference.cbegin(); it1 != lhs.cover_difference.cend(); ++it1, ++it2)
			{
				if (*it1 < *it2)
					return true;
			}
		return false;
	}
	
	friend bool operator == (const edfd_cover_transition& lhs, const edfd_cover_transition& rhs)
	{
		return lhs.cover_difference == rhs.cover_difference && lhs.new_sdp_instance == rhs.new_sdp_instance; //tmp implementation just for compile test
	}
};

namespace std
{
	template<>
	struct hash<edfd_cover_transition>
	{
		std::size_t operator()(const edfd_cover_transition& tr) const
		{
			auto result = sdp_instance_hasher(tr.new_sdp_instance);
			for (const auto& id : tr.cover_difference)
				result += id_hasher(id);
			return result;
		}

		std::hash<decltype(edfd_cover_transition::new_sdp_instance)> sdp_instance_hasher;
		std::hash<edfd_element::id_t> id_hasher;
	};
}

class edfd_cover
{
public:
	typedef edfd_cover_state state_t;
	typedef edfd_cover_transition transition_t;

	edfd_cover()
	{
	}

	edfd_cover(vector<edfd_graph> sdps) :
		sdps(sdps)
	{
	}

	void apply(state_t & state, transition_t cover_diff) const
	{
		state.sdp_instances.push_back(cover_diff.new_sdp_instance); //add new sdp instance
		for (auto id : cover_diff.cover_difference)
			state.cover[id] = state.sdp_instances.size() - 1; //mark vertices covered by this new sdp instance
	}

	unordered_set<transition_t> generate_transitions_from_state(const state_t & base_state) const
	{
		//for each sdp from available set try to find sdp-like subgraph in current state source graph
		unordered_set<transition_t> result; //using set to prevent equivalent transitions in result
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
						transition.new_sdp_instance = i;

						queue<edfd_element> elements_to_check;
						unordered_map<edfd_element, edfd_element> used; //maps sdp vertices to src_graph vertices
						unordered_set<edfd_element> sdp_visited; //sdp vertices already visited
						unordered_set<edfd_element> source_used; //source vertices already used
						used[sdp_vertex] = source_vertex;
						elements_to_check.push(sdp_vertex);
						sdp_visited.insert(sdp_vertex); //don't try to use this vertex again
						source_used.insert(source_vertex); //don't try to use this vertex again
						bool fail = false;
						while (!elements_to_check.empty() && !fail)
						{
							auto curVertInSdp = elements_to_check.front();
							auto curVertInSrcGraph = used[curVertInSdp];
							elements_to_check.pop();

							//std::cout << "Start walk from SDP vertex " << curVertInSdp.id << " - " << curVertInSdp.name << std::endl;
							sdp.forall_adj_verts(curVertInSdp, [&](const edfd_element& vert, const edfd_connection& edge)
							{
								//std::cout << "In SDP vertex " << vert.id << " - " << vert.name << std::endl;
								if (sdp_visited.find(vert) != sdp_visited.end()) //we have already visited this vertex
									return;
								sdp_visited.insert(vert);

								bool found = false;
								//std::cout << "\tStart walk from source vertex " << curVertInSrcGraph.id << " - " << curVertInSrcGraph.name << std::endl;
								base_state.src_graph.forall_adj_verts(curVertInSrcGraph, [&](const edfd_element& src_vert, const edfd_connection& src_edge)
								{
									if (used.find(vert) == used.end())
									{
										//std::cout << "\tIn source vertex " << src_vert.id << " - " << src_vert.name << std::endl;
										if (source_used.find(src_vert) == source_used.end() &&
											vert.type == src_vert.type) //checking only types for now
										{
											used[vert] = src_vert;
											source_used.insert(src_vert);
											elements_to_check.push(vert);
											//std::cout << "\t\tUsing source vertex" << src_vert.id << " as SDP vertex " << vert.id << std::endl;
											found = true;
										}
									}
								});
								if (!found)
									fail = true;
							});
						}
						if (!fail)
						{
							for (const auto& vert : used)
								transition.cover_difference.insert(vert.second.id);
							result.insert(transition);
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
		auto available_transitions = generate_transitions_from_state(base_state);
		for (const transition_t& transition : available_transitions)
			fun(transition);
	}
protected:
	vector<edfd_graph> sdps; //available set of SDPs to cover source graph
};

#endif