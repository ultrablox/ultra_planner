
#include "UPDDLFunction.h"
#include "UPDDLDomain.h"
#include <iostream>

using namespace std;

UPDDLFunction::UPDDLFunction(const std::string & _name)
	:UPDDLNamedObject(_name)
{

}
/*
void UPDDLFunction::addParameter(const char * _name, UPDDLType * _type)
{
	//UPDDLParameter * param = new UPDDLParameter(_name, _type);
	//parameters.append(param);
}
*/
void UPDDLFunction::print(std::ostream & os, bool new_line)
{
	os << "(" << name << " ";

	UPDDLParametrizedObject::print(os);

	if(new_line)
		os << ")";
}

UPDDLFunctionInstance::UPDDLFunctionInstance(const FunctionTerm & function_term, const UPDDLDomain * domain, const UPDDLProblem * problem)
{
	object = domain->function(function_term.function_name->name);
	value = atof(function_term.value_lexem->name.c_str());

	for(auto param : function_term.param_list)
	{
		auto obj = problem->object(param->name);
		params.push_back(obj);
	}
}

UPDDLFunctionInstance::UPDDLFunctionInstance(UPDDLFunction * _object, const std::vector<UPDDLObject*> & _params)
	:UPDDLTypedInstance<UPDDLFunction>(_object, _params), value(0.0f)
{

}
