
#include "USolverBase.h"
#include "UMetric.h"

//============================Profiled interface================
void UProfiledSolverInterface::prExpandInBuffer()
{
	auto start_tp = high_resolution_clock::now();

	cout << "Expanding input buffer... ";

	//cout << "Expanding input buffer (" << m_expandInputBuffer.size() << " nodes)...\n";//, " << fixed << setprecision(2) <<  static_cast<double>(m_expandInputBuffer.size()) * 100.0 / static_cast<double>(approximateInputMaxSize()) << "% of maximum
	expandInBuffer();

	auto end_tp = high_resolution_clock::now();
	cout << " finished in " << duration_cast<microseconds>(end_tp - start_tp).count() * 1e-6 << " sec.\n";
}

void UProfiledSolverInterface::prEstimateOutputBuffer()
{
	auto start_tp = high_resolution_clock::now();

	cout << "Estimating output buffer...";

	estimateOutputBuffer();

	auto end_tp = high_resolution_clock::now();
	cout << " finished in " << duration_cast<microseconds>(end_tp - start_tp).count() * 1e-6 << " sec.\n";
}

//=============================Solver Base=====================
USolverBase::USolverBase(size_t maxOutputBufferSize_)
	:m_outputToNewBuffer(true), m_maxOutputBufferSize(maxOutputBufferSize_), m_cullMetric(false)
{
}

USolverBase::~USolverBase()
{
}

void USolverBase::clearBuffers()
{
	m_expandInputBuffer.clear();
	m_expandInputBuffer.shrink_to_fit();

	m_expandOutputBuffer.clear();
	m_expandOutputBuffer.shrink_to_fit();

	m_estimatedOutput.clear();

	m_expandNewBuffer.clear();
}

void USolverBase::addGoalNode(const NodeElementType & node_ref)
{
	if(m_cullMetric)
	{
		if(m_cullingMetric > (*m_pMetric)(node_ref))
		{
			m_cullingMetric = (*m_pMetric)(node_ref);
			m_goalNodes.clear();
			m_goalNodes.push_back(node_ref);
		}
		else if(m_cullingMetric == (*m_pMetric)(node_ref))
			m_goalNodes.push_back(node_ref);
	}
	else
		m_goalNodes.push_back(node_ref);
}

bool USolverBase::goalFound() const
{
	return !(m_goalNodes.empty());
}

void USolverBase::setCullMetric(bool cull_metric, float culling_metric, const UMetric * metric)
{
	m_cullMetric = cull_metric;
	if(m_cullMetric)
	{
		m_cullingMetric = culling_metric;

		auto last_it = remove_if(m_goalNodes.begin(), m_goalNodes.end(), [=](const UStateReference & ref){
			return (*metric)(ref) > culling_metric;
		});
		m_goalNodes.erase(last_it, m_goalNodes.end());
	}
}

float USolverBase::bestGoalMetric(const UMetric * metric) const
{
	auto it = std::min_element(m_goalNodes.begin(), m_goalNodes.end(), [=](const UStateReference & ref1, const UStateReference & ref2){
		return (*metric)(ref1) < (*metric)(ref2);
	});

	if(it == m_goalNodes.end())
		throw runtime_error("No goal nodes to choose from");

	return (*metric)(*it);
}

float USolverBase::cullingMetric() const
{
	return m_cullingMetric;
}

void USolverBase::setGoal(const UPartialState & goal_)
{
	m_goal = goal_;
}

size_t USolverBase::approximateInputMaxSize() const
{
	/*if(m_expansionRatio.value() > 1.0)
		return max_output_buffer_size / m_expansionRatio.value();
	else*/
		return m_maxOutputBufferSize;
}