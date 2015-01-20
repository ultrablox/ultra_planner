
#include "sliding_puzzle.h"
#include <core/transition_system/transition_system.h>
#include <core/algorithm/graph.h>
#include <solver/search_algorithms/astar_engine.h>
#include <solver/search_algorithms/greedy_bfs_engine.h>
#include <solver/search_algorithms/batched_engine.h>
#include <solver/UExternalMemoryController.h>
#include <tclap/CmdLine.h>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace TCLAP;


template<typename T>
class state_space_solver
{
	typedef T transition_system_t;

	template<typename T>
	struct engine_generator
	{

	};


public:
	state_space_solver(transition_system_t & tr_sys)
		:m_system(tr_sys)
	{
		size_t state_power = tr_sys.max_state_count_10();

		m_maxStateCount = pow(10, state_power);
		cout << "Search space contains maximum 10^" << state_power << " states" << std::endl;
	}

	template<bool UseExternalMem>
	void solve(istream & in_file, const std::string & alg_name)
	{
		auto state = m_system.default_state();
		m_system.deserialize_state(in_file, state);

		//m_system.set_state(state);

		transition_system_graph<transition_system_t> graph(m_system);
		

		if (alg_name == "A*")
			solve_with_engine<astar_engine<transition_system_t::state_t, manhattan_heuristic<transition_system_t>, UseExternalMem>>(graph, state);
		else if (alg_name == "BA*")
			solve_with_engine<batched_engine<transition_system_t::state_t, manhattan_heuristic<transition_system_t>, UseExternalMem>>(graph, state);
		else if (alg_name == "GBFS")
			solve_with_engine<greedy_bfs_engine<transition_system_t::state_t, manhattan_heuristic<transition_system_t>, UseExternalMem>>(graph, state);
		
	}
private:

	template<typename EngType, typename GraphT, typename StateT>
	void solve_with_engine(const GraphT & graph, const StateT & initial_state)
	{
		EngType search_engine(graph);

		std::list<transition_system_t::transition_t> plan;
		std::thread monitor_thread([&](){

			std::chrono::milliseconds print_dura(10000);
			std::chrono::milliseconds dura(500);

			const int print_counter = print_dura.count() / dura.count();

			int counter = 0;

			cout << "Started solving monitor, interval " << dura.count() << " msecs" << std::endl;
			double last_explored_count = 0.0;
			while (!search_engine.finished())
			{
				if (!(counter--))
				{
					counter = print_counter;
				
					auto stats = search_engine.get_stats();

					cout << "=============Monitoring================" << std::endl;
					cout << stats;

					double explored_node_count = stats.state_count;
					size_t pow10_count = explored_node_count > 1 ? ceil(log10(explored_node_count)) : 0;

					cout << "Observed 10^" << pow10_count << " states (" << explored_node_count / m_maxStateCount << "%), speed = " << (explored_node_count - last_explored_count) / print_dura.count() * 1e3 << " nodes/sec" << std::endl;

					last_explored_count = explored_node_count;
				}

				std::this_thread::sleep_for(dura);
			}
		});


		cout << "Starting solving process..." << std::endl;
		plan = solve_puzzle<std::list<transition_system_t::transition_t>>(search_engine, m_system, graph, initial_state);

		std::ofstream out_file("plan.txt");
		for (auto p : plan)
			out_file << p << std::endl;

		monitor_thread.join();
	}

	template<typename Res, typename PuzzleT, typename Eng, typename GraphT>
	Res solve_puzzle(Eng && eng, PuzzleT & puzzle, GraphT & graph, const typename PuzzleT::state_t & initial_state)
	{
		typedef PuzzleT puzzle_t;

		std::vector<puzzle_t::state_t> path;

		auto start_tp = high_resolution_clock::now();

		bool found = eng(graph, initial_state, [&](const puzzle_t::state_t & state){
			return puzzle.is_solved(state);
		}, path);

		auto plan = puzzle.build_transition_path(path.begin(), path.end());

		auto end_tp = high_resolution_clock::now();
		double search_timecost = duration_cast<microseconds>(end_tp - start_tp).count() * 1e-6;

		std::ofstream stats_file("stats.txt");
		stats_file << "wall_time:" << search_timecost << std::endl;
		stats_file << "plan_length:" << plan.size() << std::endl;
		stats_file << eng.get_stats();

		return plan;
	}
private:
	transition_system_t & m_system;
	double m_maxStateCount;
};

void solve_fun(bool in_ext_mem, istream & in_file, const std::string & alg_name)
{
	//Read size
	int width, height;
	in_file >> width >> height;

	auto problem_size = make_pair(width, height);

	typedef transition_system<sliding_puzzle> puzzle_t;
	puzzle_t puzzle(problem_size);

	state_space_solver<puzzle_t> solver(puzzle);

	if (in_ext_mem)
		solver.solve<true>(in_file, alg_name);
	else
		solver.solve<false>(in_file, alg_name);
}

int main(int argc, const char ** argv)
{
	CmdLine cmd("Command description message", ' ', "1.0");
	cmd.setExceptionHandling(false);

	ValueArg<string> command_name("e", "command", "Command (generate, solve).", false, "solve", "string");
	cmd.add(command_name);

	//===============Generating==================

	ValueArg<int> puzzle_width("c", "width", "Puzzle width.", false, 4, "int");
	cmd.add(puzzle_width);

	ValueArg<int> puzzle_height("r", "height", "Puzzle height.", false, 4, "int");
	cmd.add(puzzle_height);

	ValueArg<int> permutations("p", "permutations", "Number of permutations.", false, 20, "int");
	cmd.add(permutations);

	//===============Solving=====================
	ValueArg<string> algorithm_name("a", "algorithm", "Algorithm (A*, BA*, GBFS).", false, "A*", "string");
	cmd.add(algorithm_name);

	ValueArg<string> heuristic_name("", "heuristic", "Heuristic (MD - manhatten distance, PDB - pattern database).", false, "PDB", "string");
	cmd.add(heuristic_name);

	ValueArg<string> storage_type("s", "storage", "Use type of storage (ext - external memory / int - internal memory).", false, "int", "string");
	cmd.add(storage_type);

	UnlabeledValueArg<std::string> problem_fn_param("file_name", "Problem description file name.", true, "", "string");
	cmd.add(problem_fn_param);

	try
	{
		cmd.parse(argc, argv);
	}
	catch(ArgException & e)
	{
		cout << "UltraPlanner failed to start.\n";
		cout << e.what();
		return 1;
	}

	if (command_name.getValue() == "solve")
	{
		std::ifstream in_file(problem_fn_param.getValue());
		if (!in_file)
		{
			cout << "Unable to open input file" << std::endl;
			return 1;
		}

		//Initialize external memory storage if needed
		std::unique_ptr<UExternalMemoryController> ext_memory_ctrl;
		if (storage_type.getValue() == "ext")
			ext_memory_ctrl.reset(new UExternalMemoryController());

		solve_fun(storage_type.getValue() == "ext", in_file, algorithm_name.getValue());
	}
	else if(command_name.getValue() == "generate")
	{
		auto problem_size = make_pair(puzzle_width.getValue(), puzzle_height.getValue());
		//sliding_puzzle::default_state(problem_size)
		transition_system<sliding_puzzle> puzzle(problem_size);

		auto random_state = random_init(puzzle, permutations.getValue());

		std::ofstream out_file(problem_fn_param.getValue());
		puzzle.serialize_state(out_file, random_state);
	}

	return 0;
}