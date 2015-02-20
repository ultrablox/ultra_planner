
#ifndef UltraPDDL_val_parser_h
#define UltraPDDL_val_parser_h

#include "config.h"
#include <string>

namespace VAL
{
	class analysis;
};

VAL::analysis* val_parse(const std::string & domain_file_name, const std::string & problem_file_name);

#endif
