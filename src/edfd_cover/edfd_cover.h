
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
};

struct edfd_connection
{
	string name; //name is required for every connection in edfd
};

typedef explicit_graph<edfd_element, edfd_connection> edfd_graph;

struct edfd_cover_state
{
	edfd_graph src_graph;
	vector<int> sdp_instances; //map SDP instance id(index in vector) to SDP type(index in sdps vector in edfd_cover class)
	vector<int> cover; //what SDP instance is used to cover i-th edfd_element in source graph
};

class edfd_cover
{
public:
	typedef edfd_cover_state state_t;

	struct transition_t
	{
		std::unordered_map<edfd_element::id_t, int> cover_difference; //new values for edfd_cover_state::cover fields
		vector<int> new_sdp_instances; //values to append to edfd_cover_state::sdp_instances vector
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
		for (const auto& sdp : sdps)
		{
			
		}
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