
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

		bool r = test_available_transitions(rubik, "0 0 0 0 0 0 0 0 0\
												   	1 1 1 1 1 1 1 1 1\
												   	2 2 2 2 2 2 2 2 2\
												   	3 3 3 3 3 3 3 3 3\
												   	4 4 4 4 4 4 4 4 4\
												   	5 5 5 5 5 5 5 5 5", correct_trans);

		assert_test(r, "Rubik: iterating available transitions.");
	}

	//Applying transitions...

	//F
	/*assert_test(test_apply_transition(rubik, "0 1 2 3 4 5 6 7 8 "
												"9 10 11 12 13 14 15 16 17 "
												"18 19 20 21 22 23 24 25 26 "
												"27 28 29 30 31 32 33 34 35 "
												"36 37 38 39 40 41 42 43 44 "
												"45 46 47 48 49 50 51 52 53 ",
												"0 1 45 3 4 46 6 7 47 "
												"15 12 9 16 13 10 17 14 11 "
												"42 19 20 43 22 23 44 25 26 "
												"27 28 29 30 31 32 33 34 35 "
												"36 37 38 39 40 41 8 5 2 "
												"24 21 18 48 49 50 51 52 53 ", rubik_t::transition_t::F), "Rubik: applying F transition.");*/
	assert_test(test_apply_transition(rubik, "0 0 0 0 0 0 0 0 0\
												1 1 1 1 1 1 1 1 1\
												2 2 2 2 2 2 2 2 2\
												3 3 3 3 3 3 3 3 3\
												4 4 4 4 4 4 4 4 4\
												5 5 5 5 5 5 5 5 5",
												"0 0 5 0 0 5 0 0 5\
												1 1 1 1 1 1 1 1 1\
												4 2 2 4 2 2 4 2 2\
												3 3 3 3 3 3 3 3 3\
												4 4 4 4 4 4 0 0 0\
												2 2 2 5 5 5 5 5 5", rubik_t::transition_t::F), "Rubik: applying F transition.");

	//B
	/*assert_test(test_apply_transition(rubik, "0 1 2 3 4 5 6 7 8 "
												"9 10 11 12 13 14 15 16 17 "
												"18 19 20 21 22 23 24 25 26 "
												"27 28 29 30 31 32 33 34 35 "
												"36 37 38 39 40 41 42 43 44 "
												"45 46 47 48 49 50 51 52 53 ",
												"38 1 2 37 4 5 36 7 8 "
												"9 10 11 12 13 14 15 16 17 "
												"18 19 53 21 22 52 24 25 51 "
												"33 30 27 34 31 28 35 32 29 "
												"20 23 26 39 40 41 42 43 44 "
												"45 46 47 48 49 50 0 3 6 ", rubik_t::transition_t::B), "Rubik: applying B transition.");*/
	assert_test(test_apply_transition(rubik, "0 0 0 0 0 0 0 0 0 "
												"1 1 1 1 1 1 1 1 1 "
												"2 2 2 2 2 2 2 2 2 "
												"3 3 3 3 3 3 3 3 3 "
												"4 4 4 4 4 4 4 4 4 "
												"5 5 5 5 5 5 5 5 5 ",
												"4 0 0 4 0 0 4 0 0 "
												"1 1 1 1 1 1 1 1 1 "
												"2 2 5 2 2 5 2 2 5 "
												"3 3 3 3 3 3 3 3 3 "
												"2 2 2 4 4 4 4 4 4 "
												"5 5 5 5 5 5 0 0 0 ", rubik_t::transition_t::B), "Rubik: applying B transition.");

	//U
	/*assert_test(test_apply_transition(rubik, "0 1 2 3 4 5 6 7 8 "
												"9 10 11 12 13 14 15 16 17 "
												"18 19 20 21 22 23 24 25 26 "
												"27 28 29 30 31 32 33 34 35 "
												"36 37 38 39 40 41 42 43 44 "
												"45 46 47 48 49 50 51 52 53 ",
												"9 10 11 3 4 5 6 7 8 "
												"18 19 20 12 13 14 15 16 17 "
												"27 28 29 21 22 23 24 25 26 "
												"0 1 2 30 31 32 33 34 35 "
												"42 39 36 43 40 37 44 41 38 "
												"45 46 47 48 49 50 51 52 53 ", rubik_t::transition_t::U), "Rubik: applying U transition.");*/

	assert_test(test_apply_transition(rubik, "0 0 0 0 0 0 0 0 0 "
												"1 1 1 1 1 1 1 1 1 "
												"2 2 2 2 2 2 2 2 2 "
												"3 3 3 3 3 3 3 3 3 "
												"4 4 4 4 4 4 4 4 4 "
												"5 5 5 5 5 5 5 5 5 ",
												"1 1 1 0 0 0 0 0 0 "
												"2 2 2 1 1 1 1 1 1 "
												"3 3 3 2 2 2 2 2 2 "
												"0 0 0 3 3 3 3 3 3 "
												"4 4 4 4 4 4 4 4 4 "
												"5 5 5 5 5 5 5 5 5 ", rubik_t::transition_t::U), "Rubik: applying U transition.");

	//D
	/*assert_test(test_apply_transition(rubik, "0 1 2 3 4 5 6 7 8 "
											"9 10 11 12 13 14 15 16 17 "
											"18 19 20 21 22 23 24 25 26 "
											"27 28 29 30 31 32 33 34 35 "
											"36 37 38 39 40 41 42 43 44 "
											"45 46 47 48 49 50 51 52 53 ",
											"0 1 2 3 4 5 33 34 35 "
											"9 10 11 12 13 14 6 7 8 "
											"18 19 20 21 22 23 15 16 17 "
											"27 28 29 30 31 32 24 25 26 "
											"36 37 38 39 40 41 42 43 44 "
											"51 48 45 52 49 46 53 50 47 ", rubik_t::transition_t::D), "Rubik: applying D transition.");*/

	assert_test(test_apply_transition(rubik, "0 0 0 0 0 0 0 0 0 "
												"1 1 1 1 1 1 1 1 1 "
												"2 2 2 2 2 2 2 2 2 "
												"3 3 3 3 3 3 3 3 3 "
												"4 4 4 4 4 4 4 4 4 "
												"5 5 5 5 5 5 5 5 5 ",
												"0 0 0 0 0 0 3 3 3 "
												"1 1 1 1 1 1 0 0 0 "
												"2 2 2 2 2 2 1 1 1 "
												"3 3 3 3 3 3 2 2 2 "
												"4 4 4 4 4 4 4 4 4 "
												"5 5 5 5 5 5 5 5 5 ", rubik_t::transition_t::D), "Rubik: applying D transition.");
	
	//L
	/*assert_test(test_apply_transition(rubik, "0 1 2 3 4 5 6 7 8 "
											"9 10 11 12 13 14 15 16 17 "
											"18 19 20 21 22 23 24 25 26 "
											"27 28 29 30 31 32 33 34 35 "
											"36 37 38 39 40 41 42 43 44 "
											"45 46 47 48 49 50 51 52 53 ",
											"6 3 0 7 4 1 8 5 2 "
											"36 10 11 39 13 14 42 16 17 "
											"18 19 20 21 22 23 24 25 26 "
											"27 28 51 30 31 48 33 34 45 "
											"35 37 38 32 40 41 29 43 44 "
											"9 46 47 12 49 50 15 52 53 ", rubik_t::transition_t::L), "Rubik: applying L transition.");*/

	assert_test(test_apply_transition(rubik, "0 0 0 0 0 0 0 0 0 "
												"1 1 1 1 1 1 1 1 1 "
												"2 2 2 2 2 2 2 2 2 "
												"3 3 3 3 3 3 3 3 3 "
												"4 4 4 4 4 4 4 4 4 "
												"5 5 5 5 5 5 5 5 5 ",
												"0 0 0 0 0 0 0 0 0 "
												"4 1 1 4 1 1 4 1 1 "
												"2 2 2 2 2 2 2 2 2 "
												"3 3 5 3 3 5 3 3 5 "
												"3 4 4 3 4 4 3 4 4 "
												"1 5 5 1 5 5 1 5 5 ", rubik_t::transition_t::L), "Rubik: applying L transition.");

	//R
	/*assert_test(test_apply_transition(rubik, "0 1 2 3 4 5 6 7 8 "
											"9 10 11 12 13 14 15 16 17 "
											"18 19 20 21 22 23 24 25 26 "
											"27 28 29 30 31 32 33 34 35 "
											"36 37 38 39 40 41 42 43 44 "
											"45 46 47 48 49 50 51 52 53 ",
											"0 1 2 3 4 5 6 7 8 "
											"9 10 47 12 13 50 15 16 53 "
											"24 21 18 25 22 19 26 23 20 "
											"44 28 29 41 31 32 38 34 35 "
											"36 37 11 39 40 14 42 43 17 "
											"45 46 33 48 49 30 51 52 27 ", rubik_t::transition_t::R), "Rubik: applying R transition.");*/

	assert_test(test_apply_transition(rubik, "0 0 0 0 0 0 0 0 0 "
												"1 1 1 1 1 1 1 1 1 "
												"2 2 2 2 2 2 2 2 2 "
												"3 3 3 3 3 3 3 3 3 "
												"4 4 4 4 4 4 4 4 4 "
												"5 5 5 5 5 5 5 5 5 ",
												"0 0 0 0 0 0 0 0 0 "
												"1 1 5 1 1 5 1 1 5 "
												"2 2 2 2 2 2 2 2 2 "
												"4 3 3 4 3 3 4 3 3 "
												"4 4 1 4 4 1 4 4 1 "
												"5 5 3 5 5 3 5 5 3 ", rubik_t::transition_t::R), "Rubik: applying R transition.");

	//Test heuristic

	manhattan_3D_distance_heuristic<rubik_t> heuristic(rubik);

	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "0 0 0 0 0 0 0 0 0 "
				"1 1 1 1 1 1 1 1 1 "
				"2 2 2 2 2 2 2 2 2 "
				"3 3 3 3 3 3 3 3 3 "
				"4 4 4 4 4 4 4 4 4 "
				"5 5 5 5 5 5 5 5 5 ";
		rubik.deserialize_state(ss, state);

		assert_test(heuristic(state) == 0.0f, "Rubik: manhattan3d solved state.");
	}

	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "0 0 0 0 0 0 0 0 0 "
			"1 1 5 1 1 5 1 1 5 "
			"2 2 2 2 2 2 2 2 2 "
			"4 3 3 4 3 3 4 3 3 "
			"4 4 1 4 4 1 4 4 1 "
			"5 5 3 5 5 3 5 5 3 ";
		rubik.deserialize_state(ss, state);

		assert_test(heuristic(state) == 1.0f, "Rubik: manhattan3d 1-distance state.");
	}

	//Test hash
	size_t solved_hash, another_hash;
	std::hash<rubik_t::state_t> hasher;
	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "0 0 0 0 0 0 0 0 0 "
				"1 1 1 1 1 1 1 1 1 "
				"2 2 2 2 2 2 2 2 2 "
				"3 3 3 3 3 3 3 3 3 "
				"4 4 4 4 4 4 4 4 4 "
				"5 5 5 5 5 5 5 5 5 ";
		rubik.deserialize_state(ss, state);

		solved_hash = hasher(state);
	}

	{
		rubik_t::state_t state(3);

		std::stringstream ss;
		ss << "0 0 0 0 0 0 0 0 0 "
				"1 1 5 1 1 5 1 1 5 "
				"2 2 2 2 2 2 2 2 2 "
				"4 3 3 4 3 3 4 3 3 "
				"4 4 1 4 4 1 4 4 1 "
				"5 5 3 5 5 3 5 5 3 ";
		rubik.deserialize_state(ss, state);

		another_hash = hasher(state);
	}

	assert_test(solved_hash != another_hash, "Rubik: hash is not good distributed.");
}