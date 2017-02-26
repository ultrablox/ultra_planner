
#include "test_helpers.h"
#include "transition_system_helpers.h"
#include <core/transition_system.h>
#include <core/state_space_solver.h>
#include <edfd_cover/edfd_cover.h>
#include <edfd_cover/heuristic.h>
#include <sstream>

void test_is_solved()
{
	edfd_element one{ 1, "one", edfd_element::type_t::entity };
	edfd_element two{ 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };
	edfd_graph source_graph(
		{ one, two, three },
		{ { one, two }, { one, three }, { two, three } }
	);

	transition_system<edfd_cover> ts(source_graph, vector<edfd_graph>{});

	edfd_cover_state state;

	edfd_cover_state final_state = state;
	final_state.sdp_instances.push_back(0);
	final_state.cover[1] = 0;
	final_state.cover[2] = 0;
	final_state.cover[3] = 0;

	assert_test(ts.is_solved(state) == false, "test_is_solved, state is not solved yet");
	assert_test(ts.is_solved(final_state), "test_is_solved, final_state is solved");
}

void test_apply_transition()
{
	edfd_element one{ 1, "one", edfd_element::type_t::entity };
	edfd_element two{ 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };
	edfd_graph source_graph(
	{ one, two, three },
	{ { one, two }, { one, three }, { two, three } }
	);
	vector<edfd_graph> sdps{ source_graph };
	transition_system<edfd_cover> ts(source_graph, sdps);

	edfd_cover_state state;

	edfd_cover_state final_state = state;
	final_state.sdp_instances.push_back(0);
	final_state.cover[1] = 0;
	final_state.cover[2] = 0;
	final_state.cover[3] = 0;

	transition_system<edfd_cover>::transition_t tr;
	tr.new_sdp_instance = 0;
	tr.cover_difference = { 1, 2, 3 }; //all vertices are covered with 0'th sdp instance

	bool result = test_apply_transition(ts, state, final_state, tr);
	assert_test(result, "test_apply_transition");
}

//! source graph is empty
//! available SDP set is empty
//! expected result: no available transitions
void test_empty()
{
	transition_system<edfd_cover> ts;
	edfd_cover_state state;

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
	transition_system<edfd_cover> ts(source_graph, sdps);

	edfd_cover_state state;

	vector<typename transition_system<edfd_cover>::transition_t> available_transitions;
	transition_system<edfd_cover>::transition_t tr;
	tr.new_sdp_instance = 0;
	tr.cover_difference = {1, 2, 3}; //all vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	bool result = test_available_transitions(ts, state, available_transitions);
	assert_test(result, "test_equal_graph");
}

