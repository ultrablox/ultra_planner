
#include "test_helpers.h"
#include <core/search_algorithms/bfs_engine.h>
#include <core/search_algorithms/dfs_engine.h>
#include <core/search_algorithms/astar_engine.h>
#include <core/search_algorithms/batched_engine.h>
#include <core/algorithm/graph.h>
#include <sliding_puzzle/sliding_puzzle.h>
#include <sliding_puzzle/heuristic.h>
#include <core/transition_system.h>
#include <array>
#include <functional>
#include <sstream>

void test_explicit_graph_search()
{
	typedef explicit_graph<int, int> graph_t;
	graph_t::vertex_streamer_t streamer;

	/*
	Simpliest graph.
	*/
	{
		std::array<int, 5> correct_path = {1, 2, 7, 3, 4};

		
		graph_t graph({ 1, 2, 3, 4, 5, 6, 7, 8 }, { { 1, 2 }, { 1, 5 }, { 2, 5 }, { 5, 6 }, { 2, 7 }, { 3, 7 }, { 3, 4 }, { 7, 8 } });

		{
			bfs_engine<graph_t> eng(streamer);
			std::vector<int> path;
			bool found = eng(graph, 1, [](int vertex){
				return vertex == 4;
			}, path);

			assert_test(found, "BFS (simple) failed to find existing solution");
			assert_test(compare_paths(correct_path, path), "BFS (simple) invalid solution");
		}

		{
			dfs_engine<graph_t> eng(streamer);
			std::vector<int> path;
			bool found = eng(graph, 1, [](int vertex){
				return vertex == 4;
			}, path);

			assert_test(found, "DFS (simple) failed to find existing solution");
			assert_test(compare_paths(correct_path, path), "DFS (simple) invalid solution");
		}
	}

	/*
	Spiral.
	*/
	{
		std::array<int, 25> correct_path = {21,22,23,24,25,20,15,10,5,4,3,2,1,6,11,16,17,18,19,14,9,8,7,12,13};

		graph_t graph({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 }, { { 13, 12 }, { 12, 7 }, { 7, 8 }, { 8, 9 }, { 9, 14 }, { 14, 19 }, { 19, 18 }, { 18, 17 }, { 17, 16 }, { 16, 11 }, { 11, 6 }, { 6, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 10 }, { 10, 15 }, { 15, 20 }, { 20, 25 }, { 25, 24 }, { 24, 23 }, { 23, 22 }, { 22, 21 } });

		{
			bfs_engine<graph_t> eng(streamer);
			std::vector<int> path;
			bool found = eng(graph, 21, [](int vertex){
				return vertex == 13;
			}, path);

			assert_test(found, "BFS (spiral) failed to find existing solution");
			assert_test(compare_paths(correct_path, path), "BFS (spiral) invalid solution");
		}

		{
			dfs_engine<graph_t> eng(streamer);
			std::vector<int> path;
			bool found = eng(graph, 21, [](int vertex){
				return vertex == 13;
			}, path);

			assert_test(found, "DFS (spiral) failed to find existing solution");
			assert_test(compare_paths(correct_path, path), "DFS (spiral) invalid solution");
		}
	}

	/*
	Christmas tree.
	*/
	{
		std::array<int, 7> correct_path = {15,9,10,18,12,20,14};

		graph_t graph({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 }, { { 15, 9 }, { 9, 3 }, { 9, 10 }, { 9, 17 }, { 17, 23 }, { 17, 24 }, { 10, 18 }, { 18, 11 }, { 11, 5 }, { 18, 12 }, { 12, 20 }, { 12, 19 }, { 19, 27 }, { 20, 13 }, { 20, 14 }, { 13, 7 } });

		{
			bfs_engine<graph_t> eng(streamer);
			std::vector<int> path;
			bool found = eng(graph, 15, [](int vertex){
				return vertex == 14;
			}, path);

			assert_test(found, "BFS (christmas) failed to find existing solution");
			assert_test(compare_paths(correct_path, path), "BFS (christmas) invalid solution");
		}

		{
			dfs_engine<graph_t> eng(streamer);
			std::vector<int> path;
			bool found = eng(graph, 15, [](int vertex){
				return vertex == 14;
			}, path);

			assert_test(found, "DFS (christmas) failed to find existing solution");
			assert_test(compare_paths(correct_path, path), "DFS (christmas) invalid solution");
		}
	}
}
