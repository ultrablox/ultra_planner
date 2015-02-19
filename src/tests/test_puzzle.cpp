
#include "test_helpers.h"
#include "transition_system_helpers.h"
#include <sliding_puzzle/sliding_puzzle.h>
#include <core/transition_system.h>
#include <sstream>

/*
void test_available_transitions(const std::string & state_data, transition_system<sliding_puzzle> & puzzle, const std::vector<transition_system<sliding_puzzle>::transition_t> & correct_res)
{
	auto state = puzzle.default_state();
	std::stringstream ss;
	ss << state_data;

	puzzle.deserialize_state(ss, state);

	std::vector<transition_system<sliding_puzzle>::transition_t> res;

	puzzle.forall_available_transitions(state, [&](transition_system<sliding_puzzle>::transition_t plate_index){
		res.push_back(plate_index);
	});

	std::sort(res.begin(), res.end());

	assert_test(res == correct_res, "Iterating available transitions.");
}*/


void test_puzzle_core()
{
	typedef transition_system<sliding_puzzle> puzzle_t;

	
	auto puzzle_size = make_pair(3, 4);
	puzzle_t puzzle(puzzle_size);

	
	//Test available transitions

	{

		/*
		7 2 4
		5 3 6
		8 1 0
		9 10 11

		available transitions: 5('6'),7('1'),11('11')
		*/
		std::vector<puzzle_t::transition_t> correct_res;

		correct_res.push_back(5);
		correct_res.push_back(7);
		correct_res.push_back(11);

		assert_test(test_available_transitions(puzzle, "7 2 4 5 3 6 8 1 0 9 10 11 ", correct_res), "Puzzle: iterating available transitions.");
	}

	{
		/*
		State:
		7 2 4
		5 0 6
		8 3 1
		9 10 11

		available transitions: 1('2'),3('5'),5('6'),7('3')
		*/
		std::vector<puzzle_t::transition_t> correct_res;

		correct_res.push_back(1);
		correct_res.push_back(3);
		correct_res.push_back(5);
		correct_res.push_back(7);

		assert_test(test_available_transitions(puzzle, "7 2 4 5 0 6 8 3 1 9 10 11 ", correct_res), "Puzzle: iterating available transitions.");
	}


	//Test applying transition
	{
		/*
			7 2 4			7 2 4
			5 0 6  + '3' -> 5 3 6
			8 3 1			8 0 1
			9 10 11			9 10 11
			*/

		assert_test(test_apply_transition(puzzle, "7 2 4 5 3 6 8 1 0 9 10 11 ", "7 2 4  5 3 6  8 0 1  9 10 11 ", 7), "Puzzle: applying transition.");
		/*auto state = puzzle_t::default_state(puzzle_size);
		{
		std::stringstream ss;
		ss << "7 2 4 5 3 6 8 1 0 9 10 11 " ;
		puzzle.deserialize_state(ss, state);
		}

		auto correct_state = puzzle_t::default_state(puzzle_size);

		{
		std::stringstream ss;
		ss << "7 2 4 " << "5 3 6 " << "8 0 1 " <<  "9 10 11 " ;
		puzzle.deserialize_state(ss, correct_state);
		}


		puzzle.apply(state, 7);

		assert_test(state == correct_state, "Applying transition.");*/
	}


	//Streamer
	{
		puzzle_t::state_streamer_t streamer(puzzle);
		std::vector<unsigned char> buffer(streamer.serialized_size());

		auto sample_state = deserialize_state(puzzle, "7 2 4  5 3 6  8 0 1  9 10 11 ");

		streamer.serialize(buffer.data(), sample_state);

		puzzle_t::state_t deserialized_state;
		streamer.deserialize(buffer.data(), deserialized_state);
		assert_test(sample_state == deserialized_state, "Puzzle Streamer: serializing/deserializing.");
	}

}
