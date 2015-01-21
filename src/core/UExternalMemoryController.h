
#ifndef UltraSolver_UExternalMemoryController_h
#define UltraSolver_UExternalMemoryController_h

#include "config.h"
#include <string>

class ULTRA_CORE_API UExternalMemoryController
{
public:
	UExternalMemoryController(const std::string & ignored_drives = "C", const std::string & subdir = "ultra_planner");
};

#endif
