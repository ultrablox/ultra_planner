
#ifndef UltraPlanner_USolverBase_h
#define UltraPlanner_USolverBase_h

#include "config.h"
#include "USearchNode.h"
#include "database/USearchNodeFactory.h"
#include "database/UStateFactory.h"
#include "database/USearchNodeReference.h"
#include <core/UError.h>
#include <vector>
#include <chrono>

using namespace std::chrono;

class UMetric;

class USolverInterface
{
public:
	/*
	Expands nodes from input buffer, and stores their children to expandOutputBuffer.
	expandInputBuffer => expandOutputBuffer
	[n1, n2, ..nN] => [c1, ..., cM], M > N
	*/
	virtual void expandInBuffer() = 0;

	/*
	Estimates ouput buffer to the indexed internal memory structure (map).
	In the end clears output buffer.
	[c1, ..., cM] => [{e1, [c1_1, ..., c1_N1]}, ..., {eK, [cK_1, ..., cK_NK]}]
	*/
	virtual void estimateOutputBuffer() = 0;

	/*
	Merges estimated output to global database and checks if nodes
	are new, or it is duplication. In the end, clears estimated
	oputput buffer and creates references to new nodes in new buffer.
	*/
	virtual void mergeEstimatedOutput() = 0;
};


class UProfiledSolverInterface : public USolverInterface
{
public:
	void prExpandInBuffer();
	void prEstimateOutputBuffer();
private:
	double m_profileData;
};

/*
Solver base, containing node buffers. Buffers are stored in
internal memory. Also has an interface for manipulating these
buffers, but not it's implementation.
*/
class USolverBase
{
public:
	typedef UStateReference NodeElementType;
	typedef std::vector<NodeElementType> NodeLargeContainerType;
	typedef std::vector<USearchNode> NodeLocalContainerType;
	//typedef std::map<NodeEstimationT, std::vector<USearchNodeReference>, NodeEstimationT::BestSortOperator> NewBufferType;
	typedef std::vector<UStateReference> NewBufferType;
	typedef std::map<NodeEstimationT, NodeLocalContainerType, NodeEstimationT::BestSortOperator> EstimatedOuputType;
	
	USolverBase(size_t maxOutputBufferSize_/* = 300000ULL*/);
	virtual ~USolverBase();

	//===============Buffers accessors=====================

	void addToExpandInput(const NodeElementType & input_element)
	{
		m_expandInputBuffer.push_back(input_element);
	}

	NodeLargeContainerType & bufExpandInput()
	{
		return m_expandInputBuffer;
	}

	NewBufferType & bufExpandNew()
	{
		return m_expandNewBuffer;
	}

	EstimatedOuputType & bufEstimatedOutput()
	{
		return m_estimatedOutput;
	}

	/*
	Clears all buffers in internal memory.
	*/
	virtual void clearBuffers();

	//=================Goals accessors===========================
	/*
	Adds goal node to goals buffer.
	*/
	void addGoalNode(const NodeElementType & node_ref);

	/*
	Returns true, if at least one goal node was found.
	*/
	bool goalFound() const;

	void setCullMetric(bool cull_metric, float culling_metric, const UMetric * metric);
	float bestGoalMetric(const UMetric * metric) const;
	float cullingMetric() const;

	/*
	Sets goal description which will be compared to all generated nodes.
	If goal is changed, goals buffer is not cleared.
	*/
	void setGoal(const UPartialState & goal_);

	const UPartialState & goal() const
	{
		return m_goal;
	}

	/*
	Returns maximum input buffer element count that is calculated
	based on current ratio and max output size limitation. Don't expand
	in one step more input nodes, then this method returns.
	*/
	size_t approximateInputMaxSize() const;

	bool outputToNewBuffer() const
	{
		return m_outputToNewBuffer;
	}

	void setMetric(const UMetric * p_metric)
	{
		m_pMetric = p_metric;
	}
protected:
	std::vector<NodeElementType> mExpansionBuffer;
	

	/*
	Input buffer is deferred container, with node references. Nodes
	data is loaded only during the expansion process. THe container
	doesn't include the real nodes data, only references. So, the
	data can be unloaded into the RAM.
	*/
	NodeLargeContainerType m_expandInputBuffer;
	
	NodeLocalContainerType m_expandOutputBuffer;

	EstimatedOuputType m_estimatedOutput;
	NewBufferType m_expandNewBuffer;

	NodeLargeContainerType m_goalNodes;

	UPartialState m_goal;

	/*
	This ratio is average assesment of expanded node count. You can use
	it for approximately advance space allocation. Expanded node count ~=
	Input node count * m_expansionRatio;
	*/
	URatio m_expansionRatio;

	bool m_outputToNewBuffer;
protected:
	NodeLargeContainerType mOpenList;

	/*
	A number of maximum states, that can appear after expansion. This limitation
	is added to prevent memory overusage.
	*/
	size_t m_maxOutputBufferSize;
	
	bool m_cullMetric;
	float m_cullingMetric;
	const UMetric * m_pMetric;
};

#endif
