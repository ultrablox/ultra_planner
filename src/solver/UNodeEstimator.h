
#ifndef UltraPlanner_UNodeEstimator_h
#define UltraPlanner_UNodeEstimator_h

#include <core/transition_system/UState.h>
#include <array>

const int heuristic_count = 1;

struct NodeEstimationT : public std::array<float, heuristic_count>
{
public:
	typedef std::less<NodeEstimationT> BestSortOperator;
	static NodeEstimationT invalidIndex();
};

std::ostream & operator<<(std::ostream & os, const NodeEstimationT & est);
std::string to_string(const NodeEstimationT & el);

class UHeuristic;
class UPartialState;

class UNodeEstimator
{
public:
	NodeEstimationT operator()(const UState & state, const UPartialState & goal) const;
	void setHeuristic(int index, UHeuristic* heuristic);
private:
	std::array<UHeuristic*, heuristic_count> m_heuristics;
};


#endif
