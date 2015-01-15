
#include "UBoolTask.h"
#include <core/utils/helpers.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <functional>

using namespace BoolTask;
using namespace std;

//==============================Bool task==============================
UBoolTask::UBoolTask()
	:m_rootType(new Type("root"))
{

}

Type * find_type(Type * node, const std::string & _name)
{
	if(node->name == _name)
		return node;
	else
	{
		for(auto c : node->children)
		{
			auto res = find_type(c, _name);
			if(res)
				return res;
		}
		return nullptr;
	}		
}

Type * UBoolTask::type(const std::string & _name) const
{
	return find_type(m_rootType.get(), _name);
}

void UBoolTask::addType(const std::string & type_name, const std::string & parent_name)
{
	//m_types.push_back(new Type(type_name));
	if(parent_name.empty())
	{
		m_rootType->addChild(type_name);
	}
	else
	{
		auto t = type(parent_name);
		if(!t)
			throw runtime_error("Type not found: " + parent_name);

		t->addChild(type_name);
	}
}

void UBoolTask::addObject(const std::string & obj_name, const std::string & type_name)
{
	m_objects.push_back(new Object(obj_name, type(type_name)));
}

TypedPredicate* UBoolTask::addPredicate(const std::string & pred_name, int param_count)
{
	auto new_pred = new TypedPredicate(pred_name, param_count);
	m_predicates.push_back(new_pred);
	return new_pred;
}

void UBoolTask::addVariable(const std::string & pred_name, const std::vector<std::string> & params)
{
	BoolVariable new_bv;

	new_bv.m_predicateIndex = predicateIndex(pred_name);

	for(auto & par : params)
		new_bv.m_paramIndices.push_back(objectIndex(par));

	m_boolVars.push_back(new_bv);
}

void UBoolTask::addAction(Action * act)
{
	m_actions.push_back(act);
}

int UBoolTask::objectIndex(const std::string & obj_str) const
{
	return 1;
	/*return std::distance(m_objects.begin(), std::find_if(m_objects.begin(), m_objects.end(), [=](const Object & obj){
			return obj.name == obj_str;
		}));*/
}

int UBoolTask::predicateIndex(const std::string & pred_str) const
{
	return std::distance(m_predicates.begin(), std::find_if(m_predicates.begin(), m_predicates.end(), [=](const TypedPredicate * pred){
		return pred->name == pred_str;
	}));
}

TypedPredicate * UBoolTask::predicate(const std::string & pred_str) const
{
	return find_by_name(m_predicates, pred_str);
}

int UBoolTask::varIndex(const std::string & var_string) const
{
	return std::distance(m_boolVars.begin(), std::find_if(m_boolVars.begin(), m_boolVars.end(), [=](const BoolVariable & var){
		return varToString(var) == var_string;
	}));
}

std::string UBoolTask::varToString(const BoolVariable & var) const
{
	std::string res = "(" + m_predicates[var.m_predicateIndex]->name;

	for(auto p_id : var.m_paramIndices)
		res += " " + m_objects[p_id]->name;

	return res + ")";
}

template<typename T>
bool cmp_without(const std::vector<T> & arr1, const std::vector<T> & arr2, int inv_index)
{
	bool res = true;
	for(int i = 0; i < arr1.size(); ++i)
	{
		if(i == inv_index)
			res = res && (arr1[i] != arr2[i]);
		else
			res = res && (arr1[i] == arr2[i]);
	}

	return res;
}

void UBoolTask::toMultivaluedTask() const
{
	//Check balancing
	//For predicate and it's parameter i
	for(int pred_id = 0; pred_id < m_predicates.size(); ++pred_id)
	{
		auto & pred = m_predicates[pred_id];

		cout << "Checking balancing for " << pred->name << "...";

		//Get actions referred to this predicate
		vector<int> referred_action_indices(m_actions.size());
		std::iota(referred_action_indices.begin(), referred_action_indices.end(), 0);
		auto last_it = std::remove_if(referred_action_indices.begin(), referred_action_indices.end(), [&](const int action_index){
			auto & act = m_actions[action_index];
			for(auto add_var : act->m_addVars)
				if(m_boolVars[add_var].m_predicateIndex == pred_id)
					return false;
			return true;
		});
		referred_action_indices.erase(last_it, referred_action_indices.end());

		cout << " total " << referred_action_indices.size() << " referred actions.\n";

		//This predicate is not changed - maybe it is constant?
		if(referred_action_indices.empty())
			continue;

		for(int param_index = 0; param_index < pred->params.size(); ++param_index)
		{
			cout << "Checking parameter " << param_index << "...";

			bool param_ballanced = true;
			for(auto rei : referred_action_indices)
			{
				//Find add effect for current predicate
				auto & act = m_actions[rei];
				auto add_it = std::find_if(act->m_addVars.begin(), act->m_addVars.end(), [&](const int var_index){
					return (m_boolVars[var_index].m_predicateIndex == pred_id);
				});

				//Get parameters list
				auto params = m_boolVars[*add_it].m_paramIndices;

				//Try to find corresponding delete effect for this set of parameters
				auto del_it = std::find_if(act->m_deleteVars.begin(), act->m_deleteVars.end(), [&](const int var_index){
					auto & bvar = m_boolVars[var_index];
					if(bvar.m_predicateIndex == pred_id)
					{
						if(cmp_without(params, bvar.m_paramIndices, param_index))
							return true;
					}

					return false;
				});

				if(del_it == act->m_deleteVars.end())
				{
					param_ballanced = false;
					//break;
				}
			}

			cout << param_ballanced << "\n";
		}
	}
}

