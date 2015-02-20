#include "UNodeEstimator.h"
#include "UHeuristic.h"

NodeEstimationT NodeEstimationT::invalidIndex()
{
	NodeEstimationT res;
	for(auto & i : res)
		i = numeric_limits<float>::max();

	return res;
}


std::ostream & operator<<(std::ostream & os, const NodeEstimationT & est)
{
	os << est[0];

	for(int i = 1; i < heuristic_count; ++i)
		os << "," << est[i];

	return os;
}

std::string to_string(const NodeEstimationT & el)
{
	std::stringstream ss;
	ss << el;
	
	std::string res;
	ss >> res;
	return res;
}

NodeEstimationT UNodeEstimator::operator()(const UState & state, const UPartialState & goal) const
{
	NodeEstimationT result;

	for(int i = 0; i < m_heuristics.size(); ++i)
		result[i] = m_heuristics[i]->estimate(state, goal);

	return result;
}

void UNodeEstimator::setHeuristic(int index, UHeuristic * p_heuristic)
{
	if(index >= heuristic_count)
		return;

	m_heuristics[index] = p_heuristic;
}
