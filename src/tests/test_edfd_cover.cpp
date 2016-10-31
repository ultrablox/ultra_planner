
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
	bool result = test_available_transitions(ts, state, {});
	assert_test(result, "test_equal_graph");
}

void test_edfd_cover()
{
	cout << "Testing edfd cover" << endl;
	
	test_empty();
	test_equal_graph();
}