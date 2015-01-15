#ifndef UltraPlanner_UParser_h
#define UltraPlanner_UParser_h

#include "../config.h"
#include "UGrammar.h"
#include "ULexer.h"
#include "../ULanguage.h"
#include "../UPDDLDomain.h"
#include "common_objects.h"
#include <unordered_map>
#include <iostream>

class ULanguage;
struct UPDDLEffect;
struct UPDDLGoalDescription;
struct UPDDLCondition;

class parser_exception : public std::exception
{
public:
    parser_exception(const InputSymbol & error_symb, const size_t lexem_index = 0)
        :exception(), mString("Old error") //, mErrorSymb(error_symb), mIndex(lexem_index)
    {
        
    }
    
    parser_exception(const std::string & error_string)
        :exception(), mString(error_string)
    {
        
    }

    
    const std::string description() const
    {
        //return "Parsing error at (" + to_string(mErrorSymb.source_row) + ", " + to_string(mErrorSymb.source_column) + ") - unexpected lexem. Lexem #" + to_string(mIndex) + "."; //\"" + mErrorSymb.lexem->name + "\"
        return mString;
    }
    
private:
    //const InputSymbol mErrorSymb;
    //size_t mIndex;
    const std::string mString;
};

#define ENTITY_PARSER_PARAMS size_t & start_index, LexerTape lexed_symbols
class ULTRA_PDDL_API UParser
{
public:
	class SyntaxTable
	{
	public:
		SyntaxTable(vector<UGrammar::GrammarObject*> terms, vector<UGrammar::GrammarObject*> nterms, UGrammar * grammar);
		~SyntaxTable();
		UGrammar::Rule *& cell(UGrammar::Terminal * term, UGrammar::NonTerminal * nterm);
		void setRule(UGrammar::Terminal * term, UGrammar::NonTerminal * nterm, UGrammar::Rule * rule);
		void print();
	private:
		UGrammar::Rule ** m_pData;
		vector<UGrammar::GrammarObject*> mTerminals, mNTerminals;
		UGrammar * m_pGrammar;
	};

	UParser(ULanguage * _language);
    //UPDDLDomain * parseDomainSafe(LexTree & lex_tree);
	//UPDDLProblem * parseProblemSafe(LexTree & problem_tree);

	template<typename T>
	T * parseSafe(T * (UParser::*parsingMethod)(LexTree::TreeItem*, const UPDDLDomain *), LexTree & lex_tree, const UPDDLDomain * domain = nullptr)
	{
		T * result(nullptr);
    
		try
		{
			result = (this->*parsingMethod)(&lex_tree.root, domain);
		}
		catch(parser_exception & e)
		{
			cout << e.description() << "\n";
		}

		return result;
	}

	
	template<typename ObjContainer> void parseObjects(LexTree::TreeItem * objects_subtree, const UPDDLDomain * domain, ObjContainer * object_container)
	{
		auto tl = parseTypedList(objects_subtree->children.begin() + 1, objects_subtree->children.end(), Lexem::Type::Identifier, true);

		for(auto obj_list : tl.data)
		{
			auto type = domain->typeTree.type(obj_list.first);
			for(auto obj : obj_list.second)
			{
				auto new_object = new UPDDLObject(obj, type);
				object_container->addObject(new_object);
			}
		}
	}

	UPDDLDomain * parseDomain(LexTree::TreeItem * domain_tree, const UPDDLDomain * domain);
	UPDDLProblem * parseProblem(LexTree::TreeItem * problem_tree, const UPDDLDomain * domain);
    void parseRequirements(LexTree::TreeItem * req_subtree, UPDDLDomain * domain);
	void parseTypes(LexTree::TreeItem * types_subtree, UPDDLDomain * domain);
	void parsePredicates(LexTree::TreeItem * pred_subtree, UPDDLDomain * domain);
	void parseFunctions(LexTree::TreeItem * funcs_subtree, UPDDLDomain * domain);

    TypedList parseTypedList(std::vector<LexTree::Item*>::iterator first, std::vector<LexTree::Item*>::iterator last, Lexem::Type item_lexem_type, bool ext_typing);

	AtomicFormulaSkeleton parseAtomicFormulaSkeleton(LexTree::TreeItem * af_subtree);

	/*struct Function
	{
		Lexem* symbol_lexem;
		TypedList params;
		Lexem* type_lexem;
	};*/

	void parseActions(LexerTape & lexer_tape, UPDDLDomain * domain);
	UPDDLAction * parseAction(LexTree::TreeItem * act_subtree, const UPDDLDomain * domain);
	void parseConditions(LexTree::TreeItem * conditions_subtree, UPDDLAction * action, const UPDDLDomain * domain);
	UPDDLGoalDescription * parseCondition(const LexTree::TreeItem * conditions_subtree, UPDDLAction * action, const UPDDLDomain * domain);
	void parseEffects(LexTree::TreeItem * effect_subtree, UPDDLAction * action, const UPDDLDomain * domain);
	UPDDLEffect * parseEffect(LexTree::TreeItem * effect_subtree, UPDDLAction * action, const UPDDLDomain * domain);
	AtomicFormula parseAtomicFormula(const LexTree::TreeItem * effect_subtree) const;
	void parseInitialState(const LexTree::TreeItem * goals_subtree, UPDDLProblem * problem, const UPDDLDomain * domain);
	void parseGoals(const LexTree::TreeItem * atomic_formula_subtree, UPDDLProblem * problem, const UPDDLDomain * domain);
	FunctionTerm parseFunctionTerm(const LexTree::TreeItem * atomic_formula_subtree);
private:
	void buildSyntaxTable();
private:
	SyntaxTable * m_pSyntaxTable;
	ULanguage * m_pLanguage;
};

#endif
