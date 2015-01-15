
#ifndef UCUDASolver_CUTransition_h
#define UCUDASolver_CUTransition_h

#include "CUBitset.cuh"
#include "CUState.cuh"
#include "cuda_runtime.h"
#include <stdio.h>


struct CUNumericEffect
{
	float value;
	unsigned char type;
};

struct CUTransition
{
	__device__ CUTransition(void * plain_data)
	{
		deserialize((char*)plain_data);
	}

	__device__ size_t deserialize(char * plain_data)
	{
		//printf("XXXX:%d\n", *((int*)plain_data));

		size_t res(0);

		res += condition.flags.deserialize(plain_data);
		
		//=================Effect============
		//Flags
		res += effect.flags.deserialize(plain_data + res);

		//Numeric
		res += cu_deserialize_int(plain_data + res, effect.numeric.count);
		effect.numeric.data = (CUNumericEffect*)(plain_data + res);
		//printf("Numeric count: %d\n", effect.numeric.count);

		return res;
	}

#if _DEBUG
	__device__ void print()
	{
		printf("Transition:\n");

		printf("condition:\n");
		condition.flags.print();

		printf("effect:\n");
		effect.flags.print();
	}
#endif

	struct
	{
		CUMaskedBitVector flags;
	} condition;

	struct
	{
		CUMaskedBitVector flags;
		struct
		{
			int count;
			CUNumericEffect * data;
		} numeric;
	} effect;
};

#endif
