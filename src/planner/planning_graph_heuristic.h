
#ifndef UltraPlanner_planning_graph_heuristic_h
#define UltraPlanner_planning_graph_heuristic_h

#include <vector>
#include <numeric>

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
		m_cache.costs.resize(_system.bool_part().size());
		m_cache.layer_costs.resize(_system.bool_part().size());
		m_relaxedSystem.to_relaxed();
	}

	float operator()(const state_t & state) const
	{
		state_t current_state(state), last_state;
		int level_index = 0;

		std::fill(m_cache.costs.begin(), m_cache.costs.end(), std::numeric_limits<float>::max());
		current_state.bool_part.for_each_true([&](int idx){
			m_cache.costs[idx] = 0.0f;
		});

		for (; !m_relaxedSystem.is_solved(current_state); ++level_index)
		{
			std::fill(m_cache.layer_costs.begin(), m_cache.layer_costs.end(), std::numeric_limits<float>::max());

			last_state = current_state;
			m_cache.layer_costs = m_cache.costs;
			m_relaxedSystem.forall_available_transitions(last_state, [&](const transition_t & trans){
				float trans_cost = m_relaxedSystem.transition_cost(last_state, trans);
				
				float condition_cost = 0.0f;
				trans.bool_part.condition.mask.for_each_true([&](int idx){
					condition_cost = max(m_cache.costs[idx], condition_cost);
				});

				//Find vars will be set with the current transition
				trans.bool_part.effect.value.for_each_true([&](int var_index){
					//if (!current_state.bool_part[var_index])
					m_cache.layer_costs[var_index] = min(m_cache.layer_costs[var_index], condition_cost + trans_cost);
				});

				m_relaxedSystem.apply(current_state, trans);
			});

			if (last_state == current_state)
				return std::numeric_limits<float>::max();

			m_cache.costs = m_cache.layer_costs;
		}

		float goal_cost = 0.0f;
		m_relaxedSystem.goal_state().bool_part.mask.for_each_true([&](int idx){
			goal_cost = max(goal_cost, m_cache.costs[idx]);
		});
		//cout << level_index << std::endl;
		//return *std::max_element(m_cache.costs.begin(), m_cache.costs.end(), );
		//return std::accumulate(m_cache.costs.begin(), m_cache.costs.end(), 0.0f);
#if _DEBUG
		if (goal_cost < 0.0f)
			throw runtime_error("Impossible");
#endif
		return goal_cost;
	}
private:
	transition_system_t m_relaxedSystem;
	
	mutable struct{
		 std::vector<float> costs, layer_costs;
	} m_cache;
	
};

#endif
