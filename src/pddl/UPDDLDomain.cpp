#include "UPDDLDomain.h"
#include "UPDDLAction.h"
#include <core/utils/helpers.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "FlexLexer.h"
#include <ptree.h>
#include <TypedAnalyser.h>
#include <TimSupport.h>
#include <TIM.h>
#include <SASActions.h>
#include <instantiation.h>
#include <SimpleEval.h>

extern int yyparse();

using namespace std;

void UPDDLRequierment::print()
{
	switch (this->type)
	{
	case Strips:
		printf("strips");
		break;
	case Typing:
		printf("typing");
		break;
	case NegativePreconditions:
		printf("negative-preconditions");
		break;
	case DisjunctivePreconditions:
		printf("disjunctive-preconditions");
		break;
	case Equality:
		printf("equality");
		break;
	case NumericFluents:
		printf("numeric-fluents");
		break;
	case ActionCosts:
		printf("action-costs");
		break;
	case DurativeActions:
		printf("durative-actions");
		break;
	case DerivedPredicates:
		printf("derived-predicates");
		break;
	default:
		printf("unknown");
		break;
	}
}

UPDDLRequierment::UPDDLRequierment()
{

}

UPDDLRequierment::UPDDLRequierment(Type _type)
	:type(_type)
{

}

bool operator==(const UPDDLRequierment r1, const UPDDLRequierment r2)
{
	return r1.type == r2.type;
}
/*
UPDDLRequierments::UPDDLRequierments(size_t _count)
    :UPDDLArray<UPDDLRequierment>(_count)
{
}

bool UPDDLRequierments::contains(UPDDLRequierment::Type req_type)
{
	return UPDDLArray<UPDDLRequierment>::contains(UPDDLRequierment(req_type));
}

void UPDDLRequierments::print()
{
	printf("  Requierments (%ld):\n", this->count);

	for(size_t i = 0; i < this->count; ++i)
	{
		printf("    ");
		this->items[i].print();
		printf("\n");
	}
}


void UPDDLFunctions::print()
{
	printf("  Functions (%d):\n", this->count);

	for(size_t i = 0; i < this->count; ++i)
	{
		printf("    ");
		this->items[i]->print();
		printf("\n");
	}
}
*/

template<class T>
void delete_vector(std::vector<T*> & vec)
{
	for(auto & e : vec)
		delete e;
	vec.clear();
}

UPDDLDomain::UPDDLDomain(const std::string & _name)
    :UPDDLNamedObject(_name)
{
}

UPDDLDomain::~UPDDLDomain()
{
	delete_vector(predicates);
	delete_vector(functions);
	delete_vector(actions);
	delete_vector(constants);
}

void UPDDLDomain::print()
{
	//Domain
	printf("Domain \"%s\":\n", this->name.c_str());

	//Requierments
//	this->requerments.print();

	//Types
	this->typeTree.print();

	//Predicates
	cout << "Predicates:\n";
	for(auto p : predicates)
	{
		cout << "\t";
		p->print();
		cout << "\n";
	}

	//Functions
	cout << "Functions:\n";
	for(auto f : functions)
	{
		cout << "\t";
		f->print();
		cout << "\n";
	}

	//Actions
	cout << "Actions:\n";
	for(auto a : actions)
	{
		cout << "\t";
		a->print();
		cout << "\n";
	}

//	this->functions.print();
//	this->actions->print();
}

UPDDLPredicate * UPDDLDomain::predicate(const std::string & pred_name) const
{
	return find_by_name<UPDDLPredicate>(predicates, pred_name);
}

UPDDLPredicate * UPDDLDomain::createPredicate(const std::string & pred_name)
{
	UPDDLPredicate * new_pred = new UPDDLPredicate(pred_name);
	predicates.push_back(new_pred);
	return new_pred;
}

UPDDLFunction * UPDDLDomain::function(const std::string & func_name) const
{
	return find_by_name<UPDDLFunction>(functions, func_name);
}

void UPDDLDomain::addObject(UPDDLObject * obj)
{
	constants.push_back(obj);
}

UPDDLObject * UPDDLDomain::constantObject(const std::string & obj_name) const
{
	return find_by_name(constants, obj_name);
}

//=================UPDDLProblem===================
void UPDDLProblem::print()
{
}

void UPDDLProblem::addObject(UPDDLObject * object)
{
	objects.push_back(object);
}

UPDDLObject * UPDDLProblem::object(const std::string & obj_name) const
{
	return find_by_name(objects, obj_name);
}


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
	if(!domainFile)
	{
		cout << "Unable to open domain file.\n";
		return nullptr;
	}
    
	yfl = new yyFlexLexer(&domainFile, &cout);
	yyparse();
	delete yfl;

	ifstream problemFile(problem_file_name);
	if(!problemFile)
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

void sas_generate()
{
	/*use_sasoutput = true;
	FunctionStructure fs;
	fs.normalise();
	fs.initialise();*/
}
