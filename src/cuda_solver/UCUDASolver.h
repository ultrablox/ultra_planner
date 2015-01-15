
#ifndef UCUDASolver_UCUDASolver_h
#define UCUDASolver_UCUDASolver_h

#include "config.h"
#include <solver/USolver.h>
#include <core/UError.h>

struct UCUDATaskContext;

class ULTRA_CUDA_SOLVER_API cudasolver_exception : public core_exception
{
public:
	cudasolver_exception(const char * msg)
		:core_exception(msg)
	{
	}
};

class ULTRA_CUDA_SOLVER_API UCUDASolver : public USolver
{
public:
	UCUDASolver();
	virtual ~UCUDASolver();
	virtual std::vector<SolverResult> solve(UTransitionSystem & transition_system, UState & initial_state, UPartialState & goal_states, const UMetric * metric = nullptr, const UHeuristic * heuristic  = nullptr);
	virtual void commit();
private:
	/*UConcurrentQueue<const USearchNode*> * m_pNodesToExpand;
	std::mutex mQueueMutex;
	std::vector<UExpandingThread*> mExpandThreads;
	int mCurrentExpandingStates;

	UConcurrentVector<const USearchNode*> * m_pOpenList;*/
	//std::vector<const USearchNode*> mExpandedNodes;
	UCUDATaskContext * m_pCudaContext;
};

#endif
