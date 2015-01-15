
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
//#include <core/transition_system/UState.h>
//#include "CUState.h"
#include <core/transition_system/UTransitionSystem.h>
#include <stdio.h>
#include <vector>
#include "kernel.cuh"
#include "helpers.cuh"
#include "cuPrintf.cuh"
#include "CUBitset.cuh"
#include "CUState.cuh"
#include "CUTransition.cuh"

__device__ bool transition_is_active(const CUState & state, const CUTransition & transition)
{
	bool r = state.flags.equalMasked(transition.condition.flags.state, transition.condition.flags.mask);
	if(!r)
		return false;

	r = state.flags.equalMasked(transition.effect.flags.state, transition.effect.flags.mask);
	if(r)
		return false;

	return true;
}

//__constant__ char cTransitionsBuffer[40000];

//Create memory for transitions and copy all transitions
__host__ void save_transitions(const UTransitionSystem * transition_system, UCUDATaskContext * task_context)
{
	cudaError_t r = cudaSuccess;

	//Calculate maximum transition size
	size_t max_transition_size = 0;
	for(auto tr : transition_system->transitions())
		max_transition_size = max(max_transition_size, tr.plainDataSize());
	task_context->transitionSize = align_memory_size<4>(max_transition_size);

	//Allocate memory for all transitions
	const size_t data_size = task_context->transitionSize * transition_system->transitions().size();
	r = cudaMalloc(&(task_context->transitionsData), data_size);
	printf("Total transitions size: %d bytes.\n", data_size);

	//Copy them to GPU
	char * transitions_data = (char*)malloc(data_size);
	for(int i = 0; i < transition_system->transitions().size(); ++i)
	{
		auto tr = transition_system->transition(i);
		size_t tr_size = tr.serialize(transitions_data + i*task_context->transitionSize);
	}

	r = cudaMemcpy(task_context->transitionsData, transitions_data, data_size, cudaMemcpyHostToDevice);
	//cudaMemcpyToSymbol(cTransitionsBuffer, transitions_data, data_size);

	free(transitions_data);
}

/*
Expands states in parallel where:
- each state to expand is a block
- each transition is a thread
*/
__global__ void expand_state(void * states_buffer, size_t state_size, void * transitions_buffer, const size_t transition_size, unsigned int * result_state_count, int * source_states, int * applied_transitions, char * result_states_data)
{
	//Get current state
	size_t state_index = blockIdx.x;
	void * state_addr = (char*)states_buffer + state_index * state_size;
	CUState state(state_addr);

	//Get current transition
	size_t transition_index = threadIdx.x;
	void * transition_addr = (char*)transitions_buffer + transition_index * transition_size;
	//void *transition_addr = cTransitionsBuffer + + transition_index * transition_size;
	CUTransition transition(transition_addr);
	//transition.print();

	//Check if transition is active
	if(transition_is_active(state, transition))
	{
		//printf("Active!\n");
		//Increase result state count
		unsigned int new_state_index = atomicInc(result_state_count, UINT32_MAX);

		//Create new state
		char * new_state_data_addr = result_states_data + new_state_index * state_size;

		size_t ss = state.serialize(new_state_data_addr);
		CUState new_state;
		size_t ds = new_state.deserialize(new_state_data_addr);
		
		//Apply transition and automatically write new state to buffer
		new_state.apply(transition);
	
		//Write used transition to buffer
		applied_transitions[new_state_index] = transition_index;

		//Write source state index to buffer
		source_states[new_state_index] = state_index;
	}
}

__host__ ExpansionResult expand_states(UCUDATaskContext * task_context, size_t state_count, char *& source_states_buffer)
{
	//=========Create memory bufferof on GPU=============
	//Result counter
	unsigned int * cu_res_state_count;
	cudaMalloc(&cu_res_state_count, sizeof(unsigned int));
	cudaMemset(cu_res_state_count, 0, sizeof(unsigned int));

	const size_t max_result_count = task_context->transitionCount * state_count;
	//Result states data
	char * cu_states_buffer;
	cudaMalloc(&cu_states_buffer, max_result_count * task_context->stateSize);

	//Result source state indices
	int * cu_source_states;
	cudaMalloc(&cu_source_states, max_result_count * sizeof(int));

	//Applied transition index
	int * cu_applied_transitions;
	cudaMalloc(&cu_applied_transitions, max_result_count * sizeof(int));
	
	//Run kernel
	//
	expand_state<<<state_count, task_context->transitionCount>>>(source_states_buffer, task_context->stateSize, task_context->transitionsData, task_context->transitionSize, cu_res_state_count, cu_source_states, cu_applied_transitions, cu_states_buffer);

	int result_state_count;
	cudaMemcpy(&result_state_count, cu_res_state_count, sizeof(int), cudaMemcpyKind::cudaMemcpyDeviceToHost);

	//printf("After expansion got %d nodes.\n", result_state_count);

	//=================Copy results back to RAM==================
	int * source_states = (int*)malloc(result_state_count * sizeof(int));
	int * applied_transitions = (int*)malloc(result_state_count * sizeof(int));
	char * states_buffer = (char*)malloc(max_result_count * task_context->stateSize);

	cudaMemcpy(source_states, cu_source_states, result_state_count * sizeof(int), cudaMemcpyKind::cudaMemcpyDeviceToHost);
	cudaMemcpy(applied_transitions, cu_applied_transitions, result_state_count * sizeof(int), cudaMemcpyKind::cudaMemcpyDeviceToHost);
	cudaMemcpy(states_buffer, cu_states_buffer, result_state_count * task_context->stateSize, cudaMemcpyKind::cudaMemcpyDeviceToHost);

	
	ExpansionResult res;
	res.elements = new ExpansionElement[result_state_count];
	res.count = result_state_count;
	//res.reserve(result_state_count);

	//Deserialize result
	for(int i = 0; i < result_state_count; ++i)
	{
		ExpansionElement * el = res.elements + i;
		el->sourceStateIndex = source_states[i];
		el->transitionIndex = applied_transitions[i];
		el->state.deserialize(states_buffer + task_context->stateSize * i);
	}

	//Clear RAM
	free(source_states);
	free(applied_transitions);
	free(states_buffer);

	//Clear CUDA memory
	cudaFree(cu_res_state_count);
	cudaFree(cu_states_buffer);
	cudaFree(cu_source_states);
	cudaFree(cu_applied_transitions);

	return std::move(res);
}
