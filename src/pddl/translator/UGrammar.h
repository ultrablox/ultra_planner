#ifndef UltraPlanner_UGrammar_h
#define UltraPlanner_UGrammar_h

#include "../config.h"
#include <vector>

using namespace std;

#define MAX_FULL_NAME_LENGTH 128
#define SPECIAL_SYMBOL_EMPTY "_eps_"
#define SPECIAL_SYMBOL_LESS "_ls_"
#define SPECIAL_SYMBOL_GREATER "_gr_"
#define SPECIAL_SYMBOL_IDENTIFIER "_identifier_"
#define SPECIAL_SYMBOL_SPACE " "
#define SPECIAL_SYMBOL_RETURN "\n"
#define SPECIAL_SYMBOL_COMMENTS_SEPARATOR ";;"

class ComplexNamedObject
{
public:
	ComplexNamedObject(const char * full_name);
	const char * name();
private:
	char mFullName[MAX_FULL_NAME_LENGTH];
};

/*
Describes grammar built from productions.
I.e. S->Aa|b|S, A->b.
*/
class ULTRA_PDDL_API UGrammar
{
public:
	class GrammarObject : public ComplexNamedObject
	{
	public:
		typedef enum {Terminal, NonTerminal} Type;

		GrammarObject(const char * name);
		void print();
		virtual int type() = 0;
	};

	class GrammarObjectSequence : public vector<GrammarObject*>
	{
	public:
		GrammarObjectSequence();
		GrammarObjectSequence(GrammarObject * obj);
		void print();
		bool contains(GrammarObject * object);
	};

	class Terminal : public GrammarObject
	{
	public:
		Terminal(const char * name);
		virtual int type();
		bool isEmpty();
	};

	class NonTerminal : public GrammarObject
	{
	public:
		NonTerminal(const char * name);
		virtual int type();
	};

	class Rule
	{
	public:
		/*
		Creates rule with empty right part.
		*/
		Rule(NonTerminal * nterm);

		/*
		Creates rule, automatically finding using terminals and non-terminals
		in given dictionaries, and addin symbols that were not found.
		*/
		Rule(const char * left_part_bnf, const char * right_part_bnf, UGrammar * grammar);

		/*
		Prints rule to std::out.
		*/
		void print();
		NonTerminal * nterm();
		GrammarObjectSequence rightPart();

		/*
		Appends given symbol to rule's right part.
		*/
		void appendSymbol(GrammarObject * symbol);
	private:
		static bool stringHasNTerminal(string str);
	private:
		NonTerminal * m_pLeftPart;
		GrammarObjectSequence mRightPart;
	};

	/*
	Groups rules with similar left part.
	*/
	class RuleGroup
	{
	public:
		RuleGroup(NonTerminal * nterm);
		~RuleGroup();
		size_t ruleCount();
		Rule * rule(size_t index);
		NonTerminal * nterm();
		void add(Rule * new_rule);
		size_t ruleIndex(Rule * rule);
	private:
		NonTerminal * m_pLeftPart;
		vector<Rule*> mRules;
	};

	/*
	Constructs grammar from given BNF description of language.
	- all non-terminals must be limiting with '<' '>'
	- all production symbols must be '::='
	- language DOESN'T contain '<' or '>'; replace it to _gt_ and _le_ if so
	- empty symbol is _eps_
	*/
	UGrammar(const char * language_bnf_description);
	UGrammar();
	~UGrammar();

	/*
	Finds and returns element with given full name.
	*/
	Terminal * terminal(const char * full_terminal_name);
	/*
	Adds new terminal to grammar.
	*/
	void addTerminal(Terminal * new_terminal);
	Terminal * createTerminal(const char * terminal_full_name);
	Terminal * createOrFindTerm(const char * term_full_name);
	vector<GrammarObject*> & terminals();

	NonTerminal * nterminal(const char * full_non_terminal_name);
	void addNTerminal(NonTerminal * new_nterminal);
	NonTerminal * createNTerminal(const char * non_terminal_full_name);
	NonTerminal * createOrFindNTerm(const char * nterm_full_name);
	vector<GrammarObject*> & nterminals();

	/*
	Finds rule group with given nterm in left part and returns it.
	If not found - 0 is returned.
	*/
	RuleGroup * ruleGroup(NonTerminal * nterm);

	/*
	Prints the grammar and all it's rules in BNF-representation.
	*/
	void print();
	void printFirstFollow();

	/*
	Checks, if the grammar belongs to given class.
	*/
	bool isLL1();

	/*
	Routines for LL1 checking.
	Builds list of predecessor symbols for given sequence.
	*/
	vector<Terminal*> getPredecessorSymbols(GrammarObjectSequence sequence, bool * unknown);
	vector<UGrammar::Terminal*> getPredecessorSymbols(NonTerminal * nterm);
	static bool containsEmtyTerminal(vector<Terminal*> arr);
	vector<Terminal*> first(GrammarObjectSequence seq);

	bool canBeReducedToEps(GrammarObjectSequence seq);
	/*
	Returns true if NTerm->*_EPS_.
	*/
	bool canBeReducedToEps(GrammarObject * symbol);
	vector<Terminal*> follow(NonTerminal * nterm);

	void removeEPS(vector<Terminal*> & arr);

	size_t ruleIndex(Rule * rule);


	/*
	Creates rule from given data "nterm_name" -> "a1", "a2", ..., 0.
	Zero must be the last param to stop searching.
	*/
	void createRule(const char * nterm_name, ...);
private:
	/*
	Adds rule to grammar and saves it to correct group. If group
	doesn't exist - creates it.
	*/
	void addRule(Rule * new_rule);
	static GrammarObject * objectWithFullName(vector<GrammarObject*> container, const char * full_name);
private:
	vector<RuleGroup*> mRuleGroups;
	vector<GrammarObject*> mTerminals, mNonTerminals;
};

#endif