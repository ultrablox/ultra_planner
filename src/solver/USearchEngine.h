
#ifndef UltraPlanner_USearchEngine_h
#define UltraPlanner_USearchEngine_h

#include "USearchNode.h"
#include "USolver.h"
#include <core/transition_system/UTransitionSystem.h>
#include <list>

//class USolverBase;
class UHeuristic;
class UMetric;

class USearchEngine
{
public:
	USearchEngine(USolver * p_solver, const UTransitionSystem & transition_system);
	void expandNodes(std::list<size_t> & nodes);
	void printMemoryUsage() const;
	void expandNodes(std::vector<UStateReference> & nodes);
	//std::vector<const USearchNode*> openList();
	void dump();
protected:
	USolver * m_pSolver;
	const UTransitionSystem & mTransitionSystem;
};

struct QueueGreater
{
    bool operator () (const UNodeEstimatedRef & a, const UNodeEstimatedRef & b) const;
    UNodeEstimatedRef min_value() const;
};

const unsigned int mem_for_pools = 16 * 1024 * 1024;

class AStarSearchEngine : public USearchEngine
{
	typedef stxxl::PRIORITY_QUEUE_GENERATOR<UNodeEstimatedRef, QueueGreater, 128*1024*1024, 1024*1024>::result queue_type;
	//typedef queue_type::block_type block_type;
	enum class state_t {Preparing, SearchingFirst, SearchingBest, Finished};
public:
	AStarSearchEngine(USolver * p_solver, const UTransitionSystem & transition_system, const UHeuristic * heuristic, const UMetric * metric);
	void execute(const UStateReference & initial_state_ref, bool search_optimal);
	
	/*
	Returns best from 2 given nodes based on metric. Returns true,
	if first node is the best.
	*/
	bool bestGoalNode(const USearchNode * n1, const USearchNode * n2) const;
	std::vector<UStateReference> pickBestNodes(size_t min_count, size_t max_count);
private:
	const UHeuristic * m_pHeuristic;
	const UMetric * m_pMetric;

	//stxxl::read_write_pool<block_type> pool;
	//queue_type m_searchQueue;
	state_t m_state;
};

#endif
