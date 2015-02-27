
#ifndef UltraCore_floatvar_system_h
#define UltraCore_floatvar_system_h

#include "varset_system_base.h"
#include "../numeric_expression.h"
#include "../bit_container.h"
#include <vector>
#include <map>

struct floatvar_transition_base
{
	enum class effect_type_t {NoEffect, Assign, ScaleUp, ScaleDown, Increase, Decrease};

	struct numeric_effect_t
	{
		effect_type_t type;
		numeric_expression * expr;

		friend bool operator==(const numeric_effect_t & lhs, const numeric_effect_t & rhs)
		{
			return (lhs.type == rhs.type) && (lhs.expr == rhs.expr);
		}
	};

	using initialize_element = std::tuple<int, effect_type_t, float>;
	floatvar_transition_base(int size = 0, const std::vector<initialize_element> non_trivial_effects = {})
	{
		for (auto & init_el : non_trivial_effects)
		{
			auto & eff = effect[get<0>(init_el)];
			eff.type = get<1>(init_el);
			eff.expr = numeric_expression::simpleValue(get<2>(init_el));
			//eff.value = get<2>(init_el);
		}
	}

	friend bool operator==(const floatvar_transition_base & lhs, const floatvar_transition_base & rhs)
	{
		return lhs.effect == rhs.effect;
	}

	void set_effect(int index, const effect_type_t _type, numeric_expression * expr)
	{
		numeric_effect_t num_eff;
		num_eff.type = _type;
		num_eff.expr = expr;
		effect.insert(make_pair(index, std::move(num_eff)));
	}

	void replace_with_const(int var_index, float const_val)
	{
		for (auto & eff : effect)
			eff.second.expr->replace_with_const(var_index, const_val);
	}

	template<typename M>
	void remap_vars(M & mapping)
	{
		std::map<int, numeric_effect_t> new_effects;
		for (auto & el : effect)
			new_effects.insert(make_pair(mapping[el.first], el.second));

		effect = new_effects;
	}

	//VarIndex -> effect
	std::map<int, numeric_effect_t> effect;
};

class floatvar_system_base
{
public:
	using state_t = std::vector<float>;
	using transition_t = varset_transition_base<floatvar_transition_base>;
	struct masked_state_t
	{
		bit_vector mask;
		state_t value;
	};

	class state_streamer_t : public streamer_base
	{
	public:
		state_streamer_t(const floatvar_system_base & _fsystem)
			:streamer_base(_fsystem.size() * sizeof(float)), m_size(_fsystem.size())
		{}

		void serialize(void * dst, const state_t & state) const
		{
			memcpy(dst, state.data(), m_size * sizeof(float));
		}

		void deserialize(const void * src, state_t & state) const
		{
			state.resize(m_size);
			memcpy(state.data(), src, m_size * sizeof(float));
		}

	private:
		int m_size;
	};

	floatvar_system_base(int var_count)
		:m_size(var_count), m_varNames(var_count, "unnamed_float")
	{}
		
	bool transition_available(const state_t & state, const floatvar_transition_base & transition) const
	{
		return true;
	}

	void apply(state_t & state, const floatvar_transition_base & transition) const
	{
		for (auto & eff : transition.effect)
		{
			auto & num_eff = eff.second;
			float exp_val = num_eff.expr->evaluate(state);
			auto & var_val = state[eff.first];

			switch (num_eff.type)
			{
			case floatvar_transition_base::effect_type_t::Assign:
				var_val = exp_val;
				break;
			case floatvar_transition_base::effect_type_t::Decrease:
				var_val -= exp_val;
				break;
			case floatvar_transition_base::effect_type_t::Increase:
				var_val += exp_val;
				break;
			case floatvar_transition_base::effect_type_t::ScaleDown:
				var_val /= exp_val;
				break;
			case floatvar_transition_base::effect_type_t::ScaleUp:
				var_val *= exp_val;
				break;
			case floatvar_transition_base::effect_type_t::NoEffect:
				break;
			default:
				throw runtime_error("Unsupported numeric effect in transition.");
				break;
			}
		}
	}

	int size() const
	{
		return m_size;
	}

	std::ostream & interpet_state(std::ostream & os, const state_t & state) const
	{
		for (int i = 0; i < state.size(); ++i)
		{
			os << m_varNames[i] << '=' << state[i] << std::endl;
		}

		return os;
	}

	float transition_cost(const state_t & state, const floatvar_transition_base & trans) const
	{
		//Find effect affecting total-cost
		auto it = trans.effect.find(m_costIndex);
		if (it == trans.effect.end())
			return 0.0f;
		else
		{
			float val = it->second.expr->evaluate(state);

			if (it->second.type == floatvar_transition_base::effect_type_t::Increase)
				return val;
			else
				throw runtime_error("Not implemented");
		}
		//return state[m_costIndex];
	}

	void set_transition_cost_var_index(int index)
	{
		m_costIndex = index;
	}

	int cost_var_index() const
	{
		return m_costIndex;
	}

	template<typename It>
	void remove_vars(It first, It last)
	{
		std::vector<int> indices(first, last);
		std::sort(indices.begin(), indices.end(), std::greater<int>());

		for (int idx : indices)
			m_varNames.erase(m_varNames.begin() + idx);

		m_size -= indices.size();
	}
private:
	int m_size, m_costIndex;
	std::vector<string> m_varNames;
};

typedef varset_system_base<floatvar_system_base> floatvar_system;

#endif
