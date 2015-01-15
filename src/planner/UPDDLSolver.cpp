
#include "UPDDLSolver.h"
#include <core/utils/helpers.h>
#include <core/UError.h>
#include <pddl/UPDDLAction.h>
#include <solver/UMetric.h>
#include <solver/UHeuristic.h>
#include <solver/heuristics/UPlanningGraph.h>
#include <solver/heuristics/UPlanningGraphHeuristic.h>
#include <chrono>

using namespace std::chrono;

//=====================UPDDLSolverBase======================

UPDDLSolverBase::UPDDLSolverBase(UPDDLDomain * domain, UPDDLProblem * problem)
	:m_pDomain(domain), m_pProblem(problem)
{

}

void UPDDLSolverBase::interpretState(const UTransitionSystem & tr_sys, const UState & state) const
{
	/*for(int fi_index = 0; fi_index < mFunctions.variableInstances.size(); ++fi_index)
	{
		std::cout << "(";
		mFunctions.variableInstances[fi_index].print(std::cout, false);
		cout << ") = " << state.functionValues[fi_index];
	}
	cout <<"\n";

	cout << "Predicates:\n";
	auto pis = objects_from_indices(state.flags.toIndices(), mPredicates.variableInstances);
	for(auto pi : pis)
		pi.print(cout);*/

	auto state_descr = tr_sys.interpretState(state);
	for(auto & sd : state_descr)
		cout << sd << "\n";
}

void UPDDLSolverBase::interpretSearchNode(const USearchNode & node) const
{
	cout << "=============Node description================\n";
	cout << "\tPath:\n";
	/*for(auto transition : node.transitions)
	{
		auto action_instance = mActionInstances[transition];
		action_instance.print(cout);
	}*/

	cout << "\tCurrent state:\n";
//	interpretState(node.state);
}

void UPDDLSolverBase::interpretTransition(const UTransition & transition) const
{
	auto pred_indices = transition.effect.flags.mask.toIndices();
	for(auto pred_index : pred_indices)
	{
		if(!transition.effect.flags.state[pred_index])
		{
			cout << "not ";
		}
		
		mPredicates.variableInstances[pred_index].print();
	}
}

void UPDDLSolverBase::interpretTransitionPath(const UTransitionSystem & tr_sys, const TransitionPath & path) const
{
	cout << "Plan (" << path.size() << " actions):\n";

	int index = 0;
	for(auto tr_index : path)
	{
		cout << "\t" << ++index << " ";
		cout << tr_sys.transitionDescription(tr_index) << "\n";
	}

	ofstream fout("best_plan.pddl", ios::out);
	for(auto tr_index : path)
	{
		//auto act_inst = mActionInstances[tr_index];
//		fout << "(" << to_string(act_inst) << ")\n";
	}

	fout.close();
}

//==================UPDDLSolver================================

size_t object_index(const std::vector<UPDDLPredicateInstance> & object_container, UPDDLPredicate * object, const vector<UPDDLObject*> & params)
{
	auto obj_it = std::find_if(object_container.begin(), object_container.end(), [=](const UPDDLPredicateInstance & pred_instance){
		return (pred_instance.object == object) && (pred_instance.params == params);
	});

	/*UPDDLPredicateInstance inst;
	inst.object = object;
	inst.params = params;

	auto obj_it = std::find(object_container.begin(), object_container.end(), inst);*/

	if(obj_it == object_container.end())
		throw core_exception("Predicate instance not found");
	
	return std::distance(object_container.begin(), obj_it);
}

size_t object_index(const std::set<UPDDLPredicateInstance> & object_container, UPDDLPredicate * object, const vector<UPDDLObject*> & params)
{
	auto obj_it = object_container.find(UPDDLPredicateInstance(object, params));

	if(obj_it == object_container.end())
		throw core_exception("Predicate instance not found");
	
	return std::distance(object_container.begin(), obj_it);
}

vector<UPDDLObject*> params_for_predicate(UPDDLAction * action, const vector<UPDDLParameter*> & params, const vector<UPDDLObject*> & objects)
{
	vector<UPDDLObject*> result;

	for(auto gd_param : params)
	{
		if(gd_param->isConstant())
		{
			auto const_param = dynamic_cast<UPDDLConstantParameter*>(gd_param);
			result.push_back(const_param->constantObject);
		}
		else
		{
			auto act_param_it = std::find(action->parameters.begin(), action->parameters.end(), gd_param);
			if(act_param_it == action->parameters.end())
				throw core_exception("Unable to locate action parameter");

			auto obj_index = std::distance(action->parameters.begin(), act_param_it);
			result.push_back(objects[obj_index]);
		}
	}

	return result;
}

