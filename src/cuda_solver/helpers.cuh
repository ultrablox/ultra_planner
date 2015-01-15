
#ifndef UltraCudaCoreSolver_helpers_h
#define UltraCudaCoreSolver_helpers_h

#include "cuda_runtime.h"
#include <stdint.h>

/*namespace CudaSolver
{
__device__ size_t deserialize_int(char * data, int & value);

//__device__ int cu_deserialize_int(char * data);
__device__ size_t cu_serialize_int(char * dest, const int32_t val);
};
*/

__device__ size_t cu_deserialize_int(char * data, int & value);

__device__ size_t cu_serialize_int(char * dest, const int32_t val);


template<int N> size_t align_memory_size(int src_size)
{
	return src_size + (N-(src_size % N));
}


#endif
