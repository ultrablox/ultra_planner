
#ifndef UCUDASolver_CUBitset_h
#define UCUDASolver_CUBitset_h

#include "cuda_runtime.h"
//#include "kernel.cuh"
#include "helpers.cuh"

template<typename T>
struct CUBitSet
{
	__device__ CUBitSet()
	{}

	/*__device__ CUBitSet(const CUBitSet & other)
		:elementCount(other.elementCount)
	{
		printf("copying bitset\n");
	}*/

	__device__ size_t serialize(char * dest) const
	{
		size_t res = 0;

		//Byte Count
		res += cu_serialize_int(dest, elementCount);

		//Byte Data
		const size_t byte_count = elementCount * sizeof(T);
		memcpy(&dest[res], data, byte_count);
		res += byte_count;

		return res;
	}

	__device__ size_t deserialize(char * cdata)
	{
		size_t res(0);

		res += cu_deserialize_int(cdata, elementCount);
		//printf("count=%d\n", elementCount);

		data = (T*)(cdata + res);
		//printf(":%d:", *(data+1));
		res += elementCount * sizeof(T);

		return res;
	}

	/*__device__ size_t mapFromMemory(char * address)
	{
		size_t r(0);
		&elementCount = address;
		r += sizeof(int32_t);

		r += elementCount * sizeof(T);

		return r;
	}*/

	__device__ bool equalMasked(const CUBitSet & val, const CUBitSet & mask) const
	{
		T * cur_ptr = data, *val_ptr = val.data, *mask_ptr = mask.data;

		for(size_t i = elementCount; i > 0 ; --i)
		{
			if(!((*val_ptr) == ((*cur_ptr) & (*mask_ptr))))
				return false;

			++cur_ptr;
			++val_ptr;
			++mask_ptr;
		}

		return true;
	}

	__device__ void setMasked(const CUBitSet & value, const CUBitSet & mask)
	{
		T * cur_ptr = data;
		const T * val_ptr = value.data, *mask_ptr = mask.data;

		for(size_t el_index = 0; el_index < elementCount; ++el_index)
		{
			*cur_ptr = ((*cur_ptr) & (~(*mask_ptr))) | ((*val_ptr) & (*mask_ptr));
			++cur_ptr;
			++val_ptr;
			++mask_ptr;
		}
	}

	__device__ static int bitIndex(size_t global_bit_index)
	{
		return global_bit_index % 32;
	}

	__device__ static void getBitAddress(size_t global_bit_index, size_t & byte_index, int & bit_index)
	{
		//Get byte index
		byte_index = global_bit_index / (sizeof(T)*8);
		
		//Calculate local bit index
		bit_index = global_bit_index % (sizeof(T)*8);
	}

	__device__ bool operator[](const size_t global_bit_index) const
	{
		size_t byte_index(0);
		int bit_index(0);

		getBitAddress(global_bit_index, byte_index, bit_index);
		//int bit_index = bitIndex(global_bit_index);

		//printf("addr: %d\n", bit_index);
		//printf("addr: %d = %d, %ld\n", global_bit_index, byteIndex, bit_index);
		T val = 1 << bit_index;
		T result = data[byte_index] & val;
		return bool(result);
	}

	__device__ void print() const
	{
		const size_t bit_count = elementCount * sizeof(T) * 8;
		//printf("Contains %d bits\n", bit_count);

		for(size_t i = 0; i < bit_count; ++i)
		{
			if(operator[](i))
				printf("1");
			else
				printf("0");
		}
	}

	int elementCount;
	T * data;
};

typedef CUBitSet<int> CUBitVector;


struct CUMaskedBitVector
{
	__device__ CUMaskedBitVector()
	{}

	__device__ size_t deserialize(char * cdata)
	{
		size_t res(0);

		res += state.deserialize(cdata);
		res += mask.deserialize(cdata + res);

		return res;
	}

#if _DEBUG
	__device__ void print()
	{
		printf("S:");
		state.print();
		printf("\n");
		printf("M:");
		mask.print();
		printf("\n");
	}
#endif

	CUBitVector state, mask;
};

#endif
