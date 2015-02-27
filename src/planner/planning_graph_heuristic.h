
#ifndef UltraPlanner_planning_graph_heuristic_h
#define UltraPlanner_planning_graph_heuristic_h

#include <vector>
#include <numeric>

class planning_graph
{

};

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
		m_relaxedSystem.build_transitions_index();
	}

	//Non-uniform version
	float operator()(const state_t & state) const
	{
		for (auto & tr : m_relaxedSystem.transitions())
			tr.m_disabled = false;

		state_t current_state(state), last_state;
		int level_index = 0;

		std::fill(m_cache.costs.begin(), m_cache.costs.end(), std::numeric_limits<float>::max());
		current_state.bool_part.for_each_true([&](int idx){
			m_cache.costs[idx] = 0.0f;
		});

		do
		{
			last_state = current_state;
			m_cache.layer_costs = m_cache.costs;
			m_relaxedSystem.forall_available_transitions(last_state, [&](const transition_t & trans){
				trans.m_disabled = true;

				float trans_cost = m_relaxedSystem.transition_cost(last_state, trans);
				
				float condition_cost = 0.0f;
				for (auto idx : trans.bool_part.m_conditionVarIndices)
					condition_cost = max(m_cache.costs[idx], condition_cost);

				//Find vars will be set with the current transition
				for (auto var_index : trans.bool_part.m_affectedIndices)
					m_cache.layer_costs[var_index] = min(m_cache.layer_costs[var_index], condition_cost + trans_cost);

				m_relaxedSystem.apply(current_state, trans);
			});

			m_cache.costs = m_cache.layer_costs;
			++level_index;
		} while (!(last_state.bool_part == current_state.bool_part)/* && (!m_relaxedSystem.is_solved(current_state))*/);

		if (!m_relaxedSystem.is_solved(current_state))
			return std::numeric_limits<float>::max();
		else
		{
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
	}
private:
	mutable transition_system_t m_relaxedSystem;
	
	mutable struct{
		 std::vector<float> costs, layer_costs;
	} m_cache;
	
};

#endif
