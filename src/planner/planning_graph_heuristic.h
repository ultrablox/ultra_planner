

#ifndef UltraPlanner_planning_graph_heuristic_h
#define UltraPlanner_planning_graph_heuristic_h

template<typename T>
class planning_graph_heuristic
{
public:
	using transition_system_t = T;
	using state_t = typename transition_system_t::state_t;
	using transition_t = typename transition_system_t::transition_t;
public:
	planning_graph_heuristic(const transition_system_t & _system)
		:m_relaxedSystem(_system)
	{
		m_relaxedSystem.to_relaxed();
	}

	float operator()(const state_t & state) const
	{
		state_t current_state(state);
		int level_index = 0;

		for (; !m_relaxedSystem.is_solved(current_state); ++level_index)
		{
			m_relaxedSystem.forall_available_transitions(current_state, [&](const transition_t & trans){
				m_relaxedSystem.apply(current_state, trans);
			});
		}

		return level_index;
	}
private:
	transition_system_t m_relaxedSystem;
};

#endif
