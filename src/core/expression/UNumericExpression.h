

#ifndef UltraCore_UNumericExpression_h
#define UltraCore_UNumericExpression_h

#include "../transition_system/UState.h"
#include <memory>

struct UExpressionNode;

struct UNumericExpression
{
	UNumericExpression()
		:m_pRootNode(nullptr)
	{
	}

	float evaluate(const UFloatVector & float_vars) const;

	static UNumericExpression * simpleValue(const float value);
	static UNumericExpression * simpleVariable(int var_index);

	UExpressionNode * m_pRootNode;
};

struct UExpressionNode
{
	enum class Type {Value, Variable, Node};

	UExpressionNode(Type _type);
	float value(const UFloatVector & float_vars) const;

	const Type type;
};

struct UValueExpressionNode : public UExpressionNode
{
	UValueExpressionNode(float _value)
		:UExpressionNode(Type::Value), m_value(_value)
	{}

	inline float value() const;

	float m_value;
};


struct UVariableExpressionNode : public UExpressionNode
{
	UVariableExpressionNode(int var_index)
		:UExpressionNode(Type::Variable), m_varIndex(var_index)
	{}

	inline float value(const UFloatVector & float_vars) const;

	int m_varIndex;
};

#endif
