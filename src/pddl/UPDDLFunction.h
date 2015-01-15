
#ifndef UltraPlanner_UPDDLFunction_h
#define UltraPlanner_UPDDLFunction_h

#include "config.h"
#include "common_objects.h"
#include "UPDDLNamedObject.h"
#include "UPDDLParametrizedObject.h"
#include "translator/common_objects.h"
#include <iostream>

struct UPDDLDomain;
struct UPDDLProblem;
struct UPDDLObject;

struct ULTRA_PDDL_API UPDDLFunction : public UPDDLNamedObject, public UPDDLParametrizedObject
{
	UPDDLFunction(const std::string & _name);
	void print(std::ostream & os = std::cout, bool new_line = true);
};

struct ULTRA_PDDL_API UPDDLFunctionInstance : public UPDDLTypedInstance<UPDDLFunction>
{
	//UPDDLFunctionInstance();
	UPDDLFunctionInstance(const FunctionTerm & function_term, const UPDDLDomain * domain, const UPDDLProblem * problem);
	UPDDLFunctionInstance(UPDDLFunction * _object = nullptr, const std::vector<UPDDLObject*> & _params = std::vector<UPDDLObject*>());

	float value;
};

#endif
