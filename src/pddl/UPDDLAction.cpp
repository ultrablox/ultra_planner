
#include "UPDDLAction.h"
#include <iostream>

using namespace std;


UPDDLEffect::Type UPDDLAtomicEffect::type() const
{
	return Type::Atomic;
}

UPDDLEffect::Type UPDDLNumericEffect::type() const
{
	return Type::Numeric;
}

void UPDDLAtomicFormula::print()
{
	/*printf("%s(", this->predicate->name.c_str());

	for(size_t p = 0; p < this->count; ++p)
	{
		if(p > 0)
			printf(", ");
		printf("%s", this->items[p]->name.c_str());
	}

	printf(")");*/
}

void UPDDLGoalDescription::print()
{
	//root->print();
}

void UPDDLCondition::print()
{
	if(gds.begin() != gds.end())
	{
		printf("      preconditions: ");

		if(gds.size() > 1)
		{
			printf("and ");
		}
		
		for(auto gd : gds)
		{
			gd->print();
			printf(" ");
		}

		printf("\n");
	}
}

UPDDLAction::UPDDLAction(const std::string & _name)
	:UPDDLNamedObject(_name), UPDDLParametrizedObject()
{

}


void UPDDLAction::print(std::ostream & os)
{
	os << "(" << name << " ";
	UPDDLParametrizedObject::print(os);
	os << ")";

}

bool UPDDLAction::usesInEffect(const UPDDLPredicate * pred) const
{
	for(auto effect : effects)
	{
		switch(effect->type())
		{
		case UPDDLEffect::Type::Atomic:
			{
				UPDDLAtomicEffect * aeffect = static_cast<UPDDLAtomicEffect*>(effect);
				if(aeffect->predicate == pred)
					return true;
				break;
			}
		}
	}

	return false;
}

bool UPDDLAction::usesInEffect(const UPDDLFunction * func) const
{
	for(auto effect : effects)
	{
		switch(effect->type())
		{
		case UPDDLEffect::Type::Numeric:
			{
				UPDDLNumericEffect * neffect = static_cast<UPDDLNumericEffect*>(effect);
				if(neffect->function == func)
					return true;
				break;
			}
		}
	}

	return false;
}

bool UPDDLAction::usesInCondition(const UPDDLPredicate * pred) const
{
	for(auto gd : conditions)
	{
		if(gd->predicate == pred)
			return true;
	}

	return false;
}

bool UPDDLAction::usesInCondition(const UPDDLFunction * func) const
{
	return false;
}
