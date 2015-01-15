
#include "UGoalCountHeuristic.h"

UGoalCountHeuristic::UGoalCountHeuristic()
{
}

float UGoalCountHeuristic::estimate(const UState & state, const UPartialState & goal) const
{
	return goal.flags.mask.trueCount() - goal.flags.equalCount(state.flags);
}
