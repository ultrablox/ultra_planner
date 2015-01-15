
#ifndef UPDDL_UBoolTask_h
#define UPDDL_UBoolTask_h

#include <string>
#include <vector>
#include <memory>

struct UNamedObject
{
	UNamedObject(const std::string & _name)
		:name(_name)
	{}

	std::string name;
};


namespace BoolTask
{

struct Type : public UNamedObject
{
	Type(const std::string & _name, Type * _parent = nullptr)
		:UNamedObject(_name), p_parent(_parent)
	{}

	~Type()
	{
		for(auto c : children)
			delete c;
		children.resize(0);
	}

	void addChild(const std::string & _name)
	{
		children.push_back(new Type(_name, this));
	}

	Type * p_parent;
	std::vector<Type*> children;
};

struct Object : public UNamedObject
{
	Object(const std::string & _name, Type * _type)
		:UNamedObject(_name), p_type(_type)
	{}

	Type * p_type;
};

struct TypedParametrizedObject
{
	typedef std::pair<Type*, std::string> ParamT;
	std::vector<ParamT> params;
};

struct UntypedParametrizedObject
{
	UntypedParametrizedObject(int param_count)
		:paramCount(param_count)
	{}

	int paramCount;
};

struct Predicate : public UNamedObject
{
	Predicate(const std::string & _name)
		:UNamedObject(_name)
	{}
};

struct TypedPredicate : public Predicate, public TypedParametrizedObject
{
	TypedPredicate(const std::string & _name, int param_count)
		:Predicate(_name)
	{
		//params.resize(param_count);
	}
};

struct UntypedPredicate : public Predicate, public UntypedParametrizedObject
{
	UntypedPredicate(const std::string & _name, int param_count)
		:Predicate(_name), UntypedParametrizedObject(param_count)
	{}
};

struct BoolVariable
{
	//Predicate which is instance-base
	int m_predicateIndex;

	//Indices of corresponding objects
	std::vector<int> m_paramIndices;
};

struct Formula
{
	Formula(const Predicate * pred, int param_count = 0)
		:predicate(pred)
	{
		if(param_count > 0)
		{
			params.resize(param_count);
			memset(params.data(), -1, sizeof(int)*param_count);
		}
	}

	const Predicate * predicate;

	//Indices of params of parent object
	std::vector<int> params;
};

struct Action : public UNamedObject, public TypedParametrizedObject
{
	Action(const std::string & _name)
		:UNamedObject(_name)
	{}

	std::vector<Formula> preconditions;
	std::vector<int> m_deleteVars, m_addVars;
	std::vector<Formula> m_deleteEffects, m_addEffects;
};

struct Atom
{
	Atom(const UntypedPredicate * _pred)
		:pred(_pred)
	{}

	const UntypedPredicate * pred;
	std::vector<Object*> params;
};

struct UNormalizedBoolTask;

class UBoolTask
{
public:
	UBoolTask();

	//Type routines
	Type * type(const std::string & _name) const;
	void addType(const std::string & type_name, const std::string & parent_name = "");

	//Object routines
	void addObject(const std::string & obj_name, const std::string & type_name);

	TypedPredicate* addPredicate(const std::string & pred_name, int param_count);
	void addVariable(const std::string & pred_name, const std::vector<std::string> & params);
	void addAction(Action * act);

	int objectIndex(const std::string & obj_str) const;
	int predicateIndex(const std::string & pred_str) const;
	TypedPredicate * predicate(const std::string & pred_str) const;
	int varIndex(const std::string & var_string) const;

	std::string varToString(const BoolVariable & var) const;

	void toMultivaluedTask() const;

	/*
	Converts to normalized task, i.e.:
	1. Compiles out types.
	*/
	UNormalizedBoolTask normalize() const;
private:
	std::vector<Object*> m_objects;
	std::vector<TypedPredicate*> m_predicates;
	std::vector<BoolVariable> m_boolVars;
	std::vector<Action*> m_actions;
	std::shared_ptr<Type> m_rootType;
};


struct UntypedAction : public UNamedObject, public UntypedParametrizedObject
{
	UntypedAction(const std::string & _name, int param_count)
		:UNamedObject(_name), UntypedParametrizedObject(param_count)
	{}

	std::vector<Formula> preconditions;
	std::vector<Formula> m_deleteEffects, m_addEffects;
};

struct UNormalizedBoolTask
{
	std::vector<UntypedPredicate*> m_predicates;
	std::vector<Atom> m_initialAtoms;
	std::vector<UntypedAction*> m_actions;
};

};


#endif
