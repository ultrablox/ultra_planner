
#ifndef UltraSolver_goal_count_heuristic_h
#define UltraSolver_goal_count_heuristic_h

#include <core/varset_system/boolvar_system.h>

template<typename T>
class goal_count_heuristic
{
	using transition_system_t = T;
	using state_t = typename transition_system_t::state_t;
public:
	goal_count_heuristic(const transition_system_t & _system)
		:m_system(_system)
	{
	}

	float operator()(const state_t & state) const
	{
		return m_system.goalState().bool_part.mask.trueCount() - m_system.goalState().bool_part.equalCount(state.bool_part);
		/*int res = 0;
		for (int i = 0; i < state.bool_part.bitCount(); ++i)
		{
			if (m_system.goalState().bool_part.mask[i])
			{
				if (m_system.goalState().bool_part.value[i] == state.bool_part[i])
					++res;
			}
		}
		return res;*/
	}

	const transition_system_t & m_system;
};

#endif
