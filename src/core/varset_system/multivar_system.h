
#ifndef UltraCore_multivar_system_h
#define UltraCore_multivar_system_h

#include "varset_system_base.h"
#include "../masked_bit_vector.h"

struct multivar_transition_base
{
	multivar_transition_base(int var_count)
		:condition(var_count), effect(var_count)
	{}

	multivar_transition_base()
	{}
	/*

	(const bit_vector & _condition, const bit_vector & _condition_mask, const bit_vector & _effect, const bit_vector & _effect_mask)
	:condition(_condition, _condition_mask), effect(_effect, _effect_mask)
	{}

	boolvar_transition_base(int var_count)
		:condition(var_count), effect(var_count)
	{}

	boolvar_transition_base()
	{}

	void to_relaxed()
	{
		effect.mask = effect.value & effect.mask;
	}

	void build()
	{
		m_conditionVarIndices.clear();
		condition.mask.for_each_true([&](int idx){
			m_conditionVarIndices.push_back(idx);
		});

		m_affectedIndices.clear();
		effect.mask.for_each_true([&](int idx){
			m_affectedIndices.push_back(idx);
		});

	}*/

	

	friend bool operator==(const multivar_transition_base & lhs, const multivar_transition_base & rhs)
	{
		return (lhs.condition == rhs.condition) && (lhs.effect == rhs.effect);
	}

	masked_bit_vector condition, effect;
	
};


class multivar_system_base
{
public:
	using state_t = bit_vector;
	using masked_state_t = masked_bit_vector;
	using transition_t = varset_transition_base<multivar_transition_base>;

	struct var_descr_t
	{
		std::string name;
		std::vector<std::string> values;
		unsigned first_bit, bit_count;
	};


	/*template<typename VD>
	multivar_system_base(const std::initializer_list<VD> & vars)
	{}*/

	void add_var(const std::string & var_name, const std::initializer_list<std::string> & vals)
	{
		var_descr_t new_var;
		new_var.name = var_name;
		new_var.values = vals;

		m_vars.push_back(new_var);
	}

	void build()
	{
		unsigned cur_bit = 0;
		for (unsigned i = 0; i < m_vars.size(); ++i)
		{
			m_vars[i].first_bit = cur_bit;
			m_vars[i].bit_count = bits_for_representing(m_vars[i].values.size());
			cur_bit += m_vars[i].bit_count;
		}

		m_totalBits = cur_bit;
	}

	transition_t create_transition(const std::initializer_list<std::tuple<string, string, string>> & vars_changes) const
	{
		transition_t res(m_totalBits);

		for (auto & vc : vars_changes)
		{
			//Get var index
			int var_index = std::distance(m_vars.begin(), std::find_if(m_vars.begin(), m_vars.end(), [&](const var_descr_t & vd){
				return vd.name == get<0>(vc);
			}));

			//Set range in both masks
			for (int i = 0; i < m_vars[var_index].bit_count; ++i)
			{
				res.condition.mask.set(m_vars[var_index].first_bit + i, true);
				res.effect.mask.set(m_vars[var_index].first_bit + i, true);
			}

			//Set first value in condition
			int cond_val = std::distance(m_vars[var_index].values.begin(), std::find(m_vars[var_index].values.begin(), m_vars[var_index].values.end(), get<1>(vc)));
			res.condition.value.set_range(m_vars[var_index].first_bit, m_vars[var_index].first_bit + m_vars[var_index].bit_count, cond_val);
			//for (int msk = 1, i = 0; i < m_vars[var_index].bit_count; ++i, msk = msk << 1)
			//	res.condition.value.set(m_vars[var_index].first_bit + i, msk & cond_val);

			//Set second value in effect
			int eff_val = std::distance(m_vars[var_index].values.begin(), std::find(m_vars[var_index].values.begin(), m_vars[var_index].values.end(), get<2>(vc)));
			res.effect.value.set_range(m_vars[var_index].first_bit, m_vars[var_index].first_bit + m_vars[var_index].bit_count, eff_val);
			//for (int msk = 1, i = 0; i < m_vars[var_index].bit_count; ++i, msk = msk << 1)
			//	res.condition.value.set(m_vars[var_index].first_bit + i, msk & eff_val);
		}
		
		return std::move(res);
	}

	state_t create_state(const std::initializer_list<std::string> & var_vals) const
	{
		assert(var_vals.size() == m_vars.size());

		state_t new_state(m_totalBits);
		for (unsigned i = 0; i < m_vars.size(); ++i)
		{
			int val = std::distance(m_vars[i].values.begin(), std::find(m_vars[i].values.begin(), m_vars[i].values.end(), *(var_vals.begin() + i)));
			new_state.set_range(m_vars[i].first_bit, m_vars[i].first_bit + m_vars[i].bit_count, val);
		}
		return std::move(new_state);
	}

	bool transition_available(const state_t & state, const multivar_transition_base & transition) const
	{
		return state.equal_masked(transition.condition.value, transition.condition.mask);
	}

	void apply(state_t & state, const multivar_transition_base & transition) const
	{
		state.set_masked(transition.effect.value, transition.effect.mask);
	}

	std::vector<var_descr_t> m_vars;
	unsigned m_totalBits;
};

typedef varset_system_base<multivar_system_base> multivar_system;

#endif
