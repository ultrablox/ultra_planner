#ifndef UltraPlanner_UPDDLParametrizedObject_h
#define UltraPlanner_UPDDLParametrizedObject_h

#include "config.h"
#include "UPDDLNamedObject.h"
#include "UPDDLTypes.h"
#include "translator/common_objects.h"
#include <string>
#include <vector>
#include <iostream>

struct UPDDLObject;

struct ULTRA_PDDL_API UPDDLParameter : public UPDDLNamedObject, public UPDDLTypedObject
{
	UPDDLParameter(const std::string & name, const UPDDLType * type);
	virtual bool isConstant() const;
	void print();
};

struct ULTRA_PDDL_API UPDDLConstantParameter : public UPDDLParameter
{
	UPDDLConstantParameter(UPDDLObject * constant_object);
	virtual bool isConstant() const override;
	UPDDLObject * constantObject;
};

struct ULTRA_PDDL_API UPDDLParametrizedObject
{
	void addParameter(const std::string & param_name, const UPDDLType * type);
	void print(std::ostream & os = std::cout);
	void loadParams(const TypedList & typed_list, const UPDDLTypeTree & types);

	/*
	Looks for parameter with given name and return it.
	*/
	UPDDLParameter * parameter(const std::string & param_name) const;
	UPDDLParameter * parameter(const int param_index) const;

	std::vector<UPDDLParameter*> parameters;
};


#endif
