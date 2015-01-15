#include "UPDDLDomain.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;

UPDDLType::UPDDLType(const std::string & _name, UPDDLType * parent_type)
	:UPDDLNamedObject(_name), parent(parent_type)
{

}

UPDDLType * UPDDLType::findChild(const std::string & type_name)
{
	if(name == type_name)
	{
		return this;
	}
	else
	{
		for(auto c : children)
		{
			auto res = c->findChild(type_name);
			if(res)
				return res;
		}

		return nullptr;
	}
}

void UPDDLType::createChild(const std::string & child_name)
{
	auto new_type = new UPDDLType(child_name, this);
	children.push_back(new_type);
}

void UPDDLType::print() const
{
	cout << "(" << name;

	if(!children.empty())
	{
		cout << " {";
		
		for(auto c : children)
		{
			c->print();
			cout << ", ";
		}

		cout << "}";
	}

	cout << ")";
}

UPDDLTypeTree::UPDDLTypeTree()
//	:UPDDLArray<UPDDLType*>(2)
{

	//Create base type
	root_type = new UPDDLType("root_type", nullptr);

	//Two common types
	root_type->children.push_back(new UPDDLType("object", root_type));
	root_type->children.push_back(new UPDDLType("number", root_type));
}

UPDDLType * UPDDLTypeTree::type(const std::string & _name) const
{
	return root_type->findChild(_name);
}

bool UPDDLTypeTree::addTypes(const std::vector<std::string> & type_names, const std::string & base_type_name)
{
	UPDDLType * base_type = root_type->findChild(base_type_name);
	if(!base_type)
		return false;

	for(auto tn : type_names)
		base_type->createChild(tn);

	return true;
}

void UPDDLTypeTree::print()
{
	cout << "Types: ";
	root_type->print();
	cout << "\n";
}

UPDDLTypedObject::UPDDLTypedObject(const UPDDLType * _type)
	:type(_type)
{

}
