
#ifndef UltraMultiCoreSolver_UConcurrentQueue_h
#define UltraMultiCoreSolver_UConcurrentQueue_h

#include <tbb/concurrent_queue.h>

template<typename T> class UConcurrentQueue : public tbb::concurrent_queue<T>
{
};

#endif
