#include "USolver.h"
#include "../pddl/UPDDLDomain.h"
#include "UInstantiator.h"
#include "UHeuristic.h"
#include "USearchEngine.h"
#include "UMetric.h"
#include <core/transition_system/UTransitionSystem.h>
#include <core/UError.h>
#include <core/utils/helpers.h>
#include <map>
#include <chrono>
#include <exception>

using namespace std;
using namespace std::chrono;

//==================================USolver=============================
USolver::USolver(size_t solver_output_buffer_size)
	:USolverBase(solver_output_buffer_size), m_merger(UOutputMerger(*this))
{}

USolver::~USolver()
{
}

std::vector<SolverResult> USolver::solve(const UTransitionSystem & transition_system, const UStateReference & initial_state_ref, const UPartialState & goal_states, const UMetric * metric, const UHeuristic * heuristic, bool search_optimal)
{
	mTransitionSystem = transition_system;
	m_database.setTransitionSystem(transition_system);

	auto start_tp = high_resolution_clock::now();

	setGoal(goal_states);

	AStarSearchEngine search_engine(this, mTransitionSystem, heuristic, metric);

	//auto initial_node = new USearchNode(initial_state);
	

	/*while(true)
	{
		createNode(m_estimator(initial_state_ref), initial_state_ref);
	}*/
	//size_t initial_node_index = 0;//createNode(initial_state);

	search_engine.execute(initial_state_ref, search_optimal);

//	cout << "\nGenerated " << m_pOutputMerger->exploredNodesCount() << " explored and " << m_pOutputMerger->unexploredNodesCount() << " unexplored states.\n";
	std::vector<SolverResult> paths;

	auto end_tp = high_resolution_clock::now();
	double search_timecost = duration_cast<microseconds>(end_tp - start_tp).count() * 1e-6;
	cout << "\nProblem solving finished, " << search_timecost << " seconds spent.\n";// average " << float(duration_cast<microseconds>(end_tp - start_tp).count()) / m_pOutputMerger->exploredNodesCount()  << " usecs/state.\n";

	if(goalFound())
	{
		cout << "Found " << m_goalNodes.size() << " goal nodes, finding best...\n";

		nth_element(m_goalNodes.begin(), m_goalNodes.begin(), m_goalNodes.end(), [&](const UStateReference & n1, const UStateReference & n2){
			return (*metric)(state(n1.address()), state(n2.address()));
		});//, *metric);

		cout << "Node with best metric " << (*metric)(state(m_goalNodes[0].address())) << ", building transition path...";
		SolverResult sr(state(m_goalNodes[0].address()));
		sr.path = m_database.buildTransitionPath(m_goalNodes[0]);
		paths.push_back(sr);

		//Ouput statistics
		ofstream os("stats.txt");
		os << "search_time:" << search_timecost << std::endl;
		os << "batch_factor:" << USolverBase::m_maxOutputBufferSize << std::endl;
	}
	else
	{
		cout << "Unable to find a solution\n";
	}

	return paths;
}

const UTransitionSystem & USolver::transitionSystem() const
{
	return mTransitionSystem;
}

void USolver::dump() const
{
	m_database.dump("search_database.usd");
}

std::vector<USearchNode> USolver::expandNode(const NodeElementType & node)
{
	auto state_ref = state(node.address());
	UState base_state = state_ref;

	auto actions_indices(mTransitionSystem.getAvalibleTransitions(base_state));

	std::vector<USearchNode> res;
	res.reserve(actions_indices.size());

	for(auto ai : actions_indices)
		res.push_back(USearchNode(mTransitionSystem.applyTransition(base_state, ai), state_ref.address()));

	return res;
}

void USolver::clearBuffers()
{
	USolverBase::clearBuffers();
	m_merger.clearCache();
}

void USolver::clearDatabase()
{
	m_database.clear();
}

UStateFactory & USolver::stateFactory()
{
	return m_database;
}

const UStateFactory & USolver::stateFactory() const
{
	return m_database;
}

USearchNodeFactory & USolver::nodeFactory()
{
	return m_database;
}

const USearchNodeFactory & USolver::nodeFactory() const
{
	return m_database;
}

//===================================Simple Solver===================================
USimpleSolver::USimpleSolver(size_t solver_output_buffer_size)
	:USolver(solver_output_buffer_size)
{
}


void USimpleSolver::expandInBuffer()
{
	cout << "(" << m_expandInputBuffer.size() << " nodes)... ";
		
	size_t expanded_node_counter(0);
	const size_t start_input_size(m_expandInputBuffer.size());

	for(auto & state_ref : m_expandInputBuffer)
	{
		//Expand node
		auto expanded_nodes(expandNode(state_ref));

		m_expandOutputBuffer.insert(m_expandOutputBuffer.end(), expanded_nodes.begin(), expanded_nodes.end());
			
		//Mark state as expanded
		state_ref.setExpanded(true);
			
		++expanded_node_counter;
	}

	m_expandInputBuffer.clear();

	//Update ratio
	m_expansionRatio.add(m_expandOutputBuffer.size(), expanded_node_counter);
}

void USimpleSolver::estimateOutputBuffer()
{
	cout << "(" << m_expandOutputBuffer.size() << " nodes)... ";
		
	for(auto node : m_expandOutputBuffer)
		m_estimatedOutput[m_estimator(node.state, m_goal)].push_back(std::move(node));
		
	m_expandOutputBuffer.clear();
}

void USimpleSolver::mergeEstimatedOutput()
{
	//Iterate each estimation group

	m_merger.merge();
}