//! source graph contains two subgraphs which are equal to one SDP graph
//! this subgraphs are not connected
//! available SDP set contains one graph
//! expected result: two transitions - cover each source subgraph with only SDP
void test_graph_with_two_subgraphs()
{
	edfd_element one  { 1, "one", edfd_element::type_t::entity };
	edfd_element two  { 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };

	edfd_element four{ 4, "one", edfd_element::type_t::entity };
	edfd_element five{ 5, "two", edfd_element::type_t::entity };
	edfd_element six { 6, "three" , edfd_element::type_t::entity };

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
	transition_system<edfd_cover> ts(source_graph, sdps);

	edfd_cover_state state;

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

//! source graph contains two subgraphs which are equal to one SDP graph
//! this subgraphs are connected with each other
//! available SDP set contains one graph
//! expected result: two transitions - cover each source subgraph with only SDP
void test_graph_with_two_connected_subgraphs()
{
	edfd_element one  { 1, "one", edfd_element::type_t::entity };
	edfd_element two  { 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };

	edfd_element four{ 4, "one", edfd_element::type_t::entity };
	edfd_element five{ 5, "two", edfd_element::type_t::entity };
	edfd_element six { 6, "three", edfd_element::type_t::entity };

	edfd_graph source_graph(
		{ one, two, three, four, five, six },
		{
			{ one, two }, { one, three }, { two, three },
			{ four, five }, { four, six }, { five, six },
			{ one, five }, { six, three } //connection between two parts
		}
	);

	edfd_graph sdp(
		{ one, two, three },
		{ { one, two }, { one, three }, { two, three } }
	);
	vector<edfd_graph> sdps{ sdp };
	transition_system<edfd_cover> ts(source_graph, sdps);

	edfd_cover_state state;

	vector<typename transition_system<edfd_cover>::transition_t> available_transitions;
	transition_system<edfd_cover>::transition_t tr;

	tr.new_sdp_instance = 0;
	tr.cover_difference = { 1, 2, 3 }; //first three vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	tr.new_sdp_instance = 0;
	tr.cover_difference = { 4, 5, 6 }; //second three vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	bool result = test_available_transitions(ts, state, available_transitions);
	assert_test(result, "test_graph_with_two_connected_subgraphs");
}

//! source graph contains three subgraphs each of them is equal to one SDP graph
//! two subgraphs is equal to each other and are part of third graph
//! available SDP set contains two graphs
//! one graph is part of another
//! expected result: three transitions - cover each source subgraph with corresponding SDP
void test_graph_with_included_subgraphs()
{
	edfd_element one  { 1, "one", edfd_element::type_t::entity };
	edfd_element two  { 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };
	edfd_element four { 4, "four", edfd_element::type_t::entity };

	edfd_graph source_graph(
		{ one, two, three, four},
		{
			{ one, two }, { one, three }, { two, three },
			{ four, one }, { four, two }
		}
	);

	edfd_graph sdp1(
		{ one, two, three },
		{ { one, two }, { one, three }, { two, three } }
	);

	edfd_graph sdp2{ source_graph };

	vector<edfd_graph> sdps{ sdp1, sdp2 };
	transition_system<edfd_cover> ts(source_graph, sdps);

	edfd_cover_state state;

	vector<typename transition_system<edfd_cover>::transition_t> available_transitions;
	transition_system<edfd_cover>::transition_t tr;

	tr.new_sdp_instance = 0;
	tr.cover_difference = { 1, 2, 3 }; //first three vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	tr.new_sdp_instance = 0;
	tr.cover_difference = { 4, 1, 2 }; //first three vertices are covered with 0'th sdp instance
	available_transitions.push_back(tr);

	tr.new_sdp_instance = 1;
	tr.cover_difference = { 1, 2, 3, 4 }; //second three vertices are covered with 1'st sdp instance
	available_transitions.push_back(tr);

	bool result = test_available_transitions(ts, state, available_transitions);
	assert_test(result, "test_graph_with_included_subgraphs");
}

void test_graph_with_two_suitable_sdps()
{
	using edge_tup = std::tuple < edfd_element, edfd_element, edfd_connection >;

	edfd_element db{ 1, "Database", edfd_element::type_t::entity };
	edfd_element receive_data{ 2, "Receive data", edfd_element::type_t::process };

	edfd_graph db_integration_sdp(
		{ db, receive_data },
		{ edge_tup{ db, receive_data, "data" } }
	);

	edfd_element app{ 3, "Application package", edfd_element::type_t::entity };
	edfd_element calc{ 4, "Do calculation", edfd_element::type_t::process };

	edfd_graph app_integration_sdp(
		{ app, calc },
		{
			edge_tup{ app, calc, "result" },
			edge_tup{ calc, app, "data" }
		}
	);

	vector<edfd_graph> sdps = { db_integration_sdp, app_integration_sdp };
	edfd_graph source_graph = app_integration_sdp;

	transition_system<edfd_cover> ts(source_graph, sdps);
	edfd_cover_state state;

	vector<typename transition_system<edfd_cover>::transition_t> available_transitions;
	transition_system<edfd_cover>::transition_t tr;

	tr.new_sdp_instance = 1;
	tr.cover_difference = { 3, 4 };
	available_transitions.push_back(tr);

	bool result = test_available_transitions(ts, state, available_transitions);
	assert_test(result, "test_graph_with_two_suitable_sdps");
}

void test_real_sdps()
{
	using edge_tup = std::tuple < edfd_element, edfd_element, edfd_connection >;

	edfd_element db{ 1, "Database", edfd_element::type_t::entity };
	edfd_element receive_data{ 2, "Receive data", edfd_element::type_t::process };

	edfd_graph db_integration_sdp(
	{ db, receive_data },
	{ edge_tup{ db, receive_data, "data" } }
	);

	edfd_element app{ 3, "Application package", edfd_element::type_t::entity };
	edfd_element calc{ 4, "Do calculation", edfd_element::type_t::process };

	edfd_graph app_integration_sdp(
	{ app, calc },
	{
		edge_tup{ app, calc, "result" },
		edge_tup{ calc, app, "data" }
	}
	);

	edfd_element nf_operation{ 5, "NF operation", edfd_element::type_t::nf_process };
	edfd_element setup_situation{ 6, "Setup initial situation", edfd_element::type_t::process };
	edfd_element show_recommendations{ 7, "Show recommendations", edfd_element::type_t::process };

	edfd_graph main_part_sdp(
	{ nf_operation, setup_situation, show_recommendations },
	{
		edge_tup{ setup_situation, nf_operation, "facts" },
		edge_tup{ nf_operation, show_recommendations, "result of solving" },
	}
	);

	edfd_graph source_graph(
	{ db, receive_data, app, calc, nf_operation, setup_situation, show_recommendations },
	{
		edge_tup{ db, receive_data, "data" },
		edge_tup{ app, calc, "result" },
		edge_tup{ calc, app, "data" },
		edge_tup{ setup_situation, nf_operation, "facts" },
		edge_tup{ nf_operation, show_recommendations, "result of solving" },
	}
	);

	vector<edfd_graph> sdps{ db_integration_sdp, app_integration_sdp, main_part_sdp };

	transition_system<edfd_cover> ts(source_graph, sdps);
	edfd_cover_state state;

	vector<typename transition_system<edfd_cover>::transition_t> available_transitions;
	transition_system<edfd_cover>::transition_t tr;

	tr.new_sdp_instance = 0;
	tr.cover_difference = { 1, 2 };
	available_transitions.push_back(tr);
	
	tr.new_sdp_instance = 1;
	tr.cover_difference = { 3, 4 };
	available_transitions.push_back(tr);

	tr.new_sdp_instance = 2;
	tr.cover_difference = { 5, 6, 7 };
	available_transitions.push_back(tr);

	bool result = test_available_transitions(ts, state, available_transitions);
	assert_test(result, "test_real_sdps");
}

void test_planning_helper(const edfd_graph & source_graph, const vector<edfd_graph> sdps, const string & test_name)
{
	transition_system<edfd_cover> ts(source_graph, sdps);
	edfd_cover_state state;
	state_space_solver<transition_system<edfd_cover>> solver(ts, cout, state);

	for (auto& alg_name : { "A*", /*"BA*",*/ /*"GBFS"*/ })
		for (bool ext_mem : {false/*, true*/})
		{
			string test_full_name = test_name + " with ext_mem=" + (ext_mem ? "true" : "false") + ", alg_name=" + alg_name;
			cout << test_full_name << endl;
			assert_test(solver.solve<simple_heuristic>(ext_mem, alg_name), test_full_name);
		}
}

//! source graph is equal to one SDP graph
//! available SDP set contains one graph
//! expected result: one transition - cover source graph with only SDP
void test_planning_simple()
{
	edfd_element one{ 1, "one", edfd_element::type_t::entity };
	edfd_element two{ 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };
	edfd_graph source_graph(
	{ one, two, three },
	{ { one, two }, { one, three }, { two, three } }
	);
	vector<edfd_graph> sdps{ source_graph };
	test_planning_helper(source_graph, sdps, "test_planning");
}

//! source graph contains two subgraphs which are equal to one SDP graph
//! this subgraphs are not connected
//! available SDP set contains one graph
//! expected result: two transitions - cover each source subgraph with only SDP
void test_planning_with_two_subgraphs()
{
	edfd_element one{ 1, "one", edfd_element::type_t::entity };
	edfd_element two{ 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };

	edfd_element four{ 4, "one", edfd_element::type_t::entity };
	edfd_element five{ 5, "two", edfd_element::type_t::entity };
	edfd_element six{ 6, "three", edfd_element::type_t::entity };

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

	test_planning_helper(source_graph, sdps, "test_planning_with_two_subgraphs");
}

void test_planning_with_included_subgraphs()
{
	edfd_element one{ 1, "one", edfd_element::type_t::entity };
	edfd_element two{ 2, "two", edfd_element::type_t::entity };
	edfd_element three{ 3, "three", edfd_element::type_t::entity };
	edfd_element four{ 4, "four", edfd_element::type_t::entity };

	edfd_graph source_graph(
	{ one, two, three, four },
	{
		{ one, two }, { one, three }, { two, three },
		{ four, one }, { four, two }
	}
	);

	edfd_graph sdp1(
	{ one, two, three },
	{ { one, two }, { one, three }, { two, three } }
	);

	edfd_graph sdp2{ source_graph };

	vector<edfd_graph> sdps{ sdp1, sdp2 };

	test_planning_helper(source_graph, sdps, "test_planning_with_included_subgraphs");
}

void test_planning_real_sdps()
{
	using edge_tup = std::tuple < edfd_element, edfd_element, edfd_connection > ;

	edfd_element db{ 1, "Database", edfd_element::type_t::entity };
	edfd_element receive_data{ 2, "Receive data", edfd_element::type_t::process };

	edfd_graph db_integration_sdp(
		{ db, receive_data },
		{ edge_tup{ db, receive_data, "data" } }
	);

	edfd_element app{ 3, "Application package", edfd_element::type_t::entity };
	edfd_element calc{ 4, "Do calculation", edfd_element::type_t::process };

	edfd_graph app_integration_sdp(
		{ app, calc },
		{
			edge_tup{ app, calc, "result" },
			edge_tup{ calc, app, "data" }
		}
	);

	edfd_element nf_operation{ 5, "NF operation", edfd_element::type_t::nf_process };
	edfd_element setup_situation{ 6, "Setup initial situation", edfd_element::type_t::process };
	edfd_element show_recommendations{ 7, "Show recommendations", edfd_element::type_t::process };

	edfd_graph main_part_sdp(
		{nf_operation, setup_situation, show_recommendations},
		{
			edge_tup{ setup_situation, nf_operation, "facts" },
			edge_tup{ nf_operation, show_recommendations, "result of solving" },
		}
	);

	edfd_graph source_graph(
		{db, receive_data, app, calc, nf_operation, setup_situation, show_recommendations},
		{
			edge_tup{ db, receive_data, "data" },
			edge_tup{ app, calc, "result" },
			edge_tup{ calc, app, "data" },
			edge_tup{ setup_situation, nf_operation, "facts" },
			edge_tup{ nf_operation, show_recommendations, "result of solving" },
		}
	);

	vector<edfd_graph> sdps {db_integration_sdp, app_integration_sdp, main_part_sdp};

	test_planning_helper(source_graph, sdps, "test_planning_real_sdps");
}

void test_full_problem()
{
	using edge_tup = std::tuple < edfd_element, edfd_element, edfd_connection >;

	edfd_element db{ 1, "Database", edfd_element::type_t::entity };
	edfd_element receive_data{ 2, "Receive data", edfd_element::type_t::process };

	edfd_graph db_integration_sdp(
	{ db, receive_data },
	{ edge_tup{ db, receive_data, "data" } }
	);

	edfd_element app{ 3, "Application package", edfd_element::type_t::entity };
	edfd_element calc{ 4, "Do calculation", edfd_element::type_t::process };

	edfd_graph app_integration_sdp(
	{ app, calc },
	{
		edge_tup{ app, calc, "result" },
		edge_tup{ calc, app, "data" }
	}
	);

	edfd_element nf_operation{ 5, "NF operation", edfd_element::type_t::nf_process };
	edfd_element setup_situation{ 6, "Setup initial situation", edfd_element::type_t::process };
	edfd_element show_recommendations{ 7, "Show recommendations", edfd_element::type_t::process };

	edfd_graph main_part_sdp(
	{ nf_operation, setup_situation, show_recommendations },
	{
		edge_tup{ setup_situation, nf_operation, "facts" },
		edge_tup{ nf_operation, show_recommendations, "result of solving" },
	}
	);

	edfd_element additional_element{ 8, "Do specific operation", edfd_element::type_t::process };

	edfd_graph source_graph(
	{ db, receive_data, app, calc, nf_operation, setup_situation, show_recommendations, additional_element },
	{
		edge_tup{ db, receive_data, "data" },
		edge_tup{ receive_data, additional_element, "data" },
		edge_tup{ additional_element, nf_operation, "data" },
		edge_tup{ nf_operation, additional_element, "requests" },
		edge_tup{ app, calc, "result" },
		edge_tup{ calc, app, "data" },
		edge_tup{ setup_situation, nf_operation, "facts" },
		edge_tup{ nf_operation, show_recommendations, "result" },
	}
	);

	vector<edfd_graph> optional_sdps{ db_integration_sdp, app_integration_sdp};
	
	transition_system<edfd_cover> rough_cover(source_graph, vector<edfd_graph>{ main_part_sdp });
	edfd_cover_state state, rough_covered_state;
	state_space_solver<transition_system<edfd_cover>> rough_solver(rough_cover, cout, state);
	bool solved = rough_solver.solve<simple_heuristic>(false, "A*", rough_covered_state);
	
	assert_test(solved, "rough cover");
	cout << "Rough cover done" << endl;

	transition_system<edfd_cover> precise_cover(source_graph, optional_sdps);
	state_space_solver<transition_system<edfd_cover>> precise_solver(precise_cover, cout, rough_covered_state);
	solved = precise_solver.solve<simple_heuristic>(false, "A*");

	assert_test(solved, "precise cover");
	cout << "Precise cover done" << endl;
}

void test_edfd_cover()
{
	cout << "Testing edfd cover" << endl;
	
	test_empty();
	test_equal_graph();
	test_graph_with_two_subgraphs();
	test_graph_with_two_connected_subgraphs();
	test_graph_with_included_subgraphs();
	test_graph_with_two_suitable_sdps();
	test_real_sdps();

	test_apply_transition();

	test_is_solved();

	test_planning_simple();
	test_planning_with_two_subgraphs();
	test_planning_with_included_subgraphs();
	test_planning_real_sdps();

	test_full_problem();
}