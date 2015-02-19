
#ifndef UltraCore_boolvar_system_h
#define UltraCore_boolvar_system_h

#include "varset_system_base.h"
#include "../bit_container.h"
#include "../masked_bit_vector.h"

struct boolvar_transition
{
	boolvar_transition(const std::initializer_list<bool> & _condition = {}, const std::initializer_list<bool> & _condition_mask = {}, const std::initializer_list<bool> & _effect = {}, const std::initializer_list<bool> & _effect_mask = {}, const std::string _name = "")
		:condition(_condition, _condition_mask), effect(_effect, _effect_mask), name(_name)
	{}

	friend bool operator==(const boolvar_transition & lhs, const boolvar_transition & rhs)
	{
		return (lhs.condition == rhs.condition) && (lhs.effect == rhs.effect) && (lhs.name == rhs.name);
	}

	masked_bit_vector condition, effect;
	std::string name;
};

class boolvar_system : public varset_system_base<boolvar_transition>
{
	using _Base = varset_system_base<boolvar_transition>;
public:
	using state_t = bit_vector;
	using transition_t = boolvar_transition;

	boolvar_system(int var_count)
		:_Base(var_count)
	{}

	bool transition_available(const state_t & state, const boolvar_transition & transition) const
	{
		return state.equalMasked(transition.condition.value, transition.condition.mask) && !(state.equalMasked(transition.effect.value, transition.effect.mask));
	}

	template<typename F>
	void forall_available_transitions(const state_t & base_state, F fun) const
	{
		for (auto & transition : m_transitions)
		{
			if (transition_available(base_state, transition))
				fun(transition);
		}
	}

	void apply(state_t & state, const transition_t & transition) const
	{
		state.setMasked(transition.effect.value, transition.effect.mask);
	}

};

#endif
