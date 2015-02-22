
//#include <vld.h>
#include <pddl/planning_task_converter.h>
#include <core/state_space_solver.h>
#include <core/transition_system.h>
#include "goal_count_heuristic.h"
#include "planning_graph_heuristic.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <memory>
#include <tclap/CmdLine.h>

using namespace TCLAP;


struct planning_goal_checker
{
	planning_goal_checker(const combinedvar_system::masked_state_t & _goal)
	:m_goal(_goal.bool_part)
	{
	}

	bool operator()(const combinedvar_system_base::state_t & state) const
	{
		return false;
	}

	const masked_bit_vector m_goal;
};

int main(int argc, const char ** argv)
{
	CmdLine cmd("Command description message", ' ', "0.9");
	cmd.setExceptionHandling(false);

	ValueArg<string> algorithm_name("a", "algorithm", "Algorithm (A*, BA*, GBFS).", false, "A*", "string");
	cmd.add(algorithm_name);

	ValueArg<string> storage_type("s", "storage", "Use type of storage (ext - external memory / int - internal memory).", false, "int", "string");
	cmd.add(storage_type);

	/*ValueArg<size_t> solver_output_size("s", "solver_max_ouput_size", "Number of maximum solver ouput buffer.", false, 300000ULL, "size_t");
	cmd.add(solver_output_size);

	ValueArg<int> optimal("o", "optimal", "Search optimal plan.", false, 1, "int");
	cmd.add(optimal);

	ValueArg<string> solver_type("t", "solver_type", "Type of using solver (st - singlethread, mt - multicore, cuda - using CUDA.", false, "st", "string");
	cmd.add(solver_type);

	ValueArg<string> ignored_drives("d", "ignored_drives", "Ignored hard drives. These drives will not be used for external search data storage.", false, "A,C", "string");
	cmd.add(ignored_drives);

	ValueArg<string> ext_mem_subdir("e", "external_data_subdir", "Subfolder from drive root, which will be used to create external data storage.", false, "ultra_planner", "string");
	cmd.add(ext_mem_subdir);*/

	UnlabeledValueArg<string> domain_fn_param("domain_file_name", "Domain PDDL file name.", true, "", "string");
	cmd.add(domain_fn_param);

	UnlabeledValueArg<string> problem_fn_param("problem_file_name", "Problem PDDL file name.", true, "", "string");
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

	//UExternalMemoryController ext_ctrl(ignored_drives.getValue(), ext_mem_subdir.getValue());

	/*UPDDLLanguage pddlLanguage;
	UParser parser(&pddlLanguage);

	auto domain_tree = parse_pddl_file(domain_fn_param.getValue());
	UPDDLDomain * domain = parser.parseSafe(&UParser::parseDomain, domain_tree);
	if(!domain)
	{
		printf("Failed to load domain.");
		return 1;
	}

	//domain->print();

	auto problem_tree = parse_pddl_file(problem_fn_param.getValue());
	UPDDLProblem * problem = parser.parseSafe(&UParser::parseProblem, problem_tree, domain);

	if(problem)
		problem->print();
	else
	{
		printf("Failed to load problem.");
		return 2;
	}*/
	
	planning_task_t planning_task;

	planning_task_converter conv;
	int res = conv.get_planning_task(planning_task, domain_fn_param.getValue(), problem_fn_param.getValue());
	if (res != 0)
	{
		cout << "Failed to parse planning task" << std::endl;
		return 1;
	}

	planning_task.optimize();
	planning_task.varset_system.set_goal_state(planning_task.goal);

	using problem_t = transition_system<combinedvar_system>;
	state_space_solver<problem_t/*, planning_goal_checker*/> solver(planning_task.varset_system, cout, planning_task.initial_state);

	//bool r = solver.solve<goal_count_heuristic>(storage_type.getValue() == "ext", algorithm_name.getValue());
	bool r = solver.solve<planning_graph_heuristic>(storage_type.getValue() == "ext", algorithm_name.getValue());
	
	if (!r)
		cout << "Solution not found!" << std::endl;

	return 0;
}
