#ifndef UltraPlanner_combinedvar_system_h
#define UltraPlanner_combinedvar_system_h

#include "varset_system_base.h"
#include "boolvar_system.h"
#include "floatvar_system.h"
#include "transition_index.h"

#include "../config.h"
#include <ostream>

/*#include "../bit_container.h"
#include "UState.h"
#include "UTransition.h"
#include <bitset>
#include <array>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <list>
#include <set>


//System that consists from rules about state changing.

class ULTRA_CORE_API UTransitionSystem
{
public:
	UTransitionSystem(const int boolean_vars_count = 0, int float_count = 0);

	void addTransition(const UTransition & new_transition)
	{
		mTransitions.push_back(new_transition);
	}

	//Returns a number of boolean-variables.
	int boolCount() const;
	int floatCount() const;

	std::vector<size_t> getAvalibleTransitions(const UState & state) const;

	
	//Checks avalible transitions in given state and sets to 1 bits in corresponding positions.
	UBitset getAvalibleTransitionsMask(const UState & state) const;

	//Checks if given transition can be applied in given state.
	bool checkTransition(const UState & state, const UTransition & transition) const;

	const UTransition & transition(const size_t transition_index) const;
	UTransition & transition(const size_t transition_index);
	const std::vector<UTransition> & transitions() const;

	void print() const
	{
		for(auto tr : mTransitions)
			tr.condition.flags.state.print();
	}

	//Applies transition with given index to given state and returns result state.
	UState applyTransition(const UState & base_state, const size_t transition_index) const;

	void removeTransition(const size_t transition_index);
	size_t transitionBetween(const UState & s1, const UState & s2) const;

	UTransitionSystem toRelaxed() const;

	void serialize(std::ofstream & os) const;
	int deserialize(std::ifstream & is);

	void setFlagsDescription(const std::vector<std::string> & flags_description);
	void setFloatsDescription(const std::vector<std::string> & floats_description);

	std::vector<string> interpretState(const UState & state) const;

	UTransition & createTransition();
	UState createState() const;
	UPartialState createPartialState() const;

	std::string transitionDescription(int index) const;
	void toMultivalueEncoding() const;
private:
	std::vector<UTransition> mTransitions;

	std::vector<std::string> m_flagsDescription, m_floatsDescription;
	int mBoolCount, m_floatCount;
	//UState mState;
};*/


struct combined_transition_base
{
	explicit combined_transition_base(const boolvar_transition_base & _bools = {}, const floatvar_transition_base & _floats = {})
	:bool_part(_bools), float_part(_floats)
	{}

	explicit combined_transition_base(int bool_count, int float_count)
		:bool_part(bool_count), float_part(float_count)
	{}

	void to_relaxed()
	{
		bool_part.to_relaxed();
	}

	friend bool operator==(const combined_transition_base & lhs, const combined_transition_base & rhs)
	{
		return (lhs.bool_part == rhs.bool_part) == (lhs.float_part == rhs.float_part);
	}
	
	boolvar_transition_base bool_part;
	floatvar_transition_base float_part;
};

struct combined_state
{
	combined_state(int bool_count, int float_count)
		:bool_part(bool_count), float_part(float_count)
	{}

	combined_state(const boolvar_system_base::state_t & bools = {}, const floatvar_system_base::state_t & floats = {})
		:bool_part(bools), float_part(floats)
	{}

	friend bool operator==(const combined_state & lhs, const combined_state & rhs)
	{
		return (lhs.bool_part == rhs.bool_part) && (lhs.float_part == rhs.float_part);
	}

	boolvar_system_base::state_t bool_part;
	floatvar_system_base::state_t float_part;
};


namespace std {
	template<>
	class hash<combined_state>
	{
	public:

		size_t operator()(const combined_state & bv) const
		{
			return std::hash<bit_vector>()(bv.bool_part);
		}
	};
}

class combinedvar_system_base
{
public:
	using transition_t = varset_transition_base<combined_transition_base>;
	using state_t = combined_state;

	struct masked_state_t
	{
		masked_state_t(int bool_count = 0, int float_count = 0)
			:bool_part(bool_count)//, float_part(float_count)
		{}

		boolvar_system_base::masked_state_t bool_part;
		floatvar_system_base::masked_state_t float_part;
	};

	class state_streamer_t : public streamer_base
	{
	public:
		state_streamer_t(const combinedvar_system_base & _csystem)
			:streamer_base(), m_bSystem(_csystem.m_boolPart), m_fSystem(_csystem.m_floatPart)
		{
			streamer_base::m_serializedSize = m_bSystem.serialized_size() + m_fSystem.serialized_size();
		}

		void serialize(void * dst, const state_t & state) const
		{
			m_fSystem.serialize(dst, state.float_part);
			m_bSystem.serialize((char*)dst + m_fSystem.serialized_size(), state.bool_part);
		}

		void deserialize(const void * src, state_t & state) const
		{
			m_fSystem.deserialize(src, state.float_part);
			m_bSystem.deserialize((const char*)src + m_fSystem.serialized_size(), state.bool_part);
		}

	private:
		boolvar_system_base::state_streamer_t m_bSystem;
		floatvar_system_base::state_streamer_t m_fSystem;
	};

	combinedvar_system_base(int bool_count = 0, int float_count = 0)
		:m_boolPart(bool_count), m_floatPart(float_count)
	{}

	void apply(state_t & state, const transition_t & transition) const
	{
		m_boolPart.apply(state.bool_part, transition.bool_part);
		m_floatPart.apply(state.float_part, transition.float_part);
	}

	bool transition_available(const state_t & state, const transition_t & transition) const
	{
		return m_boolPart.transition_available(state.bool_part, transition.bool_part) && m_floatPart.transition_available(state.float_part, transition.float_part);
	}

	std::ostream & interpet_state(std::ostream & os, const state_t & state) const
	{
		m_boolPart.interpet_state(os, state.bool_part);
		m_floatPart.interpet_state(os, state.float_part);
		return os;
	}

	boolvar_system_base & bool_part()
	{
		return m_boolPart;
	}

	const boolvar_system_base & bool_part() const
	{
		return m_boolPart;
	}

	floatvar_system_base & float_part()
	{
		return m_floatPart;
	}

	bool is_solved(const state_t & state) const
	{
		return state.bool_part.equal_masked(m_goalState.bool_part.mask, m_goalState.bool_part.value);
	}

	void set_goal_state(const masked_state_t & _state)
	{
		m_goalState = _state;
	}

	const masked_state_t & goal_state() const
	{
		return m_goalState;
	}

	float transition_cost(const state_t & state, const transition_t & trans) const
	{
		return m_floatPart.transition_cost(state.float_part, trans.float_part);
	}
	
private:
	boolvar_system_base m_boolPart;
	floatvar_system_base m_floatPart;
	masked_state_t m_goalState;
};

struct combinedvar_system : public varset_system_base<combinedvar_system_base>
{
	using _Base = varset_system_base<combinedvar_system_base>;
	using state_t = _Base::state_t;

	

	combinedvar_system(int bool_count = 0, int float_count = 0)
		:_Base(bool_count, float_count)
	{}

	void build_transitions_index()
	{
		for (auto & tr : m_transitions)
			tr.bool_part.build();

		//m_index.build(m_transitions.begin(), m_transitions.end());
	}

	template<typename F>
	void forall_available_transitions_(const state_t & base_state, F fun) const
	{
		m_index.forall_available_transitions(base_state, [=](const state_t & state, const transition_t & transition){
			return this->transition_available(state, transition);
		}, fun);
	}

	indexing::transition_index<_Base::transition_t, state_t> m_index;
};

#endif
