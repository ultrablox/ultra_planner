
#ifndef UltraPlanner_UPDDLAction_h
#define UltraPlanner_UPDDLAction_h

#include "config.h"
#include "common_objects.h"
#include "UPDDLNamedObject.h"
#include "UPDDLParametrizedObject.h"
#include <core/transition_system/UTransitionSystem.h>

struct UPDDLPredicate;
struct UPDDLFunction;

struct UPDDLAtomicFormula
{
	void print();

	UPDDLPredicate * predicate;
	std::vector<UPDDLParameter*> params;
};

/*
Expression tree.
*/
struct UPDDLGoalDescription
{
	void print();

	UPDDLPredicate * predicate;
	std::vector<UPDDLParameter*> params;
};


struct UPDDLCondition
{
	void print();
	std::vector<UPDDLGoalDescription*> gds;
};

struct ULTRA_PDDL_API UPDDLEffect
{
	enum class Type {Atomic, Numeric};

	virtual Type type() const = 0;
};

struct ULTRA_PDDL_API UPDDLAtomicEffect : public UPDDLEffect
{
	virtual Type type() const;

	bool negative;
	UPDDLPredicate * predicate;
	std::vector<UPDDLParameter*> params;
};

struct ULTRA_PDDL_API UPDDLNumericEffect : public UPDDLEffect
{
	virtual Type type() const;

	UNumericEffect::Type numericType;
	UPDDLFunction * function;
	
	
	struct
	{
		UPDDLFunction * func;
		std::vector<UPDDLParameter*> params;
		float value;
	} expression;

	std::vector<UPDDLParameter*> params;
};

struct UPDDLFormula
{

};

/*
struct UPDDLPreGoalDescription
{
	enum class Type {Universal, Preference, Simple};

	Type type;
};*/

/*
Standart non-temporal action with zero-duration.
*/
class ULTRA_PDDL_API UPDDLAction : public UPDDLNamedObject, public UPDDLParametrizedObject
{
public:
	UPDDLAction(const std::string & _name);
	void print(std::ostream & os = std::cout);

	bool usesInEffect(const UPDDLPredicate * pred) const;
	bool usesInEffect(const UPDDLFunction * func) const;
	bool usesInCondition(const UPDDLPredicate * pred) const;
	bool usesInCondition(const UPDDLFunction * func) const;

	//UPDDLCondition precondition;
	std::vector<UPDDLEffect*> effects;
	std::vector<UPDDLGoalDescription*> conditions;
};

struct UPDDLActionInstance : public UPDDLTypedInstance<UPDDLAction>
{
};

#endif
