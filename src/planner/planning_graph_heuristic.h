
#ifndef UltraPlanner_planning_graph_heuristic_h
#define UltraPlanner_planning_graph_heuristic_h

#include <core/algorithm/graph.h>
#include <vector>
#include <numeric>

#define TRACE_HEURISTIC 0

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
	enum class computation_state_t {Normal, Tail, Failed, Finished};

	struct transition_ref
	{
		transition_t * transition;
		bool enabled;
	};

	using index_graph_t = explicit_graph<transition_ref*, char>;

public:
	planning_graph_heuristic(const transition_system_t & _system)
		:m_relaxedSystem(_system)
	{
		m_cache.costs.resize(_system.bool_part().size());
		m_cache.layer_costs.resize(_system.bool_part().size());
		m_relaxedSystem.to_relaxed();
		m_relaxedSystem.build_transitions_index();

		for (auto & tr : m_relaxedSystem.transitions())
			m_cache.all_transitions.push_back(&tr);

		//Build transition dependence graph
		cout << "Building transition dependence graph..." << std::endl;
		/*std::vector<var_ref*> var_refs(m_relaxedSystem.bool_part().size(), nullptr);

		using index_graph_t = explicit_graph<var_ref*, transition_t*>;
		index_graph_t transitionDependenceGraph;

		for (unsigned i = 0; i < m_relaxedSystem.bool_part().size(); ++i)
		{
			auto new_ref = new var_ref(i, m_relaxedSystem.bool_part().var_names()[i]);
			var_refs[i] = new_ref;

			transitionDependenceGraph.add_vertex(new_ref);
		}

		for (auto & tr : m_relaxedSystem.transitions())
		{
			for (unsigned cond_index : tr.bool_part.m_conditionVarIndices)
			{
				for (unsigned effect_index : tr.bool_part.m_affectedIndices)
				{
					transitionDependenceGraph.add_edge(var_refs[cond_index], var_refs[effect_index], &tr);
				}
			}
		}

		//Compress graph
		
		for (auto ref : var_refs)
		{
			auto & list = transitionDependenceGraph.adjacent_list(ref);
			std::vector<typename index_graph_t::adjacent_vertex_t> tmp(list.begin(), list.end());
			std::sort(tmp.begin(), tmp.end(), [](const typename index_graph_t::adjacent_vertex_t & lhs, const typename index_graph_t::adjacent_vertex_t & rhs){
				if (lhs.edge < rhs.edge)
					return true;
				else if (lhs.edge > rhs.edge)
					return false;
				else
					return lhs.vertex < rhs.vertex;
			});
			auto last_it = std::unique(tmp.begin(), tmp.end(), [](const typename index_graph_t::adjacent_vertex_t & lhs, const typename index_graph_t::adjacent_vertex_t & rhs){
				return (lhs.edge == rhs.edge) && (lhs.vertex == rhs.vertex);
			});
			list.clear();
			list.insert(list.end(), tmp.begin(), last_it);
		}*/

		//std::vector<std::vector<unsigned>> condition_dependence(m_relaxedSystem.bool_part().size()), effect_dependence(m_relaxedSystem.bool_part().size()); //condition_var index -> transition list

		m_transitionIndex.trans_refs.resize(m_relaxedSystem.transitions().size(), nullptr);

		for (unsigned i = 0; i < m_relaxedSystem.transitions().size(); ++i)
		{
			auto & tr = m_relaxedSystem.transitions()[i];
			auto new_ref = new transition_ref;
			new_ref->transition = &tr;
			m_transitionIndex.trans_refs[i] = new_ref;
			m_transitionIndex.transitionDependenceGraph.add_vertex(new_ref);

/*			for (unsigned cond_index : tr.bool_part.m_conditionVarIndices)
				condition_dependence[cond_index].push_back(i);

			for (unsigned effect_index : tr.bool_part.m_affectedIndices)
				effect_dependence[effect_index].push_back(i);
				*/
		}

		//Uniquize
/*		for (auto & vec : condition_dependence)
		{
			std::sort(vec.begin(), vec.end());
			auto last_it = std::unique(vec.begin(), vec.end());
			vec.erase(last_it, vec.end());
		}

		for (auto & vec : effect_dependence)
		{
			std::sort(vec.begin(), vec.end());
			auto last_it = std::unique(vec.begin(), vec.end());
			vec.erase(last_it, vec.end());
		}*/

		for (unsigned i = 0; i < m_relaxedSystem.transitions().size(); ++i)
		{
			auto & lhs = m_relaxedSystem.transitions()[i];
			for (unsigned j = 0; j < m_relaxedSystem.transitions().size(); ++j)
			{
				auto & rhs = m_relaxedSystem.transitions()[j];

				std::vector<int> res;
				std::set_intersection(lhs.bool_part.m_affectedIndices.begin(), lhs.bool_part.m_affectedIndices.end(), rhs.bool_part.m_conditionVarIndices.begin(), rhs.bool_part.m_conditionVarIndices.end(), std::back_inserter(res));
				if (!res.empty())
				{
					m_transitionIndex.transitionDependenceGraph.add_edge(m_transitionIndex.trans_refs[i], m_transitionIndex.trans_refs[j], 0);
				}
			}
		}
	}

	//Non-uniform version
	float operator()(const state_t & state) const
	{
		//for (auto & tr : m_relaxedSystem.transitions())
		//	tr.m_disabled = false;

		state_t current_state(state), last_state;
		int level_index = 0;

		std::fill(m_cache.costs.begin(), m_cache.costs.end(), std::numeric_limits<float>::max());
		current_state.bool_part.for_each_true([&](int idx){
			m_cache.costs[idx] = 0.0f;
		});

		m_cache.current_transitions = m_cache.all_transitions;
		auto last_tr_it = m_cache.current_transitions.end();

#if TRACE_HEURISTIC
		cout << "Computing PG value..." << std::endl;
#endif

		computation_state_t cstate = computation_state_t::Normal;
		int tail_size = 1;
		bool finished = false;
	
		//Mark refs as enabled
		/*for (auto ref : m_transitionIndex.trans_refs)
			ref->enabled = true;

		std::vector<transition_ref*> transition_buffer(m_relaxedSystem.transitions().size(), nullptr);

		//Get initial transition list
		auto last_it = transition_buffer.begin(), first_it = transition_buffer.begin();
		for (auto ref : m_transitionIndex.trans_refs)
		{
			if (m_relaxedSystem.transition_available(current_state, *(ref->transition)))
			{
				ref->enabled = false;
				*last_it++ = ref;
			}
		}*/

		
		do
		{
			//cout << "Transition count " << std::distance(first_it, last_it) << std::endl;

			last_state = current_state;
			m_cache.layer_costs = m_cache.costs;

			auto it = m_cache.current_transitions.begin();
			while (it != last_tr_it)
			{
				const transition_t & trans = **it;
				if (m_relaxedSystem.transition_available(last_state, trans))
				{
					float trans_cost = m_relaxedSystem.transition_cost(last_state, trans);

					float condition_cost = 0.0f;
					for (auto idx : trans.bool_part.m_conditionVarIndices)
						condition_cost = max(m_cache.costs[idx], condition_cost);

					//Find vars will be set with the current transition
					for (auto var_index : trans.bool_part.m_affectedIndices)
						m_cache.layer_costs[var_index] = min(m_cache.layer_costs[var_index], condition_cost + trans_cost);

					m_relaxedSystem.apply(current_state, trans);

					*it = *(--last_tr_it);
				}
				else
					++it;
			};


			/*auto prev_first = first_it;
			for (auto cur_last_it = last_it; first_it != cur_last_it; ++first_it)
			{
				const transition_t & trans = *((*first_it)->transition);
				float trans_cost = m_relaxedSystem.transition_cost(last_state, trans);

				float condition_cost = 0.0f;
				for (auto idx : trans.bool_part.m_conditionVarIndices)
					condition_cost = max(m_cache.costs[idx], condition_cost);

				//Find vars will be set with the current transition
				for (auto var_index : trans.bool_part.m_affectedIndices)
					m_cache.layer_costs[var_index] = min(m_cache.layer_costs[var_index], condition_cost + trans_cost);

				m_relaxedSystem.apply(current_state, trans);
			}

			for (auto it = prev_first; it != first_it; ++it)
			{
				m_transitionIndex.transitionDependenceGraph.forall_adj_verts(*it, [&](transition_ref * pref, float edge_len){
					if (pref->enabled && m_relaxedSystem.transition_available(current_state, *(pref->transition)))
					{
						pref->enabled = false;
						*last_it++ = pref;
					}
				});
			}*/

			switch (cstate)
			{
			case computation_state_t::Normal:
				if (m_relaxedSystem.is_solved(current_state.bool_part))
				{
					cstate = computation_state_t::Tail;
				}
				else if (last_state.bool_part == current_state.bool_part)
				{
					finished = true;
					cstate = computation_state_t::Failed;
				}
				break;
			case computation_state_t::Tail:
				if (--tail_size == 0)
					finished = true;
				break;
			}

			m_cache.costs = m_cache.layer_costs;
			++level_index;
		} while (!finished);

#if TRACE_HEURISTIC
		cout << "Total " << level_index << " layers, is_solved=" << m_relaxedSystem.is_solved(current_state) << std::endl;
#endif

		if (cstate == computation_state_t::Failed)
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
			assert(goal_cost >= 0.0f);
#endif
			return goal_cost;
		}
	}


private:
	mutable transition_system_t m_relaxedSystem;
	
	mutable struct{
		 std::vector<float> costs, layer_costs;
		 std::vector<const transition_t*> all_transitions;

		 std::vector<const transition_t*> current_transitions;
	} m_cache;

	struct
	{
		index_graph_t transitionDependenceGraph;
		std::vector<transition_ref*> trans_refs;
	} m_transitionIndex;
	
};

#endif
