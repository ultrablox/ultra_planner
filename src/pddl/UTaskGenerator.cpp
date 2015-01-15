
#include "UTaskGenerator.h"
#include "UBoolTask.h"
#include "UInvariantGenerator.h"
#include <core/utils/helpers.h>
#include <SimpleEval.h>
#include <instantiation.h>

using namespace VAL;
using namespace Inst;
using namespace BoolTask;

struct UParametrizedObject
{
	typedef vector<string> UParameterList;

	UParameterList params;
	string name;
};

std::ostream & operator<<(std::ostream & os, const UParametrizedObject & obj)
{
	os << "(" << obj.name;
	for(auto & par : obj.params)
		os << " " << par;
	os << ")";

	return os;
}

vector<string> to_string_list(const vector<UParametrizedObject> & param_object)
{
	vector<string> res;
	for(auto & po : param_object)
		res.push_back(to_string(po));
	return std::move(res);
}

int element_index(vector<UParametrizedObject> & data, const string & val)
{
	auto it = std::find_if(data.begin(), data.end(), [=](const UParametrizedObject & po){
		string tmp = to_string(po);
		return tmp == val;
	});

	return std::distance(data.begin(), it);
}

template<class T>
UParametrizedObject to_param_obj(const T & obj)
{
	UParametrizedObject res;
	auto p = obj->getProp();
	res.name = p->head->getName();

//	auto & env = obj->getEnv();
	for(auto par : *obj)
	{
		//auto x = par;
		res.params.push_back(par->getName());
	}


	//cout << res << "\n";

	return res;
}

UNumericEffect::Type to_numeric_type(assign_op op)
{
	switch(op)
	{
	case E_ASSIGN:
		return UNumericEffect::Type::Assign;
	case E_INCREASE:
		return UNumericEffect::Type::Increase;
	case E_DECREASE:
		return UNumericEffect::Type::Decrease;
	case E_SCALE_UP:
		return UNumericEffect::Type::ScaleUp;
	case E_SCALE_DOWN:
		return UNumericEffect::Type::ScaleDown;
	case E_ASSIGN_CTS:
		return UNumericEffect::Type::Assign;
	}
}

std::string params_to_string(VAL::FastEnvironment & env, const parameter_symbol_list * list)
{
	string res;

	for(auto i : *list)
	{
		auto par = static_cast<const VAL::IDsymbol<VAL::var_symbol>*>(i);
		
		if(!res.empty())
			res += " ";
		res += env[par]->getName();
	}

	return res;
}

template<typename PropT>
std::string to_string(VAL::FastEnvironment & env, PropT * prop)
{
	string res("");
	//auto p = eff->getProp();
	auto p = prop;

	return "(" + p->head->getName() + " " + params_to_string(env, p->args) + ")";

	return res;
}


std::string eff_to_string(const VAL::simple_effect* se)
{
	auto p = se->prop;

	string res = "(" + p->head->getName();

	for(auto & arg : *p->args)
		res += " " + arg->getName();

	return res + ")";
}

std::string ass_to_string(const VAL::assignment* ass)
{
	string res = "(" + ass->getFTerm()->getFunction()->getName();

	for(auto & arg : *ass->getFTerm()->getArgs())
		res += " " + arg->getName();

	return res + ")";
}

UNumericExpression * to_numeric_expression(vector<string> & float_vars, VAL::FastEnvironment & env, const VAL::expression * expr)
{
	if(auto ft = dynamic_cast<const func_term*>(expr))
	{
		string var_name = "(" + ft->getFunction()->getName() + " " + params_to_string(env, ft->getArgs()) + ")";
		int var_index = element_index(float_vars, var_name);

		return UNumericExpression::simpleVariable(var_index);
	}
	if(auto ie = dynamic_cast<const VAL::int_expression*>(expr))
	{
		return UNumericExpression::simpleValue(ie->double_value());
	}
	else
	{
		cout << "Unknown expression";
	}
}

