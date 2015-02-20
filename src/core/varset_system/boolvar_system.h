
#ifndef UltraCore_boolvar_system_h
#define UltraCore_boolvar_system_h

#include "varset_system_base.h"
#include "../bit_container.h"
#include "../masked_bit_vector.h"
#include "../streamer.h"
#include "../compressed_stream.h"

struct boolvar_transition_base
{
	boolvar_transition_base(const bit_vector & _condition, const bit_vector & _condition_mask, const bit_vector & _effect, const bit_vector & _effect_mask)
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

	friend bool operator==(const boolvar_transition_base & lhs, const boolvar_transition_base & rhs)
	{
		return (lhs.condition == rhs.condition) && (lhs.effect == rhs.effect);
	}

	masked_bit_vector condition, effect;
};

class boolvar_system_base
{
public:
	using state_t = bit_vector;
	using masked_state_t = masked_bit_vector;
	using transition_t = varset_transition_base<boolvar_transition_base>;

	class state_streamer_t : public streamer_base
	{
	public:
		state_streamer_t(const boolvar_system_base & _bsystem)
			:streamer_base(integer_ceil(_bsystem.size(), 8)), m_bitCount(_bsystem.size())
		{}

		void serialize(void * dst, const state_t & state) const
		{
			compressed_stream wstream(dst);
			wstream.write(state);
		}

		void deserialize(const void * src, state_t & state) const
		{
			compressed_stream rstream(src);
			state.resize(m_bitCount);
			rstream.read(state);
		}

	private:
		int m_bitCount;
	};

	boolvar_system_base(int var_count)
		:m_size(var_count), m_varNames(var_count, "unnamed")
	{}

	bool transition_available(const state_t & state, const boolvar_transition_base & transition) const
	{
		return state.equalMasked(transition.condition.value, transition.condition.mask) && !(state.equalMasked(transition.effect.value, transition.effect.mask));
	}

	void apply(state_t & state, const boolvar_transition_base & transition) const
	{
		state.setMasked(transition.effect.value, transition.effect.mask);
	}

	std::ostream & interpet_state(std::ostream & os, const state_t & state) const
	{
		for (int i = 0; i < state.size(); ++i)
		{
			if (state[i])
				os << m_varNames[i] << std::endl;
		}
		return os;
	}

	int size() const
	{
		return m_size;
	}

	void set_var_name(int index, const std::string _name)
	{
		m_varNames[index] = _name;
	}

private:
	int m_size;
	std::vector<string> m_varNames;
};

typedef varset_system_base<boolvar_system_base> boolvar_system;

#endif
