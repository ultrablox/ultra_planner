
#ifndef UltraPlanner_UOutputMerger_h
#define UltraPlanner_UOutputMerger_h

#include "config.h"
#include "USearchNode.h"
#include "database/USearchDatabase.h"
#include "USolverBase.h"
#include <unordered_set>
#include <set>
#include <mutex>
#include <fstream>

class USearchNodeFactory;

/*
Base for any output merger.
*/
class ULTRA_SOLVER_API UOutputMergerBase
{
public:
	UOutputMergerBase(USearchNodeFactory * node_factory);
	
	/*
	Adds node to database and returns true, if node is added,
	or false, if such node already exists. If node with very
	similar state exists, but new node's metric is better - 
	it will replace the old node with this new and also returns
	true.
	
	virtual bool mergeNewNode(const size_t node_index) = 0;
*/
	/*
	Checks if node with such state exists, if yes - returns false.
	If doesn't exist - will create new node in node factory and
	initialize it with given parameters, and write it's id into
	last parameter.
	*/
	/*virtual bool mergeNewNode(const UState & state, size_t parent_node_index, size_t & new_node_index)
	{
		return false;
	}

	virtual void mergeNewNodes(const std::vector<const USearchNode*> & nodes)
	{
		for(auto n : nodes)
			mergeNewNode(n);
	}
	virtual void makeNodeExplored(const USearchNode * explored_node) = 0;
	virtual size_t exploredNodesCount() const = 0;
	virtual size_t unexploredNodesCount() const = 0;
	virtual bool contains(const size_t node_index) = 0;
	virtual void dump() const {};*/
protected:
	//USearchNodeFactory * m_pNodeFactory;
};

class USolver;

class UOutputMerger
{
	static const size_t max_count_in_internal = 1000000ULL;
public:
	UOutputMerger(USolver & solver_);
	void merge();
	void clearCache();
	void preloadLocalMergingCache(const NodeEstimationT & group_index);
private:
	size_t mergeInInternalMemory(const NodeEstimationT & group_index, USolverBase::NodeLocalContainerType & new_nodes);
	size_t mergeInExternalMemory(const NodeEstimationT & group_index, USolverBase::NodeLocalContainerType & new_nodes);
	StateAddress createNewNode(const NodeEstimationT & group_index, const UState & int_mem_node, StateAddress addr);
private:
	USolver & m_solver;

	struct
	{
		NodeEstimationT group_index;
		std::unordered_set<const USearchNode, StateHash> storage;
	} m_mergingCache;
};

#endif
