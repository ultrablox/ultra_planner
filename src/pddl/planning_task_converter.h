
#ifndef UPDDL_planning_task_converter_h
#define UPDDL_planning_task_converter_h

#include "config.h"
#include <core/varset_system/combinedvar_system.h>

namespace VAL
{
	class analysis;
};

struct ULTRA_PDDL_API planning_task_t
{
	combinedvar_system varset_system;
	combinedvar_system::state_t initial_state;
	combinedvar_system::masked_state_t goal;

	void optimize();
	void optimize_const_bools();
	void optimize_const_floats();
};

class ULTRA_PDDL_API planning_task_converter
{
public:
	int get_planning_task(planning_task_t & planning_task, const std::string & domain_fname, const std::string & problem_fname) const;
	int from_pddl_task(planning_task_t & planning_task, VAL::analysis * pAnalysis) const;
	//static BoolTask::UBoolTask generateBoolTask(VAL::analysis * pAnalysis);
};

#endif
