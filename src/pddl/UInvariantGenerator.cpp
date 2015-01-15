
#include "UInvariantGenerator.h"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace BoolTask;

std::ostream & operator<<(std::ostream & os, const InvariantCandidate & ic)
{
	os << "<{";
	for(int i = 0; i < ic.paramCount; ++i)
	{
		if(i > 0)
			os << ",";

		os << "p" << i;
	}
	os << "},{";

	for(int i = 0; i < ic.atoms.size(); ++i)
	{
		if(i > 0)
			os << ",";
		auto & atom = ic.atoms[i];
		os << atom.predicate->name << "(";

		for(int p = 0; p < atom.params.size(); ++p)
		{
			if(p > 0)
				os << ",";

			if(atom.params[p] == -1)
				os << "?";
			else
				os << "p" << atom.params[p];
		}

		os << ")";
	}

	os << "}>";
	return os;
}

void UInvariantGenerator::generate(const BoolTask::UNormalizedBoolTask & norm_task)
{
	//================Find variable predicates================
	vector<UntypedPredicate*> var_preds;

	auto effects_have_pred = [](const vector<Formula> & effects, const UntypedPredicate * pred){
		return std::find_if(effects.begin(), effects.end(), [=](const Formula & fm){
				return fm.predicate == pred;
			}) != effects.end();
	};

	for(auto pred : norm_task.m_predicates)
	{
		auto act_it = std::find_if(norm_task.m_actions.begin(), norm_task.m_actions.end(), [=](const UntypedAction * act){
			return effects_have_pred(act->m_deleteEffects, pred) || effects_have_pred(act->m_addEffects, pred);
		});
		if (act_it != norm_task.m_actions.end())
			var_preds.push_back(pred);
	}

	cout << "Found " << var_preds.size() << " variable predicates. Generating invariant candidates...\n";

	//==================Generate candidates==========================
	vector<InvariantCandidate> inv_candidates;

	for(auto pred : var_preds)
	{
		for(auto p = 0; p < pred->paramCount; ++p)
		{
			InvariantCandidate new_candidate;
			new_candidate.paramCount = 1;

			Formula fl(pred, pred->paramCount);
			fl.params[p] = 0;

			new_candidate.atoms.push_back(fl);

			inv_candidates.push_back(new_candidate);
		}
	}

	cout << "Created " << inv_candidates.size() << " initial candidates:\n";
	for(auto & cand : inv_candidates)
	{
		prooveInvariant(cand, norm_task.m_actions);
	}
}

bool UInvariantGenerator::prooveInvariant(const InvariantCandidate & ic, const std::vector<UntypedAction*> & operators)
{
	cout << "Prooving candidate " << ic << "...\n";

	for(auto op : operators)
	{
		if(operatorTooHeavy(ic, op))
			return false;
	}

	return true;
}

bool UInvariantGenerator::operatorTooHeavy(const InvariantCandidate & ic, const UntypedAction* op)
{

	return true;
}
