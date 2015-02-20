
#include "UHeuristic.h"


float UHeuristic::estimate(const USearchNode & search_node, const UPartialState & goal_description) const
{
	//Calculate accomplished goal count

	return goal_description.flags.equalCount(search_node.state.flags);
}

float UBlindHeuristic::estimate(const USearchNode & search_node, const UPartialState & goal_description) const
{
	if(goal_description.matches(search_node.state))
		return 1.0f;
	else
		return 0.0f;
}
