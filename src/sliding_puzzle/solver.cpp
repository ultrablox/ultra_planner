
#include "sliding_puzzle.h"
#include "heuristic.h"
#include <core/transition_system.h>
#include <core/algorithm/graph.h>
#include <core/state_space_solver.h>
#include <tclap/CmdLine.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace TCLAP;

int main(int argc, const char ** argv)
{
	CmdLine cmd("Command description message", ' ', "1.0");
	cmd.setExceptionHandling(false);

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
		cout << "Puzzle Solver failed to start.\n";
		cout << e.what();
		return 1;
	}

	std::ifstream in_file(problem_fn_param.getValue());
	if (!in_file)
	{
		cout << "Unable to open input file" << std::endl;
		return 1;
	}

	//Read size
	using puzzle_t = transition_system<sliding_puzzle>;
	std::ofstream plan_file("plan.txt");
	state_space_solver<puzzle_t> solver(in_file, plan_file);

	bool r = solver.solve<manhattan_heuristic>(storage_type.getValue() == "ext", algorithm_name.getValue());
	if (!r)
		cout << "Solution not found!" << std::endl;

	return 0;
}
