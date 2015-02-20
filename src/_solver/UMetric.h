
#ifndef UltraPlanner_UMetric_h
#define UltraPlanner_UMetric_h

#include "USearchNode.h"

class UMetric
{
public:
	/*
	Returns true, if first state is better than second.
	*/
	bool compare(const USearchNode * n1, const USearchNode * n2) const;
	float operator()(const UState & state) const;
	bool operator()(const UState & s1, const UState & s2) const;
};

#endif
