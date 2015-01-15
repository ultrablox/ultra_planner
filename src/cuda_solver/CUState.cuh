
#ifndef UCUDASolver_CUState_h
#define UCUDASolver_CUState_h

#include "CUBitset.cuh"
#include "CUTransition.cuh"
#include "cuda_runtime.h"

struct CUFloatVector
{
	__device__ size_t deserialize(char * plain_data)
	{
		size_t r(0);

		r += cu_deserialize_int(plain_data, varCount);

		data = (float*)(plain_data + r);

		r += varCount * sizeof(float);

		return r;
	}

	__device__ size_t serialize(char * dest) const
	{
		size_t r(0);
		
		r += cu_serialize_int(dest, varCount);

		const size_t data_size = sizeof(float) * varCount;
		memcpy(dest + r, data, data_size);
		r += data_size;

		return r;
	}

	int varCount;
	float * data;
};

struct CUState
{
	__device__ CUState()
	{}

	__device__ CUState(void * plain_data)
	{
		deserialize((char*)plain_data);
	}

	/*__device__ void mapFromMemory(char * address)
	{
		size_t r(0);
		r += flags.mapFromMemory(address);
		//r += floatVars.mapFromMemory(address + r);
	}*/

	__device__ size_t deserialize(char * plain_data)
	{
		size_t r(0);
		r += flags.deserialize(plain_data);
		r += floatVars.deserialize(plain_data + r);
		return r;
	}

	__device__ size_t serialize(char * dest) const
	{
		size_t r(0);
		r += flags.serialize(dest);
		r += floatVars.serialize(dest + r);
		return r;
	}

	__device__ void print() const
	{
		flags.print();
	}
	
	__device__ void apply(const CUTransition & transition)
	{
		//Flags changing - very fast
		flags.setMasked(transition.effect.flags.state, transition.effect.flags.mask);
	
		//Numeric changes - the slowest part
		for(int i = 0; i < transition.effect.numeric.count; ++i)
		{
			auto num_eff = transition.effect.numeric.data + i;
			float * val = floatVars.data + i;

			switch (static_cast<UNumericEffect::Type>(num_eff->type))
			{
			case UNumericEffect::Type::Assign:
				*val = num_eff->value;
				break;
			case UNumericEffect::Type::Decrease:
				*val -= num_eff->value;
				break;
			case UNumericEffect::Type::Increase:
				*val += num_eff->value;
				break;
			case UNumericEffect::Type::ScaleDown:
				*val /= num_eff->value;
				break;
			case UNumericEffect::Type::ScaleUp:
				*val *= num_eff->value;
				break;
			case UNumericEffect::Type::NoEffect:
				break;
			}
		}
	}

	CUBitVector flags;
	CUFloatVector floatVars;
};
/*
__device__ CUState operator+(const CUState & state, const CUTransition & transition)
{
	printf("\nB:");
	state.print();
	CUState new_state(state);
	new_state.apply(transition);

	printf("\nA:");
	new_state.print();
	return new_state;
}
*/
#endif
