
#include "UCUDASolver.h"
#include <cuda_runtime_api.h>
#include "kernel.cuh"

void print_device_info(const cudaDeviceProp & device_props)
{
	cout << "Device name: " << device_props.name << "\n";

	cout << "Total global memory: " << device_props.totalGlobalMem << "\n";
	cout << "Shared memory per block: " << device_props.sharedMemPerBlock << "\n";
	cout << "Registers per block: " << device_props.regsPerBlock << "\n";
	cout << "Warp size: " << device_props.warpSize << "\n";
	cout << "Memory pitch: " << device_props.memPitch << "\n";
	cout << "Max threads per block: " << device_props.maxThreadsPerBlock << "\n";
    
	cout << "Max threads dimensions: x = " << device_props.maxThreadsDim[0] << ", y = " << device_props.maxThreadsDim[1] << ", z = " << device_props.maxThreadsDim[2] << "\n";
    
	cout << "Max grid size: x = " << device_props.maxGridSize[0] << ", y = " << device_props.maxGridSize[1] << ", z = " << device_props.maxGridSize[2] << "\n";

	cout << "Clock rate: " << device_props.clockRate << "\n";
	cout << "Total constant memory: " << device_props.totalConstMem << "\n"; 
	cout << "Compute capability: " << device_props.major << "." << device_props.minor << "\n";
	cout << "Texture alignment: " << device_props.textureAlignment << "\n";
	cout << "Device overlap: " << device_props.deviceOverlap << "\n";
	cout << "Multiprocessor count: " << device_props.multiProcessorCount << "\n";

	cout << "Kernel execution timeout enabled: " << (device_props.kernelExecTimeoutEnabled ? "true" : "false") << "\n";
}

UCUDASolver::UCUDASolver()
{
	cout << "Initializing CUDA... ";
	int deviceCount;
	cudaDeviceProp deviceProp;
	cudaGetDeviceCount(&deviceCount);
	cout << deviceCount << " CUDA devices found.\n";

	for (int i = 0; i < deviceCount; i++)
	{
		cudaGetDeviceProperties(&deviceProp, i);
		print_device_info(deviceProp);
	}

	m_pCudaContext = new UCUDATaskContext();
}

UCUDASolver::~UCUDASolver()
{

}

/*
void UCUDASolver::expandNode(const USearchNode * node)
{
	m_pOutputMerger->makeNodeExplored(node);
	
	auto actions_indices(get_active_transitions(m_pCudaContext, &node->state));

	for(auto ai : actions_indices)
	{
		USearchNode * new_node = new USearchNode(mTransitionSystem.applyTransition(node->state, ai), node->transitions);
		new_node->transitions.push_back(ai);
		if(!m_pOutputMerger->mergeNewNode(new_node))
			delete new_node;
	}
}*/

std::vector<SolverResult> UCUDASolver::solve(UTransitionSystem & transition_system, UState & initial_state, UPartialState & goal_states, const UMetric * metric, const UHeuristic * heuristic)
{
	/*auto actions_indices(transition_system.getAvalibleTransitions(initial_state));

	cout << "real active actions:\n";

	for(auto ai : actions_indices)
	{
		cout << "\t" << ai << "\n";
	}

	//cout << "7th state\n";
	//transition_system.transition(7).print();*/

	m_pCudaContext->transitionCount = transition_system.transitions().size();
	m_pCudaContext->stateSize = initial_state.plainDataSize();

	save_transitions(&transition_system, m_pCudaContext);

	return USolver::solve(transition_system, initial_state, goal_states, metric, heuristic);
}

void UCUDASolver::commit()
{
	for(auto nn : mExpansionBuffer)
		m_pOutputMerger->makeNodeExplored(nn);

	auto new_nodes = expand_nodes(m_pCudaContext, mExpansionBuffer.begin(), mExpansionBuffer.size());
	mExpansionBuffer.clear();

	for(auto new_node : new_nodes)
	{
		if(m_pOutputMerger->mergeNewNode(new_node))
		{
			addToOpenList(new_node);
		}
		else
			delete new_node;
	}
}
