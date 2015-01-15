#ifndef UltraPlanner_USolver_h
#define UltraPlanner_USolver_h

#include "config.h"
#include "../pddl/UPDDLPredicates.h"
#include "../pddl/UPDDLTypes.h"
#include "../pddl/UPDDLDomain.h"
#include "../pddl/UPDDLAction.h"
#include "UInstantiator.h"
#include "UOutputMerger.h"
#include "USolverBase.h"
#include "USearchNode.h"
#include "database/UStateReference.h"
#include <core/transition_system/UTransitionSystem.h>
#include "UNodeEstimator.h"
#include "database/USearchDatabase.h"
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <functional>

struct UPDDLDomain;
struct UPDDLProblem;
struct UPDDLObject;
struct UPDDLType;

class UTransitionSystem;
class UState;
class UPartialState;
class UMetric;
class UHeuristic;

class ULTRA_SOLVER_API USolver : public USolverBase, public UProfiledSolverInterface
{
public:
	USolver(size_t solver_output_buffer_size);
	virtual ~USolver();

	virtual std::vector<SolverResult> solve(const UTransitionSystem & transition_system, const UStateReference & initial_state_ref, const UPartialState & goal_states, const UMetric * metric, const UHeuristic * heuristic, bool search_optimal);

	const UTransitionSystem & transitionSystem() const;
	
	//virtual UOutputMergerBase * createOutputMerger();
	void dump() const;

	std::vector<USearchNode> expandNode(const NodeElementType & node);

	virtual void clearBuffers();
	void clearDatabase();

	UStateReference state(size_t state_address)
	{
		return m_database[state_address];
	}
	
	//=====================Accessors============================
	//State database (in external memory)
	UStateFactory & stateFactory();
	const UStateFactory & stateFactory() const;

	//Node database (in external memory)
	USearchNodeFactory & nodeFactory();
	const USearchNodeFactory & nodeFactory() const;
private:
	void expandNode(const size_t node_index);
protected:
	//std::vector<USearchNode> mGoalStates;
	UTransitionSystem mTransitionSystem;
	UNodeEstimator m_estimator;


	//UStateFactory m_stateFactory;
	//USearchNodeFactory m_nodeFactory;
	USearchDatabase m_database;

	UOutputMerger m_merger;
};

/*
class USolverImpl : public USolverInterface
{
};*/

class USimpleSolver : public USolver
{
public:
	USimpleSolver(size_t solver_output_buffer_size);
	/*
	Expands input expansion buffer, generates new node and 	adds them to expand output buffer. In the end, clears
	the input buffer. If input buffer was overflowed, only processed elements will be deleted.
	*/
	virtual void expandInBuffer();
	virtual void estimateOutputBuffer() override;
	virtual void mergeEstimatedOutput() override;
};


#endif
