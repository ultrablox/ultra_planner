
#include <hannoi/hannoi_tower.h>
#include <core/transition_system/transition_system.h>
#include <utility>
#include <sstream>

using namespace std;

void test_available_transitions(const std::string & state_data, transition_system<hannoi_tower> & htower, const std::vector<transition_system<hannoi_tower>::transition_t> & correct_res)
{
	hannoi_tower::state_t state;
	
	std::stringstream ss;
	ss << state_data;

	htower.deserialize_state(ss, state);

	/*std::vector<transition_system<sliding_puzzle>::transition_t> res;

	puzzle.forall_available_transitions(state, [&](transition_system<sliding_puzzle>::transition_t plate_index){
		res.push_back(plate_index);
	});

	std::sort(res.begin(), res.end());

	assert_test(res == correct_res, "Iterating available transitions.");*/
}

void test_hannoi_tower_core()
{
	using hannoi_t = transition_system<hannoi_tower>;

	auto hannoi_size = make_pair(4, 3);
	hannoi_t hannoi(hannoi_size);

	std::vector<transition_system<hannoi_tower>::transition_t> correct_trans;

	correct_trans.push_back(make_pair(0, 1));
	correct_trans.push_back(make_pair(0, 2));
	test_available_transitions("* 1 2 3 * *", hannoi, correct_trans);
}
