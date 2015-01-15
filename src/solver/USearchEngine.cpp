
#include "USearchEngine.h"
#include "USolver.h"
#include "UHeuristic.h"
#include "UMetric.h"

#include <thread>


bool QueueGreater::operator()(const UNodeEstimatedRef & a, const UNodeEstimatedRef & b) const
{
	return (a.estimation > b.estimation);
}

UNodeEstimatedRef QueueGreater::min_value() const
{
	return UNodeEstimatedRef(0, NodeAddressT::NodeGroupIndexT::invalidIndex());
}

USearchEngine::USearchEngine(USolver * p_solver, const UTransitionSystem & transition_system)
	:m_pSolver(p_solver), mTransitionSystem(transition_system)
{

}

void USearchEngine::expandNodes(std::list<size_t> & nodes)
{
//	m_pSolver->expandNodes(nodes);
}

void USearchEngine::printMemoryUsage() const
{
	cout << "Current memory usage is " << get_internal_memory_usage() << " MB.\n";
}

void USearchEngine::expandNodes(std::vector<UStateReference> & current_nodes)
{
	cout << "Total " << current_nodes.size() << " nodes\n";

	std::swap(m_pSolver->bufExpandInput(), current_nodes);
	m_pSolver->prExpandInBuffer();
	m_pSolver->prEstimateOutputBuffer();
	m_pSolver->mergeEstimatedOutput();
}

void USearchEngine::dump()
{

}

/*
std::vector<const USearchNode*> USearchEngine::openList()
{
	return m_pSolver->getOpenListAndClear();
}
*/
//=========================AStarSearchEngine========================
AStarSearchEngine::AStarSearchEngine(USolver * p_solver, const UTransitionSystem & transition_system, const UHeuristic * heuristic, const UMetric * metric)
	:USearchEngine(p_solver, transition_system), m_pHeuristic(heuristic), m_pMetric(metric), pool((mem_for_pools / 2) / block_type::raw_size, (mem_for_pools / 2) / block_type::raw_size), m_searchQueue(pool), m_state(state_t::Preparing)
{
}

void AStarSearchEngine::execute(const UStateReference & initial_state_ref, bool search_optimal)
{
	m_pSolver->setMetric(m_pMetric);
	//m_pSolver->bufExpandNew()[NodeEstimationT()].push_back(initial_node_ref);
	m_pSolver->nodeFactory().createNode(initial_state_ref);

	auto init_node_ref = m_pSolver->nodeFactory().estimatedNodeRef(initial_state_ref);
	m_searchQueue.push(init_node_ref);
	//NodeRefIntervalType cur_nodes(initial_node_ref, initial_node_ref);
	int step_num = 0;

	auto start_tp = high_resolution_clock::now();

	m_state = state_t::SearchingFirst;
		
	while(m_state != state_t::Finished)
	{

		cout <<		"=============================\n"
					"::Step #" << ++step_num << "\n"
					"=============================\n";
		auto step_start_tp = high_resolution_clock::now();
		
		auto best_nodes = pickBestNodes(5000, m_pSolver->approximateInputMaxSize());
		expandNodes(best_nodes);

		cout << "Adding new nodes to queue... ";
		
		for(auto & nr : m_pSolver->bufExpandNew())
		{
			if(m_state == state_t::SearchingBest)
			{
				if((*m_pMetric)(nr) < m_pSolver->cullingMetric())
					m_searchQueue.push(m_pSolver->nodeFactory().estimatedNodeRef(nr));
			}
			else
				m_searchQueue.push(m_pSolver->nodeFactory().estimatedNodeRef(nr));
		}
		
		m_pSolver->bufExpandNew().clear();

		cout << "done.\n";

		auto step_end_tp = high_resolution_clock::now();
		cout << "Step finished in " << setprecision(2) << duration_cast<microseconds>(step_end_tp - step_start_tp).count() * 1e-6 << " secs, total search time is " << duration_cast<microseconds>(step_end_tp - start_tp).count() * 1e-6<< " sec\n";
		cout << "\n";

		cout << "Queue size is " << m_searchQueue.size() << "\n";
		//break;
		//printMemoryUsage();

		if(m_searchQueue.empty())
			m_state = state_t::Finished;
		else
		{
			if(m_pSolver->goalFound())
			{
				switch (m_state)
				{
				case AStarSearchEngine::state_t::SearchingFirst:
					{
						if(search_optimal)
						{
							cout << "Found first plan with metric.";
							m_pSolver->setCullMetric(true, m_pSolver->bestGoalMetric(m_pMetric), m_pMetric);
							m_state = state_t::SearchingBest;
						}
						else
						{
							m_state = state_t::Finished;
						}
						break;
					}
				case AStarSearchEngine::state_t::SearchingBest:
					{
						break;
					}
				default:
					throw runtime_error("Invalid state");
					break;
				}
			}
		}




		/*if(get_internal_memory_usage() > 5000.0)
		{
			cout << "Memory overflow, clearing buffers...";
			m_pSolver->clearBuffers();
			cout << " done.\n";
		}*/
	}
}

bool AStarSearchEngine::bestGoalNode(const USearchNode * n1, const USearchNode * n2) const
{
	if(m_pMetric)
	{
		if(m_pMetric->compare(n1, n2))
			return true;
		else
			return false;
	}
	else
		return true;
}

std::vector<UStateReference> AStarSearchEngine::pickBestNodes(size_t min_count, size_t max_count)
{
	//Take at least 2 best groups
	int num_of_processed_groups(0);
	NodeEstimationT last_group_id = m_searchQueue.top().estimation;

	vector<UStateReference> res;
	for(; (max_count > 0) && (!m_searchQueue.empty()) && (num_of_processed_groups < 1); --max_count)
	{
		auto node = m_searchQueue.top();

		if(last_group_id != node.estimation)
			++num_of_processed_groups;
		last_group_id = node.estimation;

		bool take_state = true;

		auto state_ref = UStateReference(&m_pSolver->stateFactory(), node.stateAddress);
		if(m_state == state_t::SearchingBest)
			take_state = ((*m_pMetric)(state_ref) < m_pSolver->cullingMetric());

		if(take_state)
			res.push_back(state_ref);

		m_searchQueue.pop();
	}

	return res;
}
