
#ifndef UltraPlanner_UPDDLNamedObject_h
#define UltraPlanner_UPDDLNamedObject_h

#include "config.h"
#include <string>

struct ULTRA_PDDL_API UPDDLNamedObject
{
    UPDDLNamedObject(const std::string & name);
    
    std::string name;
};


#endif
