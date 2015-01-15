
#ifndef UltraMultiCoreSolver_UConcurrentVector_h
#define UltraMultiCoreSolver_UConcurrentVector_h

#include <tbb/concurrent_vector.h>

template<typename T> class UConcurrentVector : public tbb::concurrent_vector<T>
{
};

#endif
