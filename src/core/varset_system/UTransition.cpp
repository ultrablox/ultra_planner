
#include "UTransition.h"
#include "../UError.h"

UTransition::Condition::Condition(size_t flags_count)
	:flags(flags_count)
{

}

const size_t UTransition::Condition::plainDataSize() const
{
	return flags.plainDataSize();
}

void UTransition::Condition::serialize(std::ofstream & os) const
{
	flags.serialize(os);
}

int UTransition::Condition::deserialize(std::ifstream & is)
{
	return flags.deserialize(is);
}

//========================================Effect===================================
void UTransition::Effect::serialize(std::ofstream & os) const
{
	flags.serialize(os);
	
	serialize_vector(os, numeric);
}

int UTransition::Effect::deserialize(std::ifstream & is)
{
	int res = flags.deserialize(is);
	if(res)
		return res;
		
	numeric = deserialize_vector<NumericPart>(is);

	return res;
}

//=========================================Transition====================================
UTransition::UTransition(size_t flags_count, size_t numeric_count)
	:condition(flags_count), effect(flags_count, numeric_count) 
{
	condition.flags.resize(flags_count);
	condition.flags.clear();

	effect.flags.resize(flags_count);
	effect.flags.clear();
}

void UTransition::setPredicateCondition(const int predicate_index, const bool value)
{
	condition.flags.set(predicate_index, value);
}


UTransition UTransition::toRelaxed() const
{
	UTransition res(*this);
	res.effect.flags.mask = effect.flags.state & effect.flags.mask;

	return std::move(res);
}

UState operator+(const UState & state, const UTransition & transition)
{
	UState result(state);
	result.apply(transition);
	return std::move(result);
}

const size_t UTransition::plainDataSize() const
{
	return condition.plainDataSize() + effect.plainDataSize();
}

size_t UTransition::serialize(char * dest) const
{
	size_t res(0);

	//================Condition=================
	res += condition.flags.serialize(dest);

	//================Effect====================
	res += effect.flags.serialize(dest + res);
	res += effect.numeric.serialize(dest + res);

	return res;
}

void UTransition::serialize(std::ofstream & os) const
{
	//Condition
	condition.serialize(os);

	//Effect
	effect.serialize(os);

	//Description
	serialize_string(os, m_description);
}

int UTransition::deserialize(std::ifstream & is)
{
	//Condition
	int res = condition.deserialize(is);
	if(res)
	{
		cout << "Failed to deserialize condition.\n";
		return res;
	}

	//Effect
	res = effect.deserialize(is);
	if(res)
	{
		cout << "Failed to deserialize effect.\n";
		return res;
	}

	//Description
	m_description = deserialize_string(is);

	return res;
}

void UTransition::setDescription(const std::string & description_)
{
	m_description = description_;
}

std::string UTransition::description() const
{
	return m_description;
}

std::ostream & operator<<(std::ostream & os, const UTransition & transition)
{
	cout << "Transition " << transition.m_description << ":\n";

	cout << "condition:\n";
	transition.condition.flags.print(os);

	cout << "effect:\n";
	transition.effect.flags.print(os);
	cout << transition.effect.numeric;

	return os;
}

std::ostream & operator<<(std::ostream & os, const UTransition::Effect::NumericPart & num)
{
	for(auto & n : num)
	{
		switch(n.type)
		{
		case UNumericEffect::Type::Assign:
			os << "=";
			break;
		case UNumericEffect::Type::Decrease:
			os << "-";
			break;
		case UNumericEffect::Type::Increase:
			os << "+";
			break;
		case UNumericEffect::Type::NoEffect:
			os << "@";
			break;
		case UNumericEffect::Type::ScaleDown:
			os << "<";
			break;
		case UNumericEffect::Type::ScaleUp:
			os << ">";
			break;
		}
	}

	cout << "\n";

	return os;
}
