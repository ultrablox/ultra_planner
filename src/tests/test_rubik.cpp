
#include "helpers.h"
#include <rubik/rubiks_cube.h>
#include <core/transition_system/transition_system.h>

void test_rubik_core()
{
	typedef transition_system<rubiks_cube> rubik_t;
	rubik_t rubik(3);

//	auto correct_state = rubik.default_state();
}