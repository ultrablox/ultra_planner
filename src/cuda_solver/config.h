
#ifndef UltraCudaCoreSolver_config_h
#define UltraCudaCoreSolver_config_h

#ifdef ULTRA_CUDA_SOLVER_EXPORT
	#define ULTRA_CUDA_SOLVER_API __declspec(dllexport)
	#define ULTRA_SOLVER_IMPORT 1
#elif defined(ULTRA_CUDA_SOLVER_IMPORT)
	#define ULTRA_CUDA_SOLVER_API __declspec(dllimport)
	#define ULTRA_SOLVER_IMPORT 1
#else
	#define ULTRA_CUDA_SOLVER_API 
#endif

#include <solver/config.h>

#endif
