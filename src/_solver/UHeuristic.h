
#ifndef UltraPlanner_UHeuristic_h
#define UltraPlanner_UHeuristic_h

#include "config.h"
#include "USearchNode.h"

class ULTRA_SOLVER_API UHeuristic
{
public:
	virtual float estimate(const USearchNode * search_node) const
	{return 0.0f;}

	virtual float estimate(const USearchNode & search_node, const UPartialState & goal_description) const;
	virtual float estimate(const UState & state, const UPartialState & goal) const = 0;
};

class UBlindHeuristic : public UHeuristic
{
public:
	virtual float estimate(const USearchNode & search_node, const UPartialState & goal_description) const;
};


#endif
