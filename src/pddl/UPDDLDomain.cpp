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

void sas_generate()
{
	/*use_sasoutput = true;
	FunctionStructure fs;
	fs.normalise();
	fs.initialise();*/
}
