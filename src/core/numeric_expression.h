

#ifndef UltraCore_numeric_expression_h
#define UltraCore_numeric_expression_h

//#include "../transition_system/UState.h"
#include <memory>
#include <vector>
#include "config.h"

struct UExpressionNode;

using UFloatVector = std::vector<float>;
struct ULTRA_CORE_API numeric_expression
{
	numeric_expression();
	~numeric_expression();

	float evaluate(const UFloatVector & float_vars) const;
	void replace_with_const(int var_index, float const_val);

	static numeric_expression * simpleValue(const float value);
	static numeric_expression * simpleVariable(int var_index);

	std::shared_ptr<UExpressionNode> m_pRootNode;
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
