
#include "test_helpers.h"
#include <core/search_algorithms/bfs_engine.h>
#include <core/search_algorithms/dfs_engine.h>
#include <core/search_algorithms/astar_engine.h>
#include <core/search_algorithms/batched_engine.h>
#include <core/problem_instance_generator.h>
#include <core/algorithm/graph.h>
#include <sliding_puzzle/sliding_puzzle.h>
#include <sliding_puzzle/heuristic.h>
#include <core/transition_system.h>
#include <array>
#include <functional>
#include <sstream>

namespace std {
template<>
class hash<pair<int, int>> {
public:
    size_t operator()(const pair<int, int> &s) const
    {
		return s.first ^ ( s.second << 1 );
    }
};
}

vector<pair<int, int>> solve_ladder(const vector<int> & horizontal_edges, pair<int, int> init_node, pair<int, int> last_node)
{
	vector<pair<int, int>> result;

	result.push_back(init_node);

	auto cur_node = init_node;

	while(cur_node != last_node)
	{
		pair<int, int> local_target_node = (cur_node.first < horizontal_edges.size()) ? make_pair(cur_node.first, horizontal_edges[cur_node.first]) : last_node;

		int sign = cur_node.second < local_target_node.second ? 1 : -1;
		while(cur_node.second != local_target_node.second)
		{
			cur_node.second += sign;
			result.push_back(cur_node);
		}

		if(cur_node == last_node)
			break;

		cur_node.first += 1;
		result.push_back(cur_node);
	}
	

	return result;
}

void test_random_ladder(int ladder_size)
{
	/*
	Graph N x N with all vertical edges, but only a one horizontal per column.
	We will call it "Ladder".
	*/

	vector<int> horizontal_edges;
	for(int i=0; i < ladder_size; ++i)
		horizontal_edges.push_back(i);
	
	std::random_shuffle(horizontal_edges.begin(), horizontal_edges.end());

	//Solve the problem with problem-oriented function
	auto correct_path = solve_ladder(horizontal_edges, make_pair(0, 0), make_pair(ladder_size, ladder_size));

	//Now solve the problem with tested search engines
	typedef implicit_graph<pair<int, int>> graph_t;
	graph_t::vertex_streamer_t streamer;

	graph_t graph([=](const pair<int, int> & vertex){
		std::list<pair<int, int>> res;

		if(vertex.second > 0)
			res.push_back(make_pair(vertex.first, vertex.second - 1));

		if(vertex.second < horizontal_edges.size())
			res.push_back(make_pair(vertex.first, vertex.second + 1));

		//Forward edge
		if((vertex.first) < ladder_size && (horizontal_edges[vertex.first] == vertex.second))
			res.push_back(make_pair(vertex.first + 1, vertex.second));

		//Backward edge
		if((vertex.first > 0) && (horizontal_edges[vertex.first-1] == vertex.second))
			res.push_back(make_pair(vertex.first - 1, vertex.second));

		return res;
	});

	{
		dfs_engine<graph_t> eng(streamer);
		std::vector<pair<int, int>> path;
		bool found = eng(graph, make_pair(0, 0), [=](const pair<int, int> & vertex){
			return vertex == make_pair(ladder_size, ladder_size);
		}, path);

		assert_test(found, "DFS (" + to_string(ladder_size) + " ladder) failed to find existing solution");
		assert_test(compare_paths(correct_path, path), "DFS (" + to_string(ladder_size) + " ladder) invalid solution");
	}

	{
		bfs_engine<graph_t> eng(streamer);
		std::vector<pair<int, int>> path;
		bool found = eng(graph, make_pair(0, 0), [=](const pair<int, int> & vertex){
			return vertex == make_pair(ladder_size, ladder_size);
		}, path);

		assert_test(found, "BFS (" + to_string(ladder_size) + " ladder) failed to find existing solution");
		assert_test(compare_paths(correct_path, path), "BFS (" + to_string(ladder_size) + " ladder) invalid solution");
	}
}

