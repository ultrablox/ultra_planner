
#ifndef UltraMultiCoreSolver_config_h
#define UltraMultiCoreSolver_config_h

#ifdef ULTRA_MCSOLVER_EXPORT
	#define ULTRA_MCSOLVER_API __declspec(dllexport)
	#define ULTRA_SOLVER_IMPORT 1
#elif defined(ULTRA_MCSOLVER_IMPORT)
	#define ULTRA_MCSOLVER_API __declspec(dllimport)
	#define ULTRA_SOLVER_IMPORT 1
#else
	#define ULTRA_MCSOLVER_API 
#endif

#include <solver/config.h>

#endif
