
#ifndef UPDDL_UTaskGenerator_h
#define UPDDL_UTaskGenerator_h

#include <tuple>
#include <core/transition_system/UTransitionSystem.h>
#include <core/transition_system/UState.h>
#include "UBoolTask.h"


namespace VAL
{
	class analysis;
};

typedef std::tuple<UTransitionSystem, UState, UPartialState, bool> UPlanningTask;

class UTaskGenerator
{
public:
	static UPlanningTask generateTask(VAL::analysis * pAnalysis);
	static BoolTask::UBoolTask generateBoolTask(VAL::analysis * pAnalysis);
};

#endif
