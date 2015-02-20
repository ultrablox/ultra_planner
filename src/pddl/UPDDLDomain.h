#ifndef UltraPlanner_UPDDLDomain_h
#define UltraPlanner_UPDDLDomain_h

#include "config.h"
#include "UPDDLTypes.h"
#include "UPDDLPredicates.h"
#include "UPDDLFunction.h"
#include <stddef.h>
#include <vector>
#include <stdlib.h>
#include <string>

class UPDDLAction;
/*
template<typename T>
struct UPDDLArray
{
	UPDDLArray(size_t _count = 0)
		:items(0), count(_count)
	{
		setCount(_count);
		
		if(_count > 0)
		{
			memset(this->items, 0, _count*sizeof(T));
		}
	}

	void setCount(size_t new_count)
	{
		size_t total_size = new_count * sizeof(T);
		if(this->items)
			this->items = (T*)realloc(this->items, total_size);
		else
			this->items = (T*)malloc(total_size);

		this->count = new_count;
	}

	bool contains(T element)
	{
		for(size_t i = 0; i < count; ++i)
		{
			if(items[i] == element)
				return true;
		}

		return false;
	}

	void append(T element)
	{
		setCount(count+1);
		items[count - 1] = element;
	}

	T * items;
	size_t count;
};*/

struct UPDDLRequierment
{
    typedef enum {  Unknown,
                    Strips, //:strips
                    Typing, //:typing
                    NegativePreconditions, //:negative-preconditions
                    DisjunctivePreconditions, //:disjunctive-preconditions
                    Equality, //:equality
					NumericFluents, //:numeric-fluents
					ActionCosts, //:action-costs
					DerivedPredicates, //:derived-predicates
					DurativeActions,// :durative-actions
        Count} Type;
    
	UPDDLRequierment();
	UPDDLRequierment(Type type);
	void print();

    Type type;
};

bool operator==(const UPDDLRequierment r1, const UPDDLRequierment r2);

/*
struct UPDDLRequierments : public UPDDLArray<UPDDLRequierment>
{
    UPDDLRequierments();
	bool contains(const UPDDLRequierment::Type req_type);
    void print();
};

struct UPDDLFunctions : public UPDDLArray<UPDDLFunction*>
{
	void print();
};*/



/*
struct UPDDLActions : public UPDDLArray<UPDDLAction*>
{
	void print();
};
*/
struct ULTRA_PDDL_API UPDDLDomain : public UPDDLNamedObject
{
    UPDDLDomain(const std::string & _name);
	~UPDDLDomain();
    void print();

	//===========================================
	//Predicate management
	//===========================================
	UPDDLPredicate * predicate(const std::string & pred_name) const;
	UPDDLPredicate * createPredicate(const std::string & pred_name);

	UPDDLFunction * function(const std::string & func_name) const;
	void addObject(UPDDLObject * obj);
	UPDDLObject * constantObject(const std::string & obj_name) const;

	std::vector<UPDDLRequierment> requerments;
    //UPDDLRequierments requerments;
	UPDDLTypeTree typeTree;
	std::vector<UPDDLPredicate*> predicates;
	std::vector<UPDDLFunction*> functions;
	std::vector<UPDDLAction*> actions;

	std::vector<UPDDLObject*> constants;
	//UPDDLActions * actions;
};


struct UPDDLObject : public UPDDLTypedObject, public UPDDLNamedObject
{
	UPDDLObject(const std::string & _name, UPDDLType * _type)
		:UPDDLTypedObject(_type), UPDDLNamedObject(_name)
	{
	}
};

struct UPDDLStateDescription
{
	std::vector<UPDDLPredicateInstance> predicates;
	std::vector<UPDDLFunctionInstance> functions;
};

struct ULTRA_PDDL_API UPDDLProblem
{
	void print();
	void addObject(UPDDLObject * object);
	UPDDLObject * object(const std::string & obj_name) const;

	std::vector<UPDDLObject*> objects;

	UPDDLStateDescription initialState, goal;
};

#endif