UTransition UPDDLSolverBase::createTransition(const UPDDLActionInstance & action_instance) const
{
	UTransition new_transition(mPredicates.variableInstances.size(), mFunctions.variableInstances.size());

	for(auto gd : action_instance.object->conditions)
	{
		//Build predicate parameters list from action parameters
		vector<UPDDLObject*> pred_params = params_for_predicate(action_instance.object, gd->params, action_instance.params);

		//If condition is referring to variable predicate
		if(std::find(mPredicates.variable.begin(), mPredicates.variable.end(), gd->predicate) != mPredicates.variable.end())
		{
			//Get predicate instance
			auto pred_instance_index = object_index(mPredicates.variableInstances, gd->predicate, pred_params);

			//Set status
			new_transition.setPredicateCondition(pred_instance_index, true);
		}
	}

	for(auto eff : action_instance.object->effects)
	{
		switch (eff->type())
		{
		case UPDDLEffect::Type::Atomic:
			{
				auto atomic_effect = static_cast<UPDDLAtomicEffect*>(eff);
				vector<UPDDLObject*> pred_params = params_for_predicate(action_instance.object, atomic_effect->params, action_instance.params);
				auto pred_instance_index = object_index(mPredicates.variableInstances, atomic_effect->predicate, pred_params);
				new_transition.effect.flags.set(pred_instance_index, !atomic_effect->negative);
				break;
			}
		case UPDDLEffect::Type::Numeric:
			{
				auto numeric_effect = static_cast<UPDDLNumericEffect*>(eff);

				//Get target function instance
				//std::find(mFunctions.variableInstances.begin(), mFunctions.variableInstances.end(), 
				vector<UPDDLObject*> target_func_params = params_for_predicate(action_instance.object, numeric_effect->params, action_instance.params);
				auto var_func_it = std::find(mFunctions.variableInstances.begin(), mFunctions.variableInstances.end(), UPDDLFunctionInstance(numeric_effect->function, target_func_params));
				if(var_func_it == mFunctions.variableInstances.end())
					throw core_exception("Unable to locate variable function instance.");
				int func_id = std::distance(mFunctions.variableInstances.begin(), var_func_it);
				
				//Get expression value
				if(numeric_effect->expression.func)
				{
					vector<UPDDLObject*> expr_func_params = params_for_predicate(action_instance.object, numeric_effect->expression.params, action_instance.params);
					auto expr_func_it = std::find(mFunctions.constantInstances.begin(), mFunctions.constantInstances.end(), UPDDLFunctionInstance(numeric_effect->expression.func, expr_func_params));
					if(expr_func_it == mFunctions.constantInstances.end())
						throw core_exception("Unable to locate constant function instance.");
					new_transition.effect.numeric.set(func_id, numeric_effect->numericType, expr_func_it->value);
				}
				else
				{
					new_transition.effect.numeric.set(func_id, numeric_effect->numericType, numeric_effect->expression.value);
				}

				break;
			}
		default:
			break;
		}
	}

//	new_transition.setDescription(to_string(action_instance));
	return new_transition;
}

std::vector<UPDDLActionInstance> UPDDLSolverBase::getActionInstances(const std::vector<size_t> & indices) const
{
	return objects_from_indices(indices, mActionInstances);
}

enum class UsageType {Unused, Constant, Variable};

void UPDDLSolverBase::instantiatePredicates(const TypesIndex & types_index)
{
	vector<UPDDLPredicate*> predicates[3];

	cout << "Instantiating predicates... ";

	//Sort out by principle groups
	for(auto pr : m_pDomain->predicates)
	{
		UsageType ut = UsageType::Unused;
		for(auto act : m_pDomain->actions)
		{
			UsageType cur_ut = UsageType::Unused;

			if(act->usesInEffect(pr))
				cur_ut = UsageType::Variable;
			else if(act->usesInCondition(pr))
				cur_ut = UsageType::Constant;

			ut = max(ut, cur_ut);
		}

		predicates[static_cast<int>(ut)].push_back(pr);
	}

	cout << predicates[static_cast<int>(UsageType::Variable)].size() << " variable, " << predicates[static_cast<int>(UsageType::Constant)].size() << " constant, " << predicates[static_cast<int>(UsageType::Unused)].size() << " unused.\n";

	mPredicates.variable = predicates[static_cast<int>(UsageType::Variable)];
	mPredicates.constant = predicates[static_cast<int>(UsageType::Constant)];

	cout << "Instancing variable predicates... ";


	//Create instances for variable predicates
	instantiateTypedObjects<UPDDLPredicate, UPDDLPredicateInstance>(types_index, predicates[static_cast<int>(UsageType::Variable)], mPredicates.variableInstances);

	cout << "created " << mPredicates.variableInstances.size() << " instances\n";
}

