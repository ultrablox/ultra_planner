#ifndef UltraPlanner_UPDDLTypes_h
#define UltraPlanner_UPDDLTypes_h

#include <stddef.h>
#include <vector>
#include <stdlib.h>
#include <string>
//#include "UPDDLDomain.h"
#include "UPDDLNamedObject.h"

struct UPDDLType : public UPDDLNamedObject
{
	UPDDLType(const std::string & name, UPDDLType * parent_type);
	UPDDLType * findChild(const std::string & type_name);
	void createChild(const std::string & child_name);
	void print() const;

	UPDDLType * parent;
	std::vector<UPDDLType*> children;
};

struct UPDDLTypeTree
{
	UPDDLTypeTree();
	UPDDLType * type(const std::string & name) const;
	bool addTypes(const std::vector<std::string> & type_names, const std::string & base_type_name);
	void print();

	UPDDLType * root_type;
};

struct UPDDLTypedObject
{
	UPDDLTypedObject(const UPDDLType * _type);
	const UPDDLType * type;
};


#endif