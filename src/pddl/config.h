
#ifndef UltraPDDL_config_h
#define UltraPDDL_config_h

#ifdef ULTRA_PDDL_EXPORT
	#define ULTRA_PDDL_API __declspec(dllexport)
    #define ULTRA_CORE_IMPORT 1
#elif defined(ULTRA_PDDL_IMPORT)
	#define ULTRA_PDDL_API __declspec(dllimport)
    #define ULTRA_CORE_IMPORT 1
#else
	#define ULTRA_PDDL_API 
#endif

#include <core/config.h>

#endif