UPlanningTask UTaskGenerator::generateTask(VAL::analysis * pAnalysis)
{
	theTC = new VAL::TypeChecker(current_analysis);
	auto td = theTC->typecheckDomain();
	/*auto tp = theTC->typecheckProblem();*/
	SimpleEvaluator::setInitialState();

	for(auto os : *(current_analysis->the_domain->ops))
    {
    	//cout << os->name->getName() << "\n";
    	instantiatedOp::instantiate(os, current_analysis->the_problem,*theTC);
    	//cout << instantiatedOp::howMany() << " so far\n";
    }

	//cout << instantiatedOp::howMany() << "\n";
	instantiatedOp::createAllLiterals(current_analysis->the_problem, theTC);
    instantiatedOp::filterOps(theTC);
    //cout << instantiatedOp::howMany() << "\n";
    //instantiatedOp::writeAll(cout);

	//cout << "\nList of all literals:\n";
    
	//instantiatedOp::writeAllLiterals(cout);

	//Predicates


	//Create string-cache of all literals
	vector<UParametrizedObject> bool_vars;
	for(auto lit = instantiatedOp::literalsBegin(); lit != instantiatedOp::literalsEnd(); ++lit)
	{

		//stringstream ss;
		//(*lit)->write(cout);
		auto bool_var = to_param_obj((*lit));
		bool_vars.push_back(bool_var);
	}

	//Create string-cache of all float-vars
	vector<string> float_vars;
	for(auto lit = instantiatedOp::pnesBegin(); lit != instantiatedOp::pnesEnd(); ++lit)
	{
		stringstream ss;
		(*lit)->write(ss);
		float_vars.push_back(ss.str());
	}

	//Create transition system
	auto transition_system = UTransitionSystem(bool_vars.size(), float_vars.size());
	transition_system.setFlagsDescription(to_string_list(bool_vars));
	transition_system.setFloatsDescription(float_vars);
	//=====================================================================
	//========================Create transitions===========================
	//=====================================================================
	for(auto op_inst = instantiatedOp::opsBegin(); op_inst != instantiatedOp::opsEnd(); ++op_inst)
	{

		auto & transition = transition_system.createTransition();

		const VAL::operator_ * op = (*op_inst)->forOp();
		auto & env = *(*op_inst)->getEnv();

		//===============Preconditions===================
		auto cond = op->precondition;
		auto cg = static_cast<conj_goal*>(cond);

		for(auto it : *cg->getGoals())
		{
			auto sg = static_cast<simple_goal*>(it);
			string cond_data = to_string(env, sg->getProp());
			int index = element_index(bool_vars, cond_data);
			transition.condition.flags.set(index, sg->getPolarity() == E_POS);
		}

		//===============Positive effects===================
		for(auto add_eff : op->effects->add_effects)
		{
			string eff_data = to_string(env, add_eff->prop);
			int index = element_index(bool_vars, eff_data);
			transition.effect.flags.set(index, true);

		}

		//===============Negative effects===================
		for(auto del_eff : op->effects->del_effects)
		{
			string eff_data = to_string(env, del_eff->prop);
			int index = element_index(bool_vars, eff_data);
			transition.effect.flags.set(index, false);

		}

		//===============Real vars effects===================
		for(auto ass_eff : op->effects->assign_effects)
		{
			//Target variable
			auto f_term = ass_eff->getFTerm();
			string var_name = "(" + f_term->getFunction()->getName() + ")";
			int var_index = element_index(float_vars, var_name);

			//Expression
			auto num_exp = to_numeric_expression(float_vars, *(*op_inst)->getEnv(), ass_eff->getExpr());
			transition.effect.numeric.set(var_index, to_numeric_type(ass_eff->getOp()), num_exp);
		}

		transition.setDescription(to_string(**op_inst));
	}

	//=====================================================================
	//======================Create initial state===========================
	//=====================================================================
	UState initial_state(transition_system.createState());
	auto val_is = pAnalysis->the_problem->initial_state;

	//Bool vals
	for(auto add_eff : val_is->add_effects)
	{
		string var_name = eff_to_string(add_eff);
		int var_index = element_index(bool_vars, var_name);
		initial_state.flags.set(var_index, true);
	}

	//Float vals
	for(auto ass_eff : val_is->assign_effects)
	{
		string var_name = ass_to_string(ass_eff);
		int var_index = element_index(float_vars, var_name);
		auto ie = dynamic_cast<const VAL::int_expression*>(ass_eff->getExpr());
		initial_state.functionValues[var_index] = ie->double_value();
	}


	//=====================================================================
	//==========================Create goal================================
	//=====================================================================
	UPartialState goal(transition_system.createPartialState());
	auto val_goal = pAnalysis->the_problem->the_goal;
	auto cg = static_cast<conj_goal*>(val_goal);

	for(auto g : *cg->getGoals())
	{
		auto sg = static_cast<simple_goal*>(g);
		auto p = sg->getProp();

		string var_name = "(" + p->head->getName();
		
		for(auto arg : *p->args)
			var_name += " " + arg->getName();
		var_name += ")";

		int var_index = element_index(bool_vars, var_name);
		goal.flags.set(var_index, sg->getPolarity() == VAL::polarity::E_POS);
	}
	
	return UPlanningTask(transition_system, initial_state, goal, false);
}

