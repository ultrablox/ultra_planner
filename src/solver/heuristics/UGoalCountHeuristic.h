
#ifndef UltraPlanner_UGoalCountHeuristic_h
#define UltraPlanner_UGoalCountHeuristic_h

#include <solver/UHeuristic.h>

class UGoalCountHeuristic : public UHeuristic
{
public:
	UGoalCountHeuristic();
	virtual float estimate(const UState & state, const UPartialState & goal) const override;
};

#endif
