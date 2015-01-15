
#ifndef UPDDL_UInvariantGenerator_h
#define UPDDL_UInvariantGenerator_h

#include "UBoolTask.h"
#include <vector>

namespace BoolTask
{
	struct UNormalizedBoolTask;
	struct UntypedAction;
};

struct InvariantCandidate
{
	int paramCount;
	std::vector<BoolTask::Formula> atoms;
};

class UInvariantGenerator
{
public:
	void generate(const BoolTask::UNormalizedBoolTask & norm_task);
private:
	bool prooveInvariant(const InvariantCandidate & ic, const std::vector<BoolTask::UntypedAction*> & operators);
	bool operatorTooHeavy(const InvariantCandidate & ic, const BoolTask::UntypedAction* op);
};

#endif
