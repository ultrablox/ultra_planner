
#ifndef UltraCore_boolvar_system_h
#define UltraCore_boolvar_system_h

#include "varset_system_base.h"
#include "../bit_container.h"
#include "../masked_bit_vector.h"



struct boolvar_transition_base
{
	boolvar_transition_base(const std::initializer_list<bool> & _condition = {}, const std::initializer_list<bool> & _condition_mask = {}, const std::initializer_list<bool> & _effect = {}, const std::initializer_list<bool> & _effect_mask = {})
		:condition(_condition, _condition_mask), effect(_effect, _effect_mask)
	{}

	friend bool operator==(const boolvar_transition_base & lhs, const boolvar_transition_base & rhs)
	{
		return (lhs.condition == rhs.condition) && (lhs.effect == rhs.effect);
	}

	masked_bit_vector condition, effect;
};

typedef varset_transition_base<boolvar_transition_base> boolvar_transition;

class boolvar_system_base
{
public:
	using state_t = bit_vector;
	using transition_t = boolvar_transition;

	boolvar_system_base(int var_count)
		:m_size(var_count)
	{}

	bool transition_available(const state_t & state, const boolvar_transition & transition) const
	{
		return state.equalMasked(transition.condition.value, transition.condition.mask) && !(state.equalMasked(transition.effect.value, transition.effect.mask));
	}

	void apply(state_t & state, const transition_t & transition) const
	{
		state.setMasked(transition.effect.value, transition.effect.mask);
	}

private:
	int m_size;
};

typedef varset_system_base<boolvar_system_base> boolvar_system;

#endif