bool UPDDLSolverBase::isReachable(const UPDDLActionInstance & inst) const
{
	for(auto gd : inst.object->conditions)
	{
		//Build predicate parameters list from action parameters
		vector<UPDDLObject*> pred_params = params_for_predicate(inst.object, gd->params, inst.params);

		//Check that predicate is varibale
		auto var_pred_it = std::find(mPredicates.variable.begin(), mPredicates.variable.end(), gd->predicate);
		if(var_pred_it != mPredicates.variable.end())
			continue;

		//Maybe it is in constant and in initial state
		auto const_pred_it = std::find(mPredicates.constantInstances.begin(), mPredicates.constantInstances.end(), UPDDLPredicateInstance(gd->predicate, pred_params));
		if(const_pred_it != mPredicates.constantInstances.end())
			continue;

		//So it is constant and not in initial state - so action is unreachable
		return false;
	}


	//Check that all functions are described
	for(auto eff : inst.object->effects)
	{
		if(eff->type() == UPDDLEffect::Type::Numeric)
		{
			auto numeric_effect = static_cast<UPDDLNumericEffect*>(eff);
			if(numeric_effect->expression.func)
			{
				vector<UPDDLObject*> expr_func_params = params_for_predicate(inst.object, numeric_effect->expression.params, inst.params);
				auto expr_func_it = std::find(mFunctions.constantInstances.begin(), mFunctions.constantInstances.end(), UPDDLFunctionInstance(numeric_effect->expression.func, expr_func_params));
				if(expr_func_it == mFunctions.constantInstances.end())
				{
					return false;
				}
			}
		}
	}

	return true;
}

void UPDDLSolverBase::bruteforceActionInstances(const UPDDLSolverBase::TypesIndex & type_index, std::vector<UPDDLActionInstance> & target_array, UPDDLActionInstance new_instance, const int param_index) const
{
	if(param_index <  new_instance.object->parameters.size())
	{
		auto current_param_type = new_instance.object->parameter(param_index)->type;

		auto type_group = type_index.find(current_param_type);
		for(auto obj : type_group->second)
		{
			new_instance.params[param_index] = obj;
			bruteforceActionInstances(type_index, target_array, new_instance, param_index + 1);
		}
	}
	else
	{
		//target_array.insert(target_array.end(), new_instance);
		if(isReachable(new_instance))
			target_array.push_back(new_instance);
	}
}

void UPDDLSolverBase::instantiateActions(const TypesIndex & types_index)
{
	cout << "Instantiating actions...\n";
	//instantiateTypedObjects<UPDDLAction, UPDDLActionInstance>(types_index, m_pDomain->actions, mActionInstances);
	//_instantiateActions(types_index, m_pDomain->actions, mActionInstances);
	int i = 0;
	for(auto act : m_pDomain->actions)
	{
		cout << "\taction " << i++ << "/" << m_pDomain->actions.size() << "... ";

		UPDDLActionInstance instance;
		instance.object = act;
		instance.params.resize(act->parameters.size());
		bruteforceActionInstances(types_index, mActionInstances, instance, 0);

		cout << "total " << mActionInstances.size() << " instances\n";
	}

	//cout << "Totaly created " << mActionInstances.size() << " instances.\n";

	/*auto act_it = mActionInstances.begin();

	while(act_it != mActionInstances.end())
	{
		if(!isReachable(*act_it))
			act_it = mActionInstances.erase(act_it);
		else
			++act_it;
	}

	cout << "Left " << mActionInstances.size() << " reachable actions.\n";*/
}

void UPDDLSolverBase::instantiateFunctions(const TypesIndex & types_index)
{
	vector<UPDDLFunction*> functions[3];

	cout << "Instantiating functions... ";

	//Sort out by principle groups
	for(auto fun : m_pDomain->functions)
	{
		UsageType ut = UsageType::Unused;
		for(auto act : m_pDomain->actions)
		{
			UsageType cur_ut = UsageType::Unused;

			if(act->usesInEffect(fun))
				cur_ut = UsageType::Variable;

			ut = max(ut, cur_ut);
		}

		functions[static_cast<int>(ut)].push_back(fun);
	}

	cout << functions[static_cast<int>(UsageType::Variable)].size() << " variable, " << functions[static_cast<int>(UsageType::Constant)].size() << " constant, " << functions[static_cast<int>(UsageType::Unused)].size() << " unused.\n";

	mFunctions.variable = functions[static_cast<int>(UsageType::Variable)];
	mFunctions.constant = functions[static_cast<int>(UsageType::Constant)];

	cout << "Instancing variable functions... ";


	//Create instances for variable predicates
	instantiateTypedObjects<UPDDLFunction, UPDDLFunctionInstance>(types_index, functions[static_cast<int>(UsageType::Variable)], mFunctions.variableInstances);

	cout << "created " << mFunctions.variableInstances.size() << " instances\n";

	//instantiateTypedObjects<UPDDLFunction, UPDDLFunctionInstance>(types_index, m_pDomain->functions, mFunctionInstances);
}