UNormalizedBoolTask UBoolTask::normalize() const
{
	UNormalizedBoolTask norm_task;

	//Transfer base predicates
	for(auto pred : m_predicates)
	{
		auto new_pred = new UntypedPredicate(pred->name, pred->params.size());
		norm_task.m_predicates.push_back(new_pred);
	}

	//=================Compile away types==================

	/*
	Translate typed objects into new atom for initial state.
	Typed objects are translated into new atoms for the initial state. For example, the specification someobj - sometype
	leads to a new initial atom (sometype someobj), plus an additional atom (supertype someobj) for each supertype
	of sometype, including the universal supertype object.
	*/

	//Create predicate for each type
	std::function<void(Type*)> create_type_preds = [&](const Type * p_type){
		norm_task.m_predicates.push_back(new UntypedPredicate(p_type->name, 1));
		for(auto c : p_type->children)
			create_type_preds(c);
	};
	create_type_preds(m_rootType.get());

	//Process typed objects
	for(auto obj : m_objects)
	{
		auto p_type = obj->p_type;

		while(p_type != m_rootType.get())
		{
			auto pred = find_by_name(norm_task.m_predicates, p_type->name);
			Atom new_atom(pred);
			new_atom.params.push_back(obj);
			norm_task.m_initialAtoms.push_back(std::move(new_atom));
			p_type = p_type->p_parent;
		}
	}

	/*
	Compile out types from action preconditions.
	Typed operators are transformed by introducing new preconditions. For example, for an operator with parameter specification
	:parameters (?par1 - type1 ?par2 - type2) and precondition phi, the parameter specification is replaced
	by :parameters (?par1 ?par2) and the precondition is replaced by (and (type1 ?par1) (type2 ?par2) phi).
	*/
	for(auto & act : m_actions)
	{
		auto new_act = new UntypedAction(act->name, act->params.size());
		
		//Transfer preconditions
		new_act->preconditions.insert(new_act->preconditions.end(), act->preconditions.begin(), act->preconditions.end());

		//Add preconditions based on typed params
		for(int p = 0; p < act->params.size(); ++p)
		{
			auto & param = act->params[p];

			/*
			Typed operators are transformed by introducing new preconditions. For example, for an operator with parameter specification
			:parameters (?par1 - type1 ?par2 - type2) and precondition, the parameter specification is replaced
			by :parameters (?par1 ?par2) and the precondition is replaced by (and (type1 ?par1) (type2 ?par2)).
			*/
			
			auto pred = find_by_name(norm_task.m_predicates, param.first->name);
			
			Formula type_cond(pred);
			type_cond.params.push_back(p);

			new_act->preconditions.push_back(type_cond);
		}
		
		//Simply transfer effects
		for(auto & eff : act->m_addEffects)
		{
			auto pred = find_by_name(norm_task.m_predicates, eff.predicate->name);
			Formula new_eff(pred);
			new_eff.params.insert(new_eff.params.end(), eff.params.begin(), eff.params.end());
			new_act->m_addEffects.push_back(new_eff);
		}

		for(auto & eff : act->m_deleteEffects)
		{
			auto pred = find_by_name(norm_task.m_predicates, eff.predicate->name);
			Formula new_eff(pred);
			new_eff.params.insert(new_eff.params.end(), eff.params.begin(), eff.params.end());
			new_act->m_deleteEffects.push_back(new_eff);
		}

		norm_task.m_actions.push_back(new_act);
	}

	return std::move(norm_task);
}

