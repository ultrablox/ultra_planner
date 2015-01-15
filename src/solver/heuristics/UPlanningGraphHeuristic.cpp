
#include "UPlanningGraphHeuristic.h"
#include "UPlanningGraph.h"

UPlanningGraphHeuristic::UPlanningGraphHeuristic(const UTransitionSystem & transition_system)
	:m_relaxedTransitionSystem(transition_system.toRelaxed())
{
	/*	cout << "Building planning graph...\n";
	
	//pg_heuristic = new UPlanningGraphHeuristic(pg);

	cout << "Planning graph built, " << m_pPlanningGraph->layerCount() << " layers.\n";*/
}
/*
const UPlanningGraph * UPlanningGraphHeuristic::planningGraph() const
{
	return m_pPlanningGraph.get();
}

float UPlanningGraphHeuristic::estimate(const USearchNode * search_node) const
{
	return estimate(search_node->state);
}*/

float UPlanningGraphHeuristic::estimate(const UState & state, const UPartialState & goal) const
{
	/*auto graph = buildPlanningGraph(state, goal);
	return static_cast<float>(graph.layerCount());*/

	UState current_state(state);
	int level_index = 0;

	std::vector<size_t> last_transitions;
	for(;!goal.matches(current_state); ++level_index)
	{
		auto _transitions = m_relaxedTransitionSystem.getAvalibleTransitions(current_state);
		std::vector<size_t> transitions(_transitions.begin(), _transitions.end());

		if(last_transitions == transitions)
			break;

		for(auto ti : transitions)
			current_state.apply(m_relaxedTransitionSystem.transition(ti));

		swap(last_transitions, transitions);
	}

	/*UBitset last_transitions;
	for(;!goal.matches(current_state); ++level_index)
	{
		auto transitions = m_relaxedTransitionSystem.getAvalibleTransitionsMask(current_state);

		if(last_transitions == transitions)
			break;

		//for(auto ti : transitions)
		for(auto bit_it = transitions.pbegin(); bit_it != transitions.pend(); ++bit_it)
			current_state.apply(m_relaxedTransitionSystem.transition(bit_it.index()));

		swap(last_transitions, transitions);
	}*/

	return level_index;
}

UPlanningGraph UPlanningGraphHeuristic::buildPlanningGraph(const UTransitionSystem & transition_system, const UState & state, const UPartialState & goal)
{
	//Create relaxed transition system

	UPlanningGraph graph;

	UState current_state(state);
	
	//UPlanningGraph::TransitionLayer * last_transition_layer = nullptr;

	std::set<size_t> last_transitions;

	do
	{
		//Get avalible actions from current state
		auto transitions = transition_system.getAvalibleTransitions(current_state);
		
		std::set<size_t> transition_indices(transitions.begin(), transitions.end());

		if(last_transitions == transition_indices)
		{
			break;
		}

		//Create new state layer in planning graph
		graph.createLayer(current_state);

		//Create actions layer
		graph.createLayer(transition_indices);

		//Calculate new state
		for(auto ti : transition_indices)
			current_state.apply(transition_system.transition(ti));

		swap(last_transitions, transition_indices);

	} while(!goal.matches(current_state));//true)

	//Add goal layer
	graph.createLayer(current_state);

	return graph;
}
