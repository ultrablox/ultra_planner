
#include "test_helpers.h"
#include "transition_system_helpers.h"
#include <edfd_cover/edfd_cover.h>
#include <core/transition_system.h>
#include <sstream>

//! source graph is empty
//! available SDP set is empty
//! expected result: no available transitions
void test_empty()
{
	transition_system<edfd_cover> ts;

	edfd_cover_state state;
	state.src_graph = edfd_graph({}, {});
	bool result = test_available_transitions(ts, state, {});
	assert_test(result, "test_empty");
}

//! source graph is equal to one SDP graph
//! available SDP set contains one graph
//! expected result: one transition - cover source graph with only SDP
void test_equal_graph()
{
	edfd_element one  { 1, "one",   edfd_element::type_t::entity };
	edfd_element two  { 2, "two",   edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };
	edfd_graph source_graph(
		{one, two, three},
		{ { one, two }, { one, three }, {two, three} }
	);
	vector<edfd_graph> sdps{ source_graph };
	transition_system<edfd_cover> ts(sdps);

	edfd_cover_state state;
	state.src_graph = source_graph;

	vector<typename transition_system<edfd_cover>::transition_t> available_transitions;
	transition_system<edfd_cover>::transition_t tr;
	tr.new_sdp_instance = 0;
	tr.cover_difference = {1, 2, 3}; //all vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	bool result = test_available_transitions(ts, state, available_transitions);
	assert_test(result, "test_equal_graph");
}

//! source graph contains two subgraphs which are equal to one SDP graph
//! available SDP set contains one graph
//! expected result: two transitions - cover each source subgraph with only SDP
void test_graph_with_two_subgraphs()
{
	edfd_element one  { 1, "one", edfd_element::type_t::entity };
	edfd_element two  { 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };

	edfd_element four{ 4, "four", edfd_element::type_t::entity };
	edfd_element five{ 5, "five", edfd_element::type_t::entity };
	edfd_element six { 6, "six" , edfd_element::type_t::entity };

	edfd_graph source_graph(
		{ one, two, three, four, five, six },
		{
			{ one, two }, { one, three }, { two, three },
			{ four, five }, { four, six }, { five, six }
		}
	);

	edfd_graph sdp(
	{ one, two, three },
	{ { one, two }, { one, three }, { two, three } }
	);
	vector<edfd_graph> sdps{ sdp };
	transition_system<edfd_cover> ts(sdps);

	edfd_cover_state state;
	state.src_graph = source_graph;

	vector<typename transition_system<edfd_cover>::transition_t> available_transitions;
	transition_system<edfd_cover>::transition_t tr;

	tr.new_sdp_instance = 0;
	tr.cover_difference = { 1, 2, 3 }; //first three vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	tr.new_sdp_instance = 0;
	tr.cover_difference = { 4, 5, 6 }; //second three vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	bool result = test_available_transitions(ts, state, available_transitions);
	assert_test(result, "test_graph_with_two_subgraphs");
}

void test_edfd_cover()
{
	cout << "Testing edfd cover" << endl;
	
	test_empty();
	test_equal_graph();
	test_graph_with_two_subgraphs();
}