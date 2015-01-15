#ifndef UltraPlanner_translator_common_objects_h
#define UltraPlanner_translator_common_objects_h

#include <unordered_map>
#include <string>
#include <vector>

struct Lexem
{
	enum class Type {Separator, Space, //This symbols are used to identify lexer next word finish position
					Operator, Keyword, Requierment, SpecSymbol, Identifier, Variable, Number, LogicalOperator};

    Lexem(const std::string & _name, const Type _type)
        :name(_name), type(_type)
    {
            
    }

	Type type;
    std::string name;
};

struct TypedList
{
    typedef std::vector<std::string> TypeGroup;
    typedef std::pair<std::string, TypeGroup> TypeRecord;
	//std::unordered_map<std::string, > data;
    
    TypeGroup & operator[](const std::string & group_name);
    
    
    std::vector<TypeRecord> data;
};

struct AtomicFormulaSkeleton
{
	Lexem * name_lexem;
	TypedList typed_list;
};

struct AtomicFormula
{
	Lexem * name_lexem;
	std::vector<Lexem*> param_list;
};

struct FunctionTerm
{
	Lexem * function_name;
	std::vector<Lexem*> param_list;
	Lexem * value_lexem;
};

struct Effect
{

};

#endif
