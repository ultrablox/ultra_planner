
#include "numeric_expression.h"


float UNumericExpression::evaluate(const UFloatVector & float_vars) const
{
	return m_pRootNode->value(float_vars);
}

UNumericExpression * UNumericExpression::simpleValue(const float value)
{
	UNumericExpression * res = new UNumericExpression;
	res->m_pRootNode = new UValueExpressionNode(value);
	return res;
}

UNumericExpression * UNumericExpression::simpleVariable(int var_index)
{
	UNumericExpression * res = new UNumericExpression;
	res->m_pRootNode = new UVariableExpressionNode(var_index);
	return res;
}

UExpressionNode::UExpressionNode(Type _type)
	:type(_type)
{
}

float UExpressionNode::value(const UFloatVector & float_vars) const
{
	switch(type)
	{
	case Type::Value:
		return static_cast<const UValueExpressionNode*>(this)->value();
	case Type::Variable:
		return static_cast<const UVariableExpressionNode*>(this)->value(float_vars);
	default:
		return 0.0f;
	}
}

float UValueExpressionNode::value() const
{
	return m_value;
}

float UVariableExpressionNode::value(const UFloatVector & float_vars) const
{
	return float_vars[m_varIndex];
}
