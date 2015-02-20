
#ifndef UltraSolver_config_h
#define UltraSolver_config_h

#ifdef ULTRA_SOLVER_EXPORT
	#define ULTRA_SOLVER_API __declspec(dllexport)
    #define ULTRA_CORE_IMPORT 1
#elif defined(ULTRA_SOLVER_IMPORT)
	#define ULTRA_SOLVER_API __declspec(dllimport)
    #define ULTRA_CORE_IMPORT 1
#else
	#define ULTRA_SOLVER_API 
#endif

#include <core/config.h>

typedef size_t StateAddress;

#endif
