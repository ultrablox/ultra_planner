
#include "test_helpers.h"
#include "transition_system_helpers.h"
#include <edfd_cover/edfd_cover.h>
#include <core/transition_system.h>
#include <sstream>

void test_edfd_cover()
{
	transition_system<edfd_cover> ts;

	edfd_cover_state state;
	state.src_graph = edfd_graph({}, {});
	test_available_transitions(ts, state, {});
}