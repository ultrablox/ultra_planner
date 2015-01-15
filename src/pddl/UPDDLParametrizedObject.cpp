
#include "UPDDLParametrizedObject.h"
#include "UPDDLDomain.h"
#include <core/utils/helpers.h>
#include <iostream>
#include <algorithm>

using namespace std;

UPDDLParameter::UPDDLParameter(const std::string & _name, const UPDDLType * _type)
	:UPDDLNamedObject(_name), UPDDLTypedObject(_type)
{

}

bool UPDDLParameter::isConstant() const
{
	return false;
}

void UPDDLParameter::print()
{
	cout << name << " - " << type->name;
}

UPDDLConstantParameter::UPDDLConstantParameter(UPDDLObject * constant_object)
	:UPDDLParameter(constant_object->name, constant_object->type), constantObject(constant_object)
{
}

bool UPDDLConstantParameter::isConstant() const
{
	return true;
}

void UPDDLParametrizedObject::addParameter(const std::string & param_name, const UPDDLType * _type)
{
	UPDDLParameter * param = new UPDDLParameter(param_name, _type);
	parameters.push_back(param);
}

void UPDDLParametrizedObject::print(std::ostream & os)
{
	for(auto p : parameters)
	{
		p->print();
		os << ", ";
	}
}

void UPDDLParametrizedObject::loadParams(const TypedList & typed_list, const UPDDLTypeTree & types)
{
	for(auto param_group : typed_list.data)
	{
		auto param_type = types.type(param_group.first);

		for(auto param : param_group.second)
			addParameter(param, param_type);
	}
}

UPDDLParameter * UPDDLParametrizedObject::parameter(const std::string & param_name) const
{
	return find_by_name<UPDDLParameter>(parameters, param_name);
}

UPDDLParameter * UPDDLParametrizedObject::parameter(const int param_index) const
{
	return parameters[param_index];
}
