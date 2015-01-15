
#ifndef UltraCudaCoreSolver_kernel_h
#define UltraCudaCoreSolver_kernel_h

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "device_launch_parameters.h"

#include <solver/USearchNode.h>
#include <stdio.h>
#include <list>

class UState;
class UTransitionSystem;

struct UCUDATaskContext
{
	size_t flagCount;
	int transitionSize;
	int transitionCount;
	char * transitionsData;
	size_t stateSize;
};

__global__ void simple_kernel(void);
__host__ std::list<size_t> get_active_transitions(UCUDATaskContext * task_context, const UState * state);
__host__ void save_transitions(const UTransitionSystem * transition_system, UCUDATaskContext * task_context);


struct ExpansionElement
{
	int sourceStateIndex, transitionIndex;
	UState state;
};

struct ExpansionResult
{
	ExpansionElement * elements;
	int count;
};


__host__ ExpansionResult expand_states(UCUDATaskContext * task_context, size_t state_count, char *& states_buffer);


template<class Iter>
__host__ vector<const USearchNode *> expand_nodes(UCUDATaskContext * task_context, const Iter begin_it, const size_t state_count)
{
	//Allocate memory for states
	char * states_buffer;
	cudaMalloc(&states_buffer, state_count * task_context->stateSize);

	//Serialize and copy states to GPU
	const size_t states_data_size = task_context->stateSize * state_count;

	void * states_data = malloc(states_data_size);

	int state_index = 0;
	
	auto it = begin_it;
	while(state_index < state_count)
	{
		(*it)->state.serialize((char*)states_data + state_index * task_context->stateSize);
		
		++it;
		++state_index;
	}
	
	cudaMemcpy(states_buffer, states_data, states_data_size, cudaMemcpyHostToDevice);

	free(states_data);


	//Execute main expansion
	auto new_states = expand_states(task_context, state_count, states_buffer);
	
	vector<const USearchNode *> result(new_states.count);

	//Syncrhonize expansion results and create new search nodes
	//for(auto ns : new_states)
	for(int i = 0; i < new_states.count; ++i)
	{
		auto ns = new_states.elements + i;

		auto src_node = *(begin_it + ns->sourceStateIndex);
		
		auto new_node = new USearchNode(ns->state, src_node);//src_node->transitions);
		//new_node->addTransition(ns->transitionIndex);
		result[i] = new_node;
	}

	delete [] new_states.elements;

	//Clear memory
	cudaFree(states_buffer);

	return std::move(result);
}

#endif
