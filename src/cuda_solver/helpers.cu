
#include "helpers.cuh"
#include <stdio.h>
#include <string>
#include <stdint.h>
/*
using namespace CudaSolver;

__device__ size_t deserialize_int(char * data, int & value)
{
	int32_t val;
	memcpy(&val, data, sizeof(int32_t));
	value = val;
	return sizeof(int32_t);
}

__device__ size_t cu_serialize_int(char * dest, const int32_t val)
{
	memcpy(dest, &val, sizeof(val));
	return sizeof(int32_t);
}
*/

__device__ size_t cu_deserialize_int(char * data, int & value)
{
	int32_t val;
	memcpy(&val, data, sizeof(int32_t));
	value = val;
	return sizeof(int32_t);
}

__device__ size_t cu_serialize_int(char * dest, const int32_t val)
{
	memcpy(dest, &val, sizeof(val));
	return sizeof(int32_t);
}
