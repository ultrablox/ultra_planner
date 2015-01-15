

#ifndef UltraPlanner_UPlanningGraphHeuristic_h
#define UltraPlanner_UPlanningGraphHeuristic_h

#include <solver/UHeuristic.h>
#include <memory>

class UPlanningGraph;
class USearchNode;
class UTransitionSystem;
class UState;
class UPartialState;

class UPlanningGraphHeuristic : public UHeuristic
{
public:
	UPlanningGraphHeuristic(const UTransitionSystem & transition_system);
	//virtual float estimate(const USearchNode * search_node) const;
	virtual float estimate(const UState & state, const UPartialState & goal) const override;
	static UPlanningGraph buildPlanningGraph(const UTransitionSystem & transition_system, const UState & state, const UPartialState & goal);
	//const UPlanningGraph * planningGraph() const;
private:
	//std::unique_ptr<UPlanningGraph> m_pPlanningGraph;
	const UTransitionSystem m_relaxedTransitionSystem;
};

#endif
