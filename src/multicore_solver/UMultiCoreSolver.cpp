
#include "UMultiCoreSolver.h"
#include "UConcurrentQueue.h"
#include "UConcurrentVector.h"

#include <tbb/concurrent_vector.h>
#include <thread>
#include <atomic>
#include <future>

const int THREAD_COUNT = 6;

using namespace std;

//===================================UMultiCoreSolver==================================


UMultiCoreSolver::UMultiCoreSolver(size_t solver_output_buffer_size)
	:USimpleSolver(solver_output_buffer_size)
{
}

void UMultiCoreSolver::expandInBuffer()
{
	cout << "(" << m_expandInputBuffer.size() << " nodes)... ";

	vector<future<vector<USearchNode>>> node_groups;

	auto f_expander = [&](const UStateReference & state_ref){
		return expandNode(state_ref);
	};

	for(auto & el : m_expandInputBuffer)
		node_groups.push_back(async(f_expander, el));

	for(auto & ng : node_groups)
	{
		auto gr = ng.get();
		m_expandOutputBuffer.insert(m_expandOutputBuffer.end(), gr.begin(), gr.end());
	}

	//Mark nodes as expanded
	for(auto & n : m_expandInputBuffer)
		n.setExpanded(true);

	m_expandInputBuffer.clear();
}

NodeEstimationT _estimate(const UNodeEstimator & est, const UState & state, const UPartialState & goal)
{
	NodeEstimationT res = est(state, goal);
	return res;
}

void UMultiCoreSolver::estimateOutputBuffer()
{
	//vector<future<NodeEstimationT>> estimations;
	vector<future<NodeEstimationT>> estimations;
	estimations.reserve(m_expandOutputBuffer.size());

	for(auto & el : m_expandOutputBuffer)
		estimations.push_back(async(_estimate, ref(m_estimator), ref(el.state), m_goal));

	for(size_t i = 0; i < m_expandOutputBuffer.size(); ++i)
		m_estimatedOutput[estimations[i].get()].push_back(move(m_expandOutputBuffer[i]));

	m_expandOutputBuffer.clear();
}