UBoolTask UTaskGenerator::generateBoolTask(VAL::analysis * pAnalysis)
{
	theTC = new VAL::TypeChecker(current_analysis);
	auto td = theTC->typecheckDomain();
	/*auto tp = theTC->typecheckProblem();*/
	SimpleEvaluator::setInitialState();

	for(auto os : *(current_analysis->the_domain->ops))
    {
    	//cout << os->name->getName() << "\n";
    	instantiatedOp::instantiate(os, current_analysis->the_problem,*theTC);
    	//cout << instantiatedOp::howMany() << " so far\n";
    }

	//cout << instantiatedOp::howMany() << "\n";
	instantiatedOp::createAllLiterals(current_analysis->the_problem, theTC);
    instantiatedOp::filterOps(theTC);
    //cout << instantiatedOp::howMany() << "\n";


	UBoolTask bv_task;
	
	//==================Types=================
	//Create default types
	bv_task.addType("object");
	bv_task.addType("number");

	for(auto t : *pAnalysis->the_domain->types)
	{
		auto parent_type = t->type;
		bv_task.addType(t->getName(), parent_type->getName());
	}

	//=================Constants and objects==================
	for(auto c : pAnalysis->const_tab)
		bv_task.addObject(c.first, c.second->type->getName());

	//=====================================================================
	//=====================Predicates=====================
	//=====================================================================
	for(auto p : *pAnalysis->the_domain->predicates)
	{
		auto args = p->getArgs();
		auto new_pred = bv_task.addPredicate(p->getPred()->getName(), args->size());
		for(auto arg : *args)
		{
			auto utype = bv_task.type(arg->type->getName());
			if(!utype)
				throw runtime_error("Unable to find type '" + arg->type->getName() + "' in task.");

			new_pred->params.push_back(TypedPredicate::ParamT(utype, arg->getName()));
		}
	}
	
	//=====================================================================
	//========================Actions======================================
	//=====================================================================
	for(auto op : *pAnalysis->the_domain->ops)
	{
		auto new_act = new Action(op->name->getName());
		bv_task.addAction(new_act);

		//===============Parameters======================
		for(auto par : *op->parameters)
		{
			auto utype = bv_task.type(par->type->getName());
			new_act->params.push_back(Action::ParamT(utype, par->getName()));
		}

		//===============Preconditions===================
		auto cg = static_cast<conj_goal*>(op->precondition);

		for(auto it : *cg->getGoals())
		{
			auto sg = static_cast<simple_goal*>(it);
			auto prop = sg->getProp();

			Formula cond(bv_task.predicate(prop->head->getName()));
			
			for(auto & arg : *prop->args)
			{
				auto _arg = static_cast<VAL::IDsymbol<VAL::var_symbol>*>(arg);
				cond.params.push_back(_arg->getId());
			}

			new_act->preconditions.push_back(cond);
		}

		//===============Positive effects===================
		for(auto add_eff : op->effects->add_effects)
		{
			auto prop = add_eff->prop;
			Formula eff(bv_task.predicate(prop->head->getName()));
			for(auto & arg : *prop->args)
			{
				auto _arg = static_cast<VAL::IDsymbol<VAL::var_symbol>*>(arg);
				eff.params.push_back(_arg->getId());
			}
			new_act->m_addEffects.push_back(eff);
		}

		//===============Negative effects===================
		for(auto del_eff : op->effects->del_effects)
		{
			auto prop = del_eff->prop;
			Formula eff(bv_task.predicate(prop->head->getName()));
			for(auto & arg : *prop->args)
			{
				auto _arg = static_cast<VAL::IDsymbol<VAL::var_symbol>*>(arg);
				eff.params.push_back(_arg->getId());
			}
			new_act->m_deleteEffects.push_back(eff);
		}
	}

	auto norm_task = bv_task.normalize();

	UInvariantGenerator inv_gen;
	inv_gen.generate(norm_task);
	
	return std::move(bv_task);
}
