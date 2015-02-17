
#include "test_helpers.h"
#include <rubik/rubiks_cube.h>
#include <rubik/heuristic.h>
#include "transition_system_helpers.h"
#include <core/transition_system/transition_system.h>

void test_rubik_core()
{
	using rubik_t = transition_system<rubiks_cube>;
	rubik_t rubik(3);

	{
		std::vector<rubik_t::transition_t> correct_trans;

		for (int i = 0; i < static_cast<int>(rubik_t::transition_t::count); ++i)
			correct_trans.push_back(static_cast<rubik_t::transition_t>(i));

		bool r = test_available_transitions(rubik, "		B B B "
													"		B B B "
													"		W W W "
													"W W G  R R R  B Y Y  O O O "
													"W W G  R R R  B Y Y  O O O "
													"W W G  R R R  B Y Y  O O O "
													"		Y Y Y "
													"		G G G "
													"		G G G ", correct_trans);

		assert_test(r, "Rubik: iterating available transitions.");
	}

	/*{
		auto state = rubik.solved_state();
		rubik.serialize_state(cout, state);

		rubik.apply(state, rubik_t::transition_t::R);
		rubik.apply(state, rubik_t::transition_t::R);
		rubik.apply(state, rubik_t::transition_t::R);
		//rubik.apply(state, rubik_t::transition_t::R);
		rubik.serialize_state(cout, state);
	}

	{
		auto state = rubik.solved_state();
		rubik.serialize_state(cout, state);

		rubik.apply(state, rubik_t::transition_t::R);
		rubik.serialize_state(cout, state);
	}

	{
		auto state = rubik.solved_state();
		rubik.serialize_state(cout, state);

		rubik.apply(state, rubik_t::transition_t::F);
		rubik.serialize_state(cout, state);

		rubik.apply(state, rubik_t::transition_t::R);
		rubik.serialize_state(cout, state);
	}*/

	//Applying transitions...

	//F
	assert_test(test_apply_transition(rubik, "		B B B "
											"		B B B "
											"		B B B "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"		G G G "
											"		G G G "
											"		G G G ",
											"		B B B "
											"		B B B "
											"		W W W "
											"W W G  R R R  B Y Y  O O O "
											"W W G  R R R  B Y Y  O O O "
											"W W G  R R R  B Y Y  O O O "
											"		Y Y Y "
											"		G G G "
											"		G G G ", rubik_t::transition_t::F), "Rubik: applying F transition.");

	//B
	assert_test(test_apply_transition(rubik, "		B B B "
											"		B B B "
											"		B B B "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"		G G G "
											"		G G G "
											"		G G G ",
											"		Y Y Y "
											"		B B B "
											"		B B B "
											"B W W  R R R  Y Y G  O O O "
											"B W W  R R R  Y Y G  O O O "
											"B W W  R R R  Y Y G  O O O "
											"		G G G "
											"		G G G "
											"		W W W ", rubik_t::transition_t::B), "Rubik: applying B transition.");

	//U
	assert_test(test_apply_transition(rubik, "		B B B "
											"		B B B "
											"		B B B "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"		G G G "
											"		G G G "
											"		G G G ",
											"		B B B "
											"		B B B "
											"		B B B "
											"R R R  Y Y Y  O O O  W W W "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"		G G G "
											"		G G G "
											"		G G G ", rubik_t::transition_t::U), "Rubik: applying U transition.");

	//D
	assert_test(test_apply_transition(rubik, "		B B B "
											"		B B B "
											"		B B B "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"		G G G "
											"		G G G "
											"		G G G ",
											"		B B B "
											"		B B B "
											"		B B B "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"O O O  W W W  R R R  Y Y Y "
											"		G G G "
											"		G G G "
											"		G G G ", rubik_t::transition_t::D), "Rubik: applying D transition.");

	//L
	assert_test(test_apply_transition(rubik, "		B B B "
											"		B B B "
											"		B B B "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"		G G G "
											"		G G G "
											"		G G G ",
											"		O B B "
											"		O B B "
											"		O B B "
											"W W W  B R R  Y Y Y  O O G "
											"W W W  B R R  Y Y Y  O O G "
											"W W W  B R R  Y Y Y  O O G "
											"		R G G "
											"		R G G "
											"		R G G ", rubik_t::transition_t::L), "Rubik: applying L transition.");

	//R
	assert_test(test_apply_transition(rubik, "		B B B "
											"		B B B "
											"		B B B "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"W W W  R R R  Y Y Y  O O O "
											"		G G G "
											"		G G G "
											"		G G G ",
											"		B B R "
											"		B B R "
											"		B B R "
											"W W W  R R G  Y Y Y  B O O "
											"W W W  R R G  Y Y Y  B O O "
											"W W W  R R G  Y Y Y  B O O "
											"		G G O "
											"		G G O "
											"		G G O ", rubik_t::transition_t::R), "Rubik: applying R transition.");

	//Test heuristic

	manhattan_3D_distance_heuristic<rubik_t> heuristic(rubik);

	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "		B B B "
			"		B B B "
			"		B B B "
			"W W W  R R R  Y Y Y  O O O "
			"W W W  R R R  Y Y Y  O O O "
			"W W W  R R R  Y Y Y  O O O "
			"		G G G "
			"		G G G "
			"		G G G ";
		rubik.deserialize_state(ss, state);

		assert_test(heuristic(state) == 0.0f, "Rubik: manhattan3d solved state.");
	}

	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "		B B R "
			"		B B R "
			"		B B R "
			"W W W  R R G  Y Y Y  B O O "
			"W W W  R R G  Y Y Y  B O O "
			"W W W  R R G  Y Y Y  B O O "
			"		G G O "
			"		G G O "
			"		G G O ";
		rubik.deserialize_state(ss, state);

		assert_test(heuristic(state) == 1.0f, "Rubik: manhattan3d 1-distance state.");
	}

	//Test hash
	size_t solved_hash, another_hash;
	std::hash<rubik_t::state_t> hasher;
	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "		B B B "
			"		B B B "
			"		B B B "
			"W W W  R R R  Y Y Y  O O O "
			"W W W  R R R  Y Y Y  O O O "
			"W W W  R R R  Y Y Y  O O O "
			"		G G G "
			"		G G G "
			"		G G G ";
		rubik.deserialize_state(ss, state);

		solved_hash = hasher(state);
	}

	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "		B B R "
			"		B B R "
			"		B B R "
			"W W W  R R G  Y Y Y  B O O "
			"W W W  R R G  Y Y Y  B O O "
			"W W W  R R G  Y Y Y  B O O "
			"		G G O "
			"		G G O "
			"		G G O ";
		rubik.deserialize_state(ss, state);

		another_hash = hasher(state);
	}

	assert_test(solved_hash != another_hash, "Rubik: hash is not good distributed.");
}