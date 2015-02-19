
#include "transition_system_helpers.h"
#include <hannoi/hannoi_tower.h>
#include <core/transition_system.h>
#include "test_helpers.h"
#include <utility>

using namespace std;


void test_hannoi_tower_core()
{
	using hannoi_t = transition_system<hannoi_tower>;

	hannoi_t::size_description_t hannoi_size(4, 3);
	hannoi_t hannoi(hannoi_size);

	{
		std::vector<transition_system<hannoi_tower>::transition_t> correct_trans;
		correct_trans.push_back(make_pair(0, 1));
		correct_trans.push_back(make_pair(0, 2));
		bool r = test_available_transitions(hannoi, "1 * * \
													2 * * \
													3 * * \
													4 * *", correct_trans);

		assert_test(r, "Hannoi: iterating available transitions.");
	}

	{
		std::vector<transition_system<hannoi_tower>::transition_t> correct_trans;
		correct_trans.push_back(make_pair(1, 0));
		correct_trans.push_back(make_pair(1, 2));
		correct_trans.push_back(make_pair(2, 0));
		bool r = test_available_transitions(hannoi, "* * * \
													* * * \
													* 1 * \
													4 2 3", correct_trans);

		assert_test(r, "Hannoi: iterating available transitions.");
	}

	{
		std::vector<transition_system<hannoi_tower>::transition_t> correct_trans;
		correct_trans.push_back(make_pair(0, 1));
		correct_trans.push_back(make_pair(2, 1));
		correct_trans.push_back(make_pair(2, 0));
		bool r = test_available_transitions(hannoi, "* * * \
													* * * \
													2 * 1 \
													4 * 3", correct_trans);

		assert_test(r, "Hannoi: iterating available transitions.");
	}


	//Test applying transition
	{
		assert_test(test_apply_transition(hannoi, "* * * \
													* * * \
													* 1 * \
													4 2 3", 
													"* * * \
													* * * \
													1 * * \
													4 2 3", make_pair(1, 0)), "Hannoi: applying transition.");

		assert_test(test_apply_transition(hannoi, "* * * \
												  * * * \
												  3 1 * \
												  4 2 *",
												  "* * * \
												  * * * \
												  * 1 * \
												  4 2 3", make_pair(0, 2)), "Hannoi: applying transition.");
	}

	//Test hash
	{
		cout << "Testing Hannoi hash function..." << std::endl;

		auto sample_state = hannoi.default_state();
		std::hash<hannoi_t::state_t> hasher;
		cout << hasher(sample_state) << std::endl;

		hannoi.apply(sample_state, std::make_pair(2, 0));
		cout << hasher(sample_state) << std::endl;
	}

	//Test heuristic

}