void test_implicit_graph_search()
{
	test_random_ladder(10);
	test_random_ladder(30);
	//test_random_ladder(100);
}

template<int PuzzleW, int PuzzleH>
void test_sliding_puzzle_blind()
{
	typedef transition_system<sliding_puzzle> puzzle_t;
	puzzle_t pr_puzzle(make_pair(PuzzleW, PuzzleH));
//	assert_test(pr_puzzle.is_solved(), "Sliding puzzle invalid initial state");
	
	problem_instance_generator<puzzle_t> instance_gen;

	auto initial_state = instance_gen.generate_state(pr_puzzle, 2 * PuzzleW * PuzzleH); //random_init(pr_puzzle, 2 * PuzzleW * PuzzleH);

	assert_test(pr_puzzle.has_solution(), "Sliding puzzle does not have solution");

	typedef transition_system_graph<puzzle_t> graph_t;
	graph_t::vertex_streamer_t streamer(pr_puzzle);

	graph_t graph(pr_puzzle);

	//Blind search
	cout << "Checking " << PuzzleW << "x" << PuzzleH << " puzzle blind search...";
	{
		auto puzzle = pr_puzzle;
		dfs_engine<graph_t> eng(streamer);
		std::vector<puzzle_t::state_t> path;

		bool found = eng(graph, initial_state, [=](const puzzle_t::state_t & puzzle_state){
			return puzzle_state == puzzle.solved_state();
		}, path);
		assert_test(found, "DFS (puzzle) failed to find existing solution");

		auto plan = puzzle.build_transition_path(path.begin(), path.end());
		assert_test(puzzle.verify_solution(initial_state, plan), "DFS (puzzle): Solution verification failed.");
	}

	{
		auto puzzle = pr_puzzle;
		bfs_engine<graph_t> eng(streamer);
		std::vector<puzzle_t::state_t> path;

		bool found = eng(graph, initial_state, [=](const puzzle_t::state_t & puzzle_state){
			return puzzle_state == puzzle.solved_state();
		}, path);
		assert_test(found, "BFS (puzzle) failed to find existing solution");

		//Verify plan
		auto plan = puzzle.build_transition_path(path.begin(), path.end());
		assert_test(puzzle.verify_solution(initial_state, plan), "BFS (puzzle): Solution verification failed.");
	}
	cout << "done." << std::endl;
}

template<int PuzzleWidth, int PuzzleHeight>
void test_sliding_puzzle_heuristic()
{
	typedef transition_system<sliding_puzzle> puzzle_t;

	puzzle_t pr_puzzle(make_pair(PuzzleWidth, PuzzleHeight));
//	assert_test(pr_puzzle.is_solved(), "Sliding puzzle invalid initial state");
	
	problem_instance_generator<puzzle_t> instance_gen;
	auto initial_state = instance_gen.generate_state(pr_puzzle, 2 * PuzzleWidth * PuzzleHeight); // random_init(pr_puzzle, 2 * PuzzleWidth * PuzzleHeight);
	
	assert_test(pr_puzzle.has_solution(), "Sliding puzzle does not have solution");

	typedef transition_system_graph<puzzle_t> graph_t;
	graph_t::vertex_streamer_t streamer(pr_puzzle);

	graph_t graph(pr_puzzle);


	//Heuristic search
	cout << "Checking " << PuzzleWidth << "x" << PuzzleHeight << " puzzle heuristic search...";

/*	{
		puzzle_t puzzle = pr_puzzle;

		astar_engine<puzzle_t::state_t, fringe_db_heuristic<puzzle_t>, false> eng(graph);
		std::vector<puzzle_t::state_t> path;

		bool found = eng(graph, initial_state, [&](const puzzle_t::state_t & state){
			return puzzle.is_solved(state);
		}, path);
		assert_test(found, "A*-patternDB (puzzle) failed to find existing solution");

		//Verify plan
		auto plan = puzzle.build_transition_path(path.begin(), path.end());
		assert_test(puzzle.verify_solution(initial_state, plan), "A*-patternDB (puzzle): Solution verification failed.");
	}*/

	{
		puzzle_t puzzle = pr_puzzle;
		astar_engine<graph_t, manhattan_heuristic<puzzle_t>, false> eng(streamer);
		std::vector<puzzle_t::state_t> path;

		bool found = eng(graph, initial_state, [&](const puzzle_t::state_t & state){
			return puzzle.is_solved(state);
		}, path);
		assert_test(found, "A*-manhatten (puzzle) failed to find existing solution");

		//Verify plan
		auto plan = puzzle.build_transition_path(path.begin(), path.end());
		assert_test(puzzle.verify_solution(initial_state, plan), "A*-manhatten (puzzle): Solution verification failed.");
		cout << "A* plan length: " << plan.size() << std::endl;
	}

	{
		auto puzzle = pr_puzzle;
		batched_engine<graph_t, manhattan_heuristic<puzzle_t>, false> eng(streamer);
		std::vector<puzzle_t::state_t> path;

		bool found = eng(graph, initial_state, [&](const puzzle_t::state_t & state){
			return puzzle.is_solved(state);
		}, path);
		assert_test(found, "BA*-manhatten (puzzle) failed to find existing solution");

		//Verify plan
		auto plan = puzzle.build_transition_path(path.begin(), path.end());
		assert_test(puzzle.verify_solution(initial_state, plan), "BA*-manhatten (puzzle): Solution verification failed.");
		cout << "BA* plan length: " << plan.size() << std::endl;
	}

	cout << "done." << std::endl;
}

void test_heuristics()
{
	typedef transition_system<sliding_puzzle> puzzle_t;
	

	{
		auto puzzle_size = make_pair(4, 4);
		puzzle_t pr_puzzle(puzzle_size);

		fringe_db_heuristic<puzzle_t> pdb_heuristic(pr_puzzle);

		/*
		State:
		5 10 14 7
		8 3 6 1
		15 0 12 9
		2 11 4 13

		Expected value: 31
		*/
		auto state = puzzle_t::state_t(puzzle_size);

		std::stringstream ss;
		ss << "5 10 14 7 " << "8 3 6 1 " << "15 0 12 9 " << "2 11 4 13 ";

		pr_puzzle.deserialize_state(ss, state);

		float r = pdb_heuristic(state);
		assert_test(r == 31, "PDB heuristic failure.");
	}

	{
		auto puzzle_size = make_pair(3, 3);
		puzzle_t pr_puzzle(puzzle_size);

		manhattan_heuristic<puzzle_t> heuristic(pr_puzzle);

		/*
		State:
		7 2 4
		5 0 6
		8 3 1

		Expected value: 18
		*/

		auto state = puzzle_t::state_t(puzzle_size);

		std::stringstream ss;
		ss << "7 2 4 " << "5 0 6 " << "8 3 1 ";

		pr_puzzle.deserialize_state(ss, state);

		float r = heuristic(state);
		assert_test(r == 18.0f, "Manhattan heuristic check.");
	}

}

void test_hash()
{
	typedef transition_system<sliding_puzzle> puzzle_t;
	auto puzzle_size = make_pair(3, 5);
	auto state = puzzle_t::state_t(puzzle_size);

	std::hash<puzzle_t::state_t> hash;
	size_t res = hash(state);
	assert_test(res == 12312, "Hashing function.");
}

void test_search()
{
	test_hash();
	test_heuristics();
	test_implicit_graph_search();
	//test_sliding_puzzle_heuristic<1, 2>();
	test_sliding_puzzle_blind<2,2>();
	test_sliding_puzzle_blind<3,2>();
	test_sliding_puzzle_heuristic<4, 4>();
	test_sliding_puzzle_heuristic<5, 5>();
	test_sliding_puzzle_heuristic<3, 5>();
	test_sliding_puzzle_heuristic<3, 4>();
}
