
#include "UPDDLPredicates.h"
#include "UPDDLTypes.h"
#include "UPDDLDomain.h"
#include <iostream>

using namespace std;

UPDDLPredicate::UPDDLPredicate(const std::string & _name)
	:UPDDLNamedObject(_name)
{

}

void UPDDLPredicate::print(std::ostream & os)
{
	os << "(" << this->name;
	UPDDLParametrizedObject::print(os);
	os << ")";
}
/*
void UPDDLPredicates::print()
{
	cout << "Predicates (" << predicates.size() << "):\n";

	for(auto p : predicates)
	{
		cout << "\t";
		p->print();
		cout << "\n";
	}
}
*/
UPDDLPredicateInstance::UPDDLPredicateInstance(UPDDLPredicate * _object, const std::vector<UPDDLObject*> & _params)
	:UPDDLTypedInstance<UPDDLPredicate>(_object, _params)
{

}

UPDDLPredicateInstance::UPDDLPredicateInstance(const AtomicFormula & atomic_formula, const UPDDLDomain * domain, const UPDDLProblem * problem)
{
//	negative = false;
	object = domain->predicate(atomic_formula.name_lexem->name);

	for(auto param : atomic_formula.param_list)
	{
		auto obj = problem->object(param->name);
		this->params.push_back(obj);
	}
}

bool operator<(const UPDDLPredicateInstance & i1, const UPDDLPredicateInstance & i2)
{
	if(i1.object < i2.object)
		return true;

	/*if(i1.params.size() < i2.params.size())
		return true;

	for(int i = 0; i < i1.params.size(); ++i)
	{
		if(i1.params[i] < i2.params[i])
			return true;
	}*/

	if(i1.params < i2.params)
		return true;

	return false;
}
