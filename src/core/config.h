
#ifndef UltraCore_config_h
#define UltraCore_config_h

#ifdef ULTRA_CORE_EXPORT
	#define ULTRA_CORE_API __declspec(dllexport)
#elif defined(ULTRA_CORE_IMPORT)
	#define ULTRA_CORE_API __declspec(dllimport)
#else
	#define ULTRA_CORE_API 
#endif

#define USE_HDD_STORAGE 1

#define USE_INTRINSIC 0

#endif
