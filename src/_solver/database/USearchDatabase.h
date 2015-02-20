

#ifndef UltraPlanner_USearchDatabase_h
#define UltraPlanner_USearchDatabase_h
/*
#include <unordered_set>
#include <windows.h>
#include <map>
#include <memory>*/
#include "USearchNodeFactory.h"
#include "UStateFactory.h"

typedef std::vector<std::pair<int, USearchNodeReference>> DetailedTransitionPath;

class USearchDatabase : public USearchNodeFactory, public UStateFactory
{
public:
	void clear();
	void dump(const std::string & file_name) const;
	int loadFromDump(const std::string & file_name);
	void setTransitionSystem(const UTransitionSystem & tr_sys);
	const UTransitionSystem & transitionSystem() const;
	
	/*
	Builds transition path between given nodes.
	*/
	//TransitionPath buildTransitionPath(const USearchNodeReference & first_node_ref, const USearchNodeReference & last_node_ref);

	/*
	Builds transition path starting from given node till the
	parentless node will be reached.
	*/
	//DetailedTransitionPath buildTransitionPath(const USearchNodeReference & first_node_ref);
	TransitionPath buildTransitionPath(const UStateReference & last_state_ref);
private:
	UTransitionSystem mTransitionSystem;
};

#endif
