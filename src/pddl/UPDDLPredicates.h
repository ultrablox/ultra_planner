
#ifndef UltraPlanner_UPDDLPredicates_h
#define UltraPlanner_UPDDLPredicates_h

#include "config.h"
#include "UPDDLNamedObject.h"
#include "UPDDLTypes.h"
#include "UPDDLParametrizedObject.h"
#include "common_objects.h"
#include <vector>
#include <iostream>

struct UPDDLObject;
struct UPDDLDomain;
struct UPDDLProblem;

struct ULTRA_PDDL_API UPDDLPredicate : public UPDDLNamedObject, public UPDDLParametrizedObject
{
	UPDDLPredicate(const std::string & name);
	
	void print(std::ostream & os = std::cout);
};

struct ULTRA_PDDL_API UPDDLPredicateInstance : public UPDDLTypedInstance<UPDDLPredicate>
{
	UPDDLPredicateInstance(UPDDLPredicate * _object = nullptr, const std::vector<UPDDLObject*> & _params = std::vector<UPDDLObject*>());
	UPDDLPredicateInstance(const AtomicFormula & atomic_formula, const UPDDLDomain * domain, const UPDDLProblem * problem);

	ULTRA_PDDL_API friend bool operator<(const UPDDLPredicateInstance & i1, const UPDDLPredicateInstance & i2);
	//bool negative;	
};

#endif
