
#ifndef UltraSolver_USearchNodeFactory_h
#define UltraSolver_USearchNodeFactory_h

#include "../config.h"
#include "../USearchNode.h"
#include "../UNodeEstimator.h"
#include "USearchNodeReference.h"
#include <core/UError.h>
#include <stxxl.h>
#include <mutex>
#include <unordered_map>

class UState;

class USearchNodeFactory;


struct USearchNodeData
{
	USearchNodeData(const StateAddress & parentAddress_ = StateAddress(), size_t stateIndex_ = 0, bool expanded_ = false);

	StateAddress parentIndex;
	size_t stateIndex;
	bool expanded;
};

class ULTRA_SOLVER_API USearchNodeFactory
{
	friend class USearchNodeReference;
public:
#if USE_HDD_STORAGE
	typedef stxxl::VECTOR_GENERATOR<USearchNodeData, 8U, 2, 512*1024, stxxl::RC, stxxl::lru>::result node_data_container_type;
	typedef stxxl::VECTOR_GENERATOR<UNodeReference>::result node_ref_container_type;
#else
	typedef std::vector<USearchNodeData> node_data_container_type;
#endif
	typedef std::map<NodeEstimationT, node_data_container_type, NodeEstimationT::BestSortOperator> data_type;

	USearchNodeFactory();

	/*
	Creates new node with parent and returns reference to it.
	*/
	USearchNodeReference createNode(const NodeEstimationT & estimation_data, const UStateReference & state_ref);
	UNodeReference createNode(const UStateReference & state_ref);

	node_data_container_type & nodeGroup(const NodeEstimationT & group)
	{
		return m_data_[group];
	}

	//USearchNodeReference groupBegin(const NodeEstimationT & group) const;
	//USearchNodeReference groupEnd(const NodeEstimationT & group) const;

	NodeEstimationT bestGroupIndex();

	/*
	Returns reference to node with given address.
	*/
	//USearchNodeReference nodeRef(const NodeAddressT & address) const;
	//USearchNodeReference nodeRef(const USearchNodeData & data, const NodeAddressT & address) const;
	UNodeEstimatedRef estimatedNodeRef(const UStateReference & state_ref) const;

	/*
	Finds a set of nodes from given group. Maximum returned node count is
	max_count.
	*/
	//std::vector<USearchNodeReference> getUnexpandedNodes(const NodeEstimationT & node_group, const size_t max_count) const;
	size_t expandedNodeCount() const;

	void flush() const;
	void clear();

	const data_type & data() const;
	static USearchNodeFactory * m_pSingleton;

	void serialize(ofstream & fout) const;
	int deserialize(ifstream & fin);


private:
	std::mutex mMutex;
protected:
	data_type m_data_;

	std::map<NodeEstimationT, node_ref_container_type> m_data;
};

#endif
