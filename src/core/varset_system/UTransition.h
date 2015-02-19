
#ifndef UltraPlanner_UTransition_h
#define UltraPlanner_UTransition_h

#include "../config.h"
#include "../UBitset.h"
#include "UState.h"
#include "../utils/helpers.h"
#include "../expression/UNumericExpression.h"
#include <iostream>

struct UNumericEffect
{
	enum class Type {NoEffect, Assign, ScaleUp, ScaleDown, Increase, Decrease};

	UNumericEffect()
		:type(Type::NoEffect), expr(nullptr)
	{}

	size_t serialize(char * dest) const
	{
		size_t r(0);
		//r += Serializing::serialize<float>(dest, value);
		r += Serializing::serialize_uchar(dest + r, static_cast<unsigned char>(type));
		return r;
	}

	float value(const UFloatVector & float_vars) const
	{
		return expr->evaluate(float_vars);
	}

	Type type;

	UNumericExpression * expr;
};

class ULTRA_CORE_API UTransition
{
public:
	UTransition(size_t flags_count = 0, size_t numeric_count = 0);

	struct Condition
	{
		Condition(size_t flags_count);
		const size_t plainDataSize() const;
		void serialize(std::ofstream & os) const;
		int deserialize(std::ifstream & is);

		UMaskedBitVector flags;
		//Complex conditions needed to be calculated
	} condition;

	struct Effect
	{
		Effect(size_t flags_count, size_t numeric_count)
			:flags(flags_count), numeric(numeric_count)
		{

		}

		UMaskedBitVector flags;

		class NumericPart : public std::vector<UNumericEffect>
		{
		public:
			NumericPart(size_t num_count = 0)
				:vector<UNumericEffect>(num_count)
			{}

			void set(const size_t function_index, const UNumericEffect::Type numeric_type, const float value)
			{
				set(function_index, numeric_type, UNumericExpression::simpleValue(value));
			}

			void set(const size_t function_index, const UNumericEffect::Type numeric_type, UNumericExpression * expr)
			{
				(*this)[function_index].type = numeric_type;
				(*this)[function_index].expr = expr;
			}

			const size_t plainDataSize() const
			{
				return sizeof(int32_t) + (sizeof(float) + sizeof(char)) * size();
			}

			size_t serialize(char * dest) const
			{
				size_t r = 0;
				r += serialize_int(dest, size());

				for(int i = 0; i < size(); ++i)
				{
					r += at(i).serialize(dest + r);
				}
				return r;
			}

			friend std::ostream & operator<<(std::ostream & os, const NumericPart & num);
		} numeric;

		const size_t plainDataSize() const
		{
			return numeric.plainDataSize() + flags.plainDataSize();
		}

		void serialize(std::ofstream & os) const;
		int deserialize(std::ifstream & is);

	} effect;

	void setPredicateCondition(const int predicate_index, const bool value);
	UTransition toRelaxed() const;
	const size_t plainDataSize() const;
	size_t serialize(char * dest) const;

	void serialize(std::ofstream & os) const;
	int deserialize(std::ifstream & is);

	void setDescription(const std::string & description_);
	std::string description() const;
	friend std::ostream & operator<<(std::ostream & os, const UTransition & transition);
private:
	std::string m_description;
};

typedef std::vector<int> TransitionPath;

//Applies transition to given state
ULTRA_CORE_API UState operator+(const UState & state, const UTransition & transition);

#endif
