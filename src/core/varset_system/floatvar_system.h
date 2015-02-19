
#ifndef UltraCore_floatvar_system_h
#define UltraCore_floatvar_system_h

#include "varset_system_base.h"

struct floatvar_transition
{
	enum class effect_type_t {NoEffect, Assign, ScaleUp, ScaleDown, Increase, Decrease};

	struct numeric_effect_t
	{
		effect_type_t type;
		float value;

		friend bool operator==(const numeric_effect_t & lhs, const numeric_effect_t & rhs)
		{
			return (lhs.type == rhs.type) && (lhs.value == rhs.value);
		}
	};

	floatvar_transition(int size = 0, const std::initializer_list<std::tuple<int, effect_type_t, float>> non_trivial_effects = {}, const std::string & _name = "")
		:effect(size, { effect_type_t::NoEffect , 0.0f}), name(_name)
	{
		for (auto & init_el : non_trivial_effects)
		{
			auto & eff = effect[get<0>(init_el)];
			eff.type = get<1>(init_el);
			eff.value = get<2>(init_el);
		}
	}

	friend bool operator==(const floatvar_transition & lhs, const floatvar_transition & rhs)
	{
		return lhs.effect == rhs.effect;
	}


	std::vector<numeric_effect_t> effect;
	std::string name;
};

class floatvar_system_base
{
	using _Base = varset_system_base<boolvar_transition>;
public:
	using state_t = std::vector<float>;
	using transition_t = floatvar_transition;

	floatvar_system_base(int var_count)
		:m_size(var_count)
	{}
		
	bool transition_available(const state_t & state, const floatvar_transition & transition) const
	{
		return true;
	}

	void apply(state_t & state, const transition_t & transition) const
	{
		for (int i = 0; i < m_size; ++i)
		{
			auto & num_eff = transition.effect[i];
			auto & var_val = state[i];
			
			switch (num_eff.type)
			{
			case floatvar_transition::effect_type_t::Assign:
				var_val = num_eff.value;
				break;
			case floatvar_transition::effect_type_t::Decrease:
				var_val -= num_eff.value;
				break;
			case floatvar_transition::effect_type_t::Increase:
				var_val += num_eff.value;
				break;
			case floatvar_transition::effect_type_t::ScaleDown:
				var_val /= num_eff.value;
				break;
			case floatvar_transition::effect_type_t::ScaleUp:
				var_val *= num_eff.value;
				break;
			case floatvar_transition::effect_type_t::NoEffect:
				break;
			default:
				throw runtime_error("Unsupported numeric effect in transition.");
				break;
			}
		}
	}
private:
	int m_size;
};

typedef varset_system_base<floatvar_system_base> floatvar_system;

#endif
