
#include "sliding_puzzle.h"
#include <core/transition_system/transition_system.h>
#include <core/algorithm/graph.h>
#include <solver/state_space_solver.h>
#include <core/UExternalMemoryController.h>
#include <tclap/CmdLine.h>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace TCLAP;


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

	//ValueArg<string> heuristic_name("", "heuristic", "Heuristic (MD - manhatten distance, PDB - pattern database).", false, "PDB", "string");
	//cmd.add(heuristic_name);

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

		//Read size

		using puzzle_t = transition_system<sliding_puzzle>;
		state_space_solver<puzzle_t> solver(in_file, cout);

		bool r = solver.solve(storage_type.getValue() == "ext", algorithm_name.getValue());
		if (!r)
			cout << "Solution not found!" << std::endl;
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