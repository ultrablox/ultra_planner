#include "UTransitionSystem.h"
#include "../UError.h"
#include "UTransition.h"

//==================================UTransitionSystem==========================
UTransitionSystem::UTransitionSystem(const int boolean_vars_count, int float_count)
	:mBoolCount(boolean_vars_count), m_floatCount(float_count)
{

}

int UTransitionSystem::boolCount() const
{
	return mBoolCount;
}

int UTransitionSystem::floatCount() const
{
	return m_floatCount;
}

std::vector<size_t> UTransitionSystem::getAvalibleTransitions(const UState & state) const
{
	std::vector<size_t> result;
	result.reserve(mTransitions.size());
	for(size_t i = 0, transition_count = mTransitions.size(); i < transition_count; ++i)
	{
		if(checkTransition(state, mTransitions[i]))
		{
			result.push_back(i);
			//result.insert(i);
		}
	}

	result.shrink_to_fit();
	return result;
}

UBitset UTransitionSystem::getAvalibleTransitionsMask(const UState & state) const
{
	UBitset res(mTransitions.size());
	res.clear();

	for(int i = 0; i < mTransitions.size(); ++i)
	{
		if(checkTransition(state, mTransitions[i]))
			res.set(i, true);
	}

	return res;
}

bool UTransitionSystem::checkTransition(const UState & state, const UTransition & transition) const
{
	//Check predicate states
	bool r = state.flags.equalMasked(transition.condition.flags.state, transition.condition.flags.mask);
	if(!r)
		return false;

	//Check that transition can be useful (will affect state)
	r = state.flags.equalMasked(transition.effect.flags.state, transition.effect.flags.mask);
	if(r)
		return false;
	
	return true;
}

const UTransition & UTransitionSystem::transition(const size_t transition_index) const
{
	return mTransitions[transition_index];
}

UTransition & UTransitionSystem::transition(const size_t transition_index)
{
	return mTransitions[transition_index];
}

const std::vector<UTransition> & UTransitionSystem::transitions() const
{
	return mTransitions;
}

UState UTransitionSystem::applyTransition(const UState & base_state, const size_t transition_index) const
{
	const UTransition & transition = mTransitions[transition_index];
	return base_state + transition;
}

void UTransitionSystem::removeTransition(const size_t transition_index)
{
	mTransitions.erase(mTransitions.begin()+transition_index);
}

size_t UTransitionSystem::transitionBetween(const UState & s1, const UState & s2) const
{
	for(size_t i = 0; i < mTransitions.size(); ++i)
	{
		//mTransitions[i].print();
		//cout << "\n";

		if(s1 + mTransitions[i] == s2)
			return i;
	}

	throw core_exception("Unable to find transition");

	return 0;
}

UTransitionSystem UTransitionSystem::toRelaxed() const
{
	UTransitionSystem res(*this);

	for(auto & tr : res.mTransitions)
		tr = tr.toRelaxed();

	return res;
}

void UTransitionSystem::serialize(std::ofstream & os) const
{
	//Count
	serialize_value<size_t>(os, mTransitions.size());

	//Data
	for(auto & tr : mTransitions)
		tr.serialize(os);

	//Flags descriptions
	serialize_value<size_t>(os, m_flagsDescription.size());
	for(auto fd : m_flagsDescription)
		serialize_string(os, fd);
}

int UTransitionSystem::deserialize(std::ifstream & is)
{
	//Count
	auto transition_count = deserialize_value<size_t>(is);
	cout << "Total " << transition_count << " transitions.\n";
	mTransitions.resize(transition_count);

	//Data
	int res = 0;
	for(size_t i = 0; i < transition_count; ++i)
	{
		res = mTransitions[i].deserialize(is);
		if(res)
		{
			cout << "Failed to deserialize " << i << " transition.\n";
			return res;
		}
	}

	//Flags descriptions
	size_t descr_count = deserialize_value<size_t>(is);
	m_flagsDescription.resize(descr_count);
	for(int i = 0; i < descr_count; ++i)
	{
		m_flagsDescription[i] = deserialize_string(is);
	}

	return 0;
}

void UTransitionSystem::setFlagsDescription(const std::vector<std::string> & flags_description)
{
	m_flagsDescription = flags_description;
}

void UTransitionSystem::setFloatsDescription(const std::vector<std::string> & floats_description)
{
	m_floatsDescription = floats_description;
}

std::vector<string> UTransitionSystem::interpretState(const UState & state) const
{
	std::vector<string> res;

	auto flag_indices = state.flags.toIndices();
	for(auto i : flag_indices)
		res.push_back(m_flagsDescription[i]);

	std::sort(res.begin(), res.end());

	for(int i = 0; i < state.functionValues.size(); ++i)
		res.push_back(m_floatsDescription[i] + "=" + to_string(state.functionValues[i]));
	

	return res;
}

UTransition & UTransitionSystem::createTransition()
{
	mTransitions.push_back(UTransition(mBoolCount, m_floatCount));
	return mTransitions[mTransitions.size()-1];
}

UState UTransitionSystem::createState() const
{
	return UState(mBoolCount, m_floatCount);
}

UPartialState UTransitionSystem::createPartialState() const
{
	return UPartialState(mBoolCount);
}

std::string UTransitionSystem::transitionDescription(int index) const
{
	return transition(index).description();
}

void UTransitionSystem::toMultivalueEncoding() const
{

}
