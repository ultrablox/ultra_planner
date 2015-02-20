
#ifndef UltraTest_transition_system_helpers_h
#define UltraTest_transition_system_helpers_h

#include <sstream>
#include <algorithm>
#include <vector>

template<typename TrSys>
typename TrSys::state_t deserialize_state(const TrSys & tr_system, const std::string & state_data)
{
	typename TrSys::state_t state(tr_system.size());
	std::stringstream ss;
	ss << state_data;

	tr_system.deserialize_state(ss, state);

	return std::move(state);
}


template<typename TrSys>
bool test_available_transitions(const TrSys & tr_system, const typename TrSys::state_t & state, std::vector<typename TrSys::transition_t> correct_res)
{
	std::vector<typename TrSys::transition_t> res;

	tr_system.forall_available_transitions(state, [&](typename TrSys::transition_t transition){
		res.push_back(transition);
	});

	std::sort(correct_res.begin(), correct_res.end());
	std::sort(res.begin(), res.end());

	return correct_res == res;
}

template<typename TrSys>
bool test_available_transitions(const TrSys & tr_system, const std::string & state_data, const std::vector<typename TrSys::transition_t> & correct_res)
{
	typename TrSys::state_t state(tr_system.size());

	std::stringstream ss;
	ss << state_data;

	tr_system.deserialize_state(ss, state);

	return test_available_transitions(tr_system, state, correct_res);
}

template<typename TrSys>
bool test_apply_transition(const TrSys & tr_system, typename TrSys::state_t init_state, const typename TrSys::state_t & final_state, const typename TrSys::transition_t & transition)
{
	tr_system.apply(init_state, transition);
	return init_state == final_state;
}

template<typename TrSys>
bool test_apply_transition(const TrSys & tr_system, const std::string & init_state_data, const std::string & final_state_data, const typename TrSys::transition_t & transition)
{

	typename TrSys::state_t initial_state(tr_system.size()), final_state(tr_system.size());

	{
		std::stringstream ss;
		ss << init_state_data;
		tr_system.deserialize_state(ss, initial_state);
	}

	{
		std::stringstream ss;
		ss << final_state_data;
		tr_system.deserialize_state(ss, final_state);
	}

	return test_apply_transition(tr_system, initial_state, final_state, transition);	
}

#endif
