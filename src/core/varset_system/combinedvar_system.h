#ifndef UltraPlanner_combinedvar_system_h
#define UltraPlanner_combinedvar_system_h

#include "varset_system_base.h"
#include "boolvar_system.h"
#include "floatvar_system.h"

#include "../config.h"
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


struct combined_transition
{
	boolvar_transition bool_part;
	floatvar_transition float_part;
};

struct combined_state
{
	boolvar_system_base::state_t bool_part;
	floatvar_system_base::state_t float_part;
};

class combinedvar_system_base
{
public:
	using transition_t = combined_transition;
	using state_t = combined_state;

	combinedvar_system_base(int bool_count, int float_count)
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
private:
	boolvar_system_base m_boolPart;
	floatvar_system_base m_floatPart;
};

typedef varset_system_base<combinedvar_system_base> combinedvar_system;

#endif
