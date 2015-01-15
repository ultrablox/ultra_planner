
#ifndef UltraMultiCoreSolver_UMultiCoreSolver_h
#define UltraMultiCoreSolver_UMultiCoreSolver_h

#include "config.h"
#include <solver/USolver.h>
#include <thread>
#include <mutex>
#include <tbb/concurrent_unordered_set.h>

template<typename T> class UConcurrentQueue;
template<typename T> class UConcurrentVector;

class UMultiCoreSolver;
/*
class ULTRA_MCSOLVER_API UMultiCoreSolver : public USolver
{
public:
	UMultiCoreSolver();
	virtual ~UMultiCoreSolver();
	virtual void expandNode(const USearchNode * node);
	virtual bool ready();
	const USearchNode * getNodeToExpand();
	void addExpandedNode(const USearchNode * node);
	void addExpandedNodes(const std::vector<const USearchNode*> & nodes);
	void nodeExpanded();
	virtual bool finished();
	virtual void commit();
	virtual UOutputMergerBase * createOutputMerger();
	virtual std::vector<const USearchNode*> getOpenListAndClear();
	virtual void unexploredNodeAdded(const USearchNode * new_node);
private:
	UConcurrentQueue<const USearchNode*> * m_pNodesToExpand;
	std::mutex mQueueMutex;//, mMergerMutex, mLocalMergeMutex;
	int mCurrentExpandingStates;

	UConcurrentVector<const USearchNode*> * m_pOpenList;
	//OutputMergerType * pOutputMerger;
	//std::vector<const USearchNode*> mExpandedNodes;
};
*/


class UMultiCoreSolver : public USimpleSolver
{
public:
	UMultiCoreSolver(size_t solver_output_buffer_size);

	virtual void expandInBuffer() override;
	virtual void estimateOutputBuffer() override;
};

#endif
