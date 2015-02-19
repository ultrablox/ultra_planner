
#include "UState.h"
#include "UTransition.h"
#include "../UError.h"
//#include "../UStateFactory.h"

using namespace Serializing;

size_t UFloatVector::serialize(char * dest) const
{
	size_t r(0);

	r += serialize_int(dest, size());

	const size_t data_size = sizeof(float) * size();
	memcpy(dest + r, data(), data_size);
	r += data_size;

	return r;
}

size_t UFloatVector::deserialize(char * dest)
{
	size_t r(0);

	//Count
	int varCount;
	r += deserialize_int(dest, varCount);

	//Data
	const size_t data_size = sizeof(float) * varCount;

	resize(varCount);
	memcpy(data(), dest + r, data_size);
	r += data_size;

	return r;
}

std::ostream & operator<<(std::ostream & os, const UFloatVector & vec)
{
	auto beg_it = vec.begin();

	for(auto it = beg_it; it != vec.end(); ++it)
	{
		if(it != beg_it)
			os << ",";
		os << *it;
	}

	return os;
}

//===============================UState====================================
UState::UState(int predicate_instances_count, int function_instances_count)
{
	if(predicate_instances_count > 0)
		flags.resize(predicate_instances_count);
	if(function_instances_count > 0)
		functionValues.resize(function_instances_count);
}

UState::UState(const UState & other)
	:flags(other.flags), functionValues(other.functionValues)
{

}
/*
UState::UState(const UState && other)
	:flags(other.flags), functionValues(other.functionValues)
{
}*/

UState::~UState()
{
}

void UState::clear()
{
	flags.setValues(false);

	for(auto fv : functionValues)
		fv = 0.0f;
}

void UState::setPredicateState(size_t predicate_index, bool value)
{
	flags[predicate_index] = value;
}

void UState::apply(const UTransition & transition)
{
	//Flags changing - very fast
	flags.setMasked(transition.effect.flags.state, transition.effect.flags.mask);
	
	//Numeric changes - the slowest part
	for(int i = 0; i < transition.effect.numeric.size(); ++i)
	{
		auto & num_eff = transition.effect.numeric[i];
		switch (num_eff.type)
		{
		case UNumericEffect::Type::Assign:
			functionValues[i] = num_eff.value(this->functionValues);
			break;
		case UNumericEffect::Type::Decrease:
			functionValues[i] -= num_eff.value(this->functionValues);
			break;
		case UNumericEffect::Type::Increase:
			functionValues[i] += num_eff.value(this->functionValues);
			break;
		case UNumericEffect::Type::ScaleDown:
			functionValues[i] /= num_eff.value(this->functionValues);
			break;
		case UNumericEffect::Type::ScaleUp:
			functionValues[i] *= num_eff.value(this->functionValues);
			break;
		case UNumericEffect::Type::NoEffect:
			break;
		default:
			throw core_exception("Unsupported numeric effect in transition.");
			break;
		}
	}
}

void UState::applyRelaxed(const UTransition & transition)
{
	UTransition relaxed_tr = transition.toRelaxed();
	apply(relaxed_tr);
}

//http://stackoverflow.com/questions/9040689/stl-less-operator-and-invalid-operator-error
bool operator<(const UState & s1, const UState & s2)
{
	return (memcmp(s1.flags.mData.data(), s2.flags.mData.data(), s1.flags.byteCount()) < 0);
	/*if(memcmp(s1.flags.mData.data(), s2.flags.mData.data(), s1.flags.byteCount()) < 0)
		return true;
	else
		return (memcmp(s1.functionValues.data(), s2.functionValues.data(), s1.functionValues.size() * sizeof(UFloatVector::value_type)) < 0);
	if(s1.flags.mData < s2.flags.mData)
		return true;

	if(s1.functionValues < s2.functionValues)
		return true;
	for(int i = 0; i < s1.flags.mData.size(); ++i)
		if(s1.flags.mData[i] < s2.flags.mData[i])
			return true;

	for(int i = 0; i < s1.functionValues.size(); ++i)
		if(s1.functionValues[i] < s2.functionValues[i])
			return true;

	return false;	*/
}

/*
std::hash<size_t> UState::hash() const
{
	return flags.hash();
}*/

void * UState::toPlainData() const
{
	char * plain_data = (char*)malloc(plainDataSize());
	int cur_index = 0;
	//================Flags======================
	plain_data[cur_index++] = flags.mData.size();
	memcpy(&plain_data[cur_index], flags.mData.data(), flags.mData.size() * flags.elementByteCount());
	cur_index += 1 + flags.mData.size() * flags.elementByteCount();

	//================Numeric====================
	plain_data[cur_index++] = functionValues.size();
	memcpy(&plain_data[cur_index], functionValues.data(), functionValues.size() * sizeof(float));

	return plain_data;
}

size_t UState::serialize(char * dest) const
{
	size_t res(0);
	res += flags.serialize(dest);
	res += functionValues.serialize(dest + res);
	return res;
}

void UState::deserialize(char * src)
{
	size_t r(0);
	r += flags.deserialize(src);
	r += functionValues.deserialize(src+r);
}

size_t UState::plainDataSize() const
{
	return sizeof(int) + flags.mData.size() * flags.elementByteCount() + sizeof(int) + functionValues.size() * sizeof(float);
}

void UState::print(ostream & os) const
{
	flags.print(os, false);
	os << ":";
	os << functionValues;
}

bool operator==(const UState & s1, const UState & s2)
{
	return (s1.flags == s2.flags);// && (s1.functionValues == s2.functionValues);
}

std::string to_string(const UState & state)
{
	stringstream ss;
	state.print(ss);

	string res;
	ss >> res;
	return res;
}

//==========================Partial state========================

UPartialState::UPartialState(int bool_count)
	:flags(UMaskedBitVector(bool_count))
{
}

bool UPartialState::matches(const UState & state) const
{
	return state.flags.equalMasked(flags.state, flags.mask);
}
