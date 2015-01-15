
#include "UMetric.h"

bool UMetric::compare(const USearchNode * n1, const USearchNode * n2) const
{
	return n1->state.functionValues[0] < n2->state.functionValues[0];
}

float UMetric::operator()(const UState & state) const
{
	return state.functionValues[0];
}

bool UMetric::operator()(const UState & s1, const UState & s2) const
{
	return (*this)(s1) < (*this)(s2);
}
