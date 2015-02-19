
//#include <vld.h>
#include <pddl/translator/UGrammar.h>
#include <pddl/UPDDLGrammar.h>
#include <pddl/ULanguage.h>
#include <pddl/UPDDLDomain.h>
#include <pddl/translator/ULexer.h>
#include <pddl/translator/UParser.h>
#include <pddl/UTaskGenerator.h>
#include <solver/USolver.h>
//#include <solver/UExternalMemoryController.h>
//#include <multicore_solver/UMultiCoreSolver.h>
#include "planner/UPDDLSolver.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <memory>
#include <tclap/CmdLine.h>
//#include <ezOptionParser.hpp>
//#include <stxxl.h>

using namespace TCLAP;


#ifndef __APPLE__
    #include <conio.h>
#endif
/*
size_t get_file_size(const std::string & file_name)
{
	struct stat filestatus;
	stat(file_name, &filestatus);

	return filestatus.st_size;
}*/

LexTree parse_pddl_file(const std::string & file_name)
{
	UPDDLLanguage pddlLanguage;
	ULexer lexer(&pddlLanguage);
	return lexer.parseFile(file_name);
}

int main(int argc, const char ** argv)
{
	CmdLine cmd("Command description message", ' ', "0.9");
	cmd.setExceptionHandling(false);

	ValueArg<size_t> solver_output_size("s", "solver_max_ouput_size", "Number of maximum solver ouput buffer.", false, 300000ULL, "size_t");
	cmd.add(solver_output_size);

	ValueArg<int> optimal("o", "optimal", "Search optimal plan.", false, 1, "int");
	cmd.add(optimal);

	ValueArg<string> solver_type("t", "solver_type", "Type of using solver (st - singlethread, mt - multicore, cuda - using CUDA.", false, "st", "string");
	cmd.add(solver_type);

	ValueArg<string> ignored_drives("d", "ignored_drives", "Ignored hard drives. These drives will not be used for external search data storage.", false, "A,C", "string");
	cmd.add(ignored_drives);

	ValueArg<string> ext_mem_subdir("e", "external_data_subdir", "Subfolder from drive root, which will be used to create external data storage.", false, "ultra_planner", "string");
	cmd.add(ext_mem_subdir);

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

	auto parse_result = val_parse(domain_fn_param.getValue(), problem_fn_param.getValue());

	auto planning_task = UTaskGenerator::generateTask(parse_result);
	get<3>(planning_task) = optimal.getValue();

	if(solver_type.getValue() == "st")
	{
		UPDDLSolver<USimpleSolver> solver(solver_output_size.getValue());
		solver.solve(planning_task);
	}
	else if(solver_type.getValue() == "mt")
	{
#ifndef __APPLE__
		UPDDLSolver<UMultiCoreSolver> solver(solver_output_size.getValue());
		solver.solve(planning_task);
#endif
	}
	else if(solver_type.getValue() == "cuda")
	{
		//solveWithSolver<UCUDASolver>(domain, problem);
	}

#ifndef __APPLE__
	//getch();
#endif

	return 0;
}
