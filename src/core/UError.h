
#ifndef UltraCore_UError_h
#define UltraCore_UError_h

#include "config.h"
#include <exception>

class ULTRA_CORE_API core_exception : public std::exception
{
public:
	core_exception(const char * msg);
};

#endif
