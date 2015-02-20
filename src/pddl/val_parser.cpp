
#include "val_parser.h"
#include "FlexLexer.h"
#include <ptree.h>
#include <TypedAnalyser.h>
#include <TimSupport.h>
#include <TIM.h>
#include <SASActions.h>
#include <instantiation.h>
#include <SimpleEval.h>
#include <fstream>

using namespace std;

extern int yyparse();

namespace VAL {

	parse_category* top_thing = NULL;


	analysis* current_analysis;

	yyFlexLexer* yfl;
	int Silent;
	int errorCount;
	bool Verbose;
	bool ContinueAnyway;
	bool ErrorReport;
	bool InvariantWarnings;
	bool LaTeX;

	ostream * report = &cout;

	TypeChecker * theTC;
};

char * current_filename;
/**/
using namespace VAL;
using namespace TIM;
using namespace SAS;
using namespace Inst;

extern int yydebug;

VAL::analysis* val_parse(const std::string & domain_file_name, const std::string & problem_file_name)
{
	analysis *res = new analysis;

	yydebug = 0;
	current_analysis = res;

	IDopTabFactory * fac = new IDopTabFactory;
	current_analysis->setFactory(fac);

	ifstream domainFile(domain_file_name);
	if (!domainFile)
	{
		cout << "Unable to open domain file.\n";
		return nullptr;
	}

	yfl = new yyFlexLexer(&domainFile, &cout);
	yyparse();
	delete yfl;

	ifstream problemFile(problem_file_name);
	if (!problemFile)
	{
		cout << "Unable to open problem file.\n";
		return nullptr;
	}

	yfl = new yyFlexLexer(&problemFile, &cout);
	yyparse();
	delete yfl;

	/*DurativeActionPredicateBuilder dapb;
	current_analysis->the_domain->visit(&dapb);


	TypePredSubstituter a;
	current_analysis->the_problem->visit(&a);
	current_analysis->the_domain->visit(&a);

	char * filenames[2];
	filenames[0] = new char[domain_file_name.size()+1];
	filenames[1] = new char[problem_file_name.size()+1];
	//filenames[0][domain_file_name.size()] = 0;
	//filenames[1][problem_file_name.size()] = 0;

	memcpy(filenames[0], domain_file_name.c_str(), domain_file_name.length()+1);
	memcpy(filenames[1], problem_file_name.c_str(), problem_file_name.length()+1);

	performTIMAnalysis(filenames);

	for_each(TA->pbegin(),TA->pend(), ptrwriter<PropertySpace>(cout,"\n"));
	for_each(TA->abegin(),TA->aend(), ptrwriter<PropertySpace>(cout,"\n"));

	auto tx = TA;*/
	return res;
}
