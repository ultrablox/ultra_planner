
#include "numeric_expression.h"

numeric_expression::numeric_expression()
{
}

numeric_expression::~numeric_expression()
{}

float numeric_expression::evaluate(const UFloatVector & float_vars) const
{
	return m_pRootNode->value(float_vars);
}

void numeric_expression::replace_with_const(int var_index, float const_val)
{
	if (m_pRootNode->type == UExpressionNode::Type::Variable)
	{
		auto var_exp = static_cast<const UVariableExpressionNode*>(m_pRootNode.get());
		if (var_exp->m_varIndex == var_index)
			m_pRootNode.reset(new UValueExpressionNode(const_val));
	}
}

numeric_expression * numeric_expression::simpleValue(const float value)
{
	numeric_expression * res = new numeric_expression;
	res->m_pRootNode.reset(new UValueExpressionNode(value));
	return res;
}

numeric_expression * numeric_expression::simpleVariable(int var_index)
{
	numeric_expression * res = new numeric_expression;
	res->m_pRootNode.reset(new UVariableExpressionNode(var_index));
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
