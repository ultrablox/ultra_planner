
#include "sliding_puzzle.h"
#include <core/transition_system.h>
#include <core/problem_instance_generator.h>
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

	//===============Generating==================

	ValueArg<int> puzzle_width("c", "width", "Puzzle width.", false, 4, "int");
	cmd.add(puzzle_width);

	ValueArg<int> puzzle_height("r", "height", "Puzzle height.", false, 4, "int");
	cmd.add(puzzle_height);

	ValueArg<int> permutations("p", "permutations", "Number of permutations.", false, 20, "int");
	cmd.add(permutations);

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
	
	std::ofstream out_file(problem_fn_param.getValue());
	
	using problem_t = transition_system<sliding_puzzle>;
	problem_instance_generator<problem_t> problem_generator;
	problem_t::size_description_t problem_size(puzzle_width.getValue(), puzzle_height.getValue());
	
	problem_generator.generate(problem_size, permutations.getValue(), out_file);

	return 0;
}