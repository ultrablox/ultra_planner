#include "UGrammar.h"
#include <core/utils/helpers.h>
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <regex>

//==========================ComplexNamesObject========================
ComplexNamedObject::ComplexNamedObject(const char * full_name)
{
//	sprintf(mFullName, "%s", full_name);
}

const char * ComplexNamedObject::name()
{
	return mFullName;
}
//=======================GrammarObject==================

UGrammar::GrammarObject::GrammarObject(const char * _name)
	:ComplexNamedObject(_name)
{

}

void UGrammar::GrammarObject::print()
{
	bool needs_brackets = (type() == Type::NonTerminal);

	if(needs_brackets)
		printf("<");

	printf("%s", name());

	if(needs_brackets)
		printf(">");
}

//=======================GrammarObjectSequence==================
UGrammar::GrammarObjectSequence::GrammarObjectSequence()
	:vector<GrammarObject*>()
{

}

UGrammar::GrammarObjectSequence::GrammarObjectSequence(GrammarObject * obj)
	:vector<GrammarObject*>()
{
	push_back(obj);
}

void UGrammar::GrammarObjectSequence::print()
{
	for(auto i = begin(); i != end(); ++i)
	{
		(*i)->print();
	}
}

bool UGrammar::GrammarObjectSequence::contains(GrammarObject * object)
{
	auto p = std::find(begin(), end(), object);
	return p != end();
}

//=======================Terminal==================
UGrammar::Terminal::Terminal(const char * _name)
	:GrammarObject(_name)
{

}

int UGrammar::Terminal::type()
{
	return GrammarObject::Type::Terminal;
}

bool UGrammar::Terminal::isEmpty()
{
	return !strcmp(name(), SPECIAL_SYMBOL_EMPTY);
}

//=======================Non-terminal==================
UGrammar::NonTerminal::NonTerminal(const char * _name)
	:GrammarObject(_name)
{

}

int UGrammar::NonTerminal::type()
{
	return GrammarObject::Type::NonTerminal;
}

//=======================Rule=========================
UGrammar::Rule::Rule(NonTerminal * nterm)
	:m_pLeftPart(nterm)
{

}

UGrammar::Rule::Rule(const char * left_part_bnf, const char * right_part_bnf, UGrammar * grammar)
{
//	printf("Building rule from string: %s\n", rule_bnf_description);

	

	string left_part = trim(string(left_part_bnf));

	/*
	====================
	Analize left part of the rule
	====================
	*/
	string nterm_name = removeBrackets(left_part);
	NonTerminal * left_nterm = grammar->nterminal(nterm_name.c_str());
	if(!left_nterm)
	{
		left_nterm = new NonTerminal(nterm_name.c_str());
		grammar->addNTerminal(left_nterm);
	}

	m_pLeftPart = left_nterm;

	/*
	====================
	And right.
	====================
	*/

	string right_part = trim(string(right_part_bnf));

	//Loop, while string has non-terminals
	while(stringHasNTerminal(right_part))
	{
		//Get nterm beginning symb
		std::size_t nterm_begin = right_part.find("<");

		
		if(nterm_begin == 0)
		{
			right_part.erase(0, 1);
		}
		else	//If it is not 0 - the terminal is before
		{
			//Get the terminal
			string term_name = right_part.substr(0, nterm_begin);
			Terminal * term = grammar->terminal(term_name.c_str());
			if(!term)
				term = grammar->createTerminal(term_name.c_str());
			mRightPart.push_back(term);

			//Remove terminal from string
			right_part.erase(0, nterm_begin + 1);
		}

		std::size_t nterm_end = right_part.find(">");
		string nterm_name = right_part.substr(0, nterm_end);
		
		NonTerminal * nterm = grammar->nterminal(nterm_name.c_str());
		if(!nterm)
			nterm = grammar->createNTerminal(nterm_name.c_str());
		mRightPart.push_back(nterm);

		right_part.erase(0, nterm_end + 1);

		//And then get the non-terminal
	}

	if(right_part.length() > 0)		//If something lost - it is terminal
	{
		Terminal * term = grammar->terminal(right_part.c_str());
		if(!term)
			term = grammar->createTerminal(right_part.c_str());
		mRightPart.push_back(term);
	}
//	print();
}

void UGrammar::Rule::print()
{
	m_pLeftPart->print();
	printf(" ::= ");

	for(auto i = mRightPart.begin(); i != mRightPart.end(); ++i)
	{
		(*i)->print();
	}

	printf("\n");
}

UGrammar::NonTerminal * UGrammar::Rule::nterm()
{
	return m_pLeftPart;
}

UGrammar::GrammarObjectSequence UGrammar::Rule::rightPart()
{
	return mRightPart;
}

void UGrammar::Rule::appendSymbol(GrammarObject * symbol)
{
	mRightPart.push_back(symbol);
}

bool UGrammar::Rule::stringHasNTerminal(string str)
{
	std::size_t f_ls = str.find("<");
	if(f_ls == std::string::npos)
		return false;

	std::size_t f_gt = str.find(">", f_ls);
	if(f_gt == std::string::npos)
		return false;

	return true;
}

//===================RuleGroup=============================
UGrammar::RuleGroup::RuleGroup(NonTerminal * nterm)
	:m_pLeftPart(nterm)
{

}

UGrammar::RuleGroup::~RuleGroup()
{
	for(auto i = mRules.begin(); i != mRules.end(); ++i)
		delete (*i);
	mRules.clear();
}

size_t UGrammar::RuleGroup::ruleCount()
{
	return mRules.size();
}

UGrammar::Rule * UGrammar::RuleGroup::rule(size_t index)
{
	return mRules[index];
}

UGrammar::NonTerminal * UGrammar::RuleGroup::nterm()
{
	return m_pLeftPart;
}

void UGrammar::RuleGroup::add(Rule * new_rule)
{
	mRules.push_back(new_rule);
}

size_t UGrammar::RuleGroup::ruleIndex(Rule * rule)
{
	auto r_it = std::find(mRules.begin(), mRules.end(), rule);
	return std::distance(mRules.begin(), r_it);
}

//===================UGrammar==========================
UGrammar::UGrammar(const char * language_bnf_description)
{
	vector<string> rule_strings;
	string_split(rule_strings, language_bnf_description, "\n");

	for(auto i = rule_strings.begin(); i != rule_strings.end(); ++i)
	{
		//Split string by ::=
		vector<string> rule_parts;
		string_split(rule_parts, i->c_str(), "::=");

		if(rule_parts.size() != 2)
		{
			printf("Incorrect grammar rule: %s\n", i->c_str());
			continue;
		}
		//Split by 'or' | symbol - each part must produce one rule
		vector<string> or_branches;
		string_split(or_branches, rule_parts[1].c_str(), "|");

		for(auto b = or_branches.begin(); b != or_branches.end(); ++b)
		{
			Rule * rule = new Rule(rule_parts[0].c_str(), (*b).c_str(), this);
			addRule(rule);
		}
	}
}

UGrammar::UGrammar()
{

}

UGrammar::~UGrammar()
{
	//Delete rules
	for(auto i = mRuleGroups.begin(); i != mRuleGroups.end(); ++i)
		delete (*i);
	mRuleGroups.clear();

	for(auto i = mTerminals.begin(); i != mTerminals.end(); ++i)
		delete (*i);
	mTerminals.clear();

	for(auto i : mNonTerminals)
		delete i;
	mNonTerminals.clear();
}

UGrammar::Terminal * UGrammar::terminal(const char * full_terminal_name)
{
	return (Terminal*)(objectWithFullName(mTerminals, full_terminal_name));
}

void UGrammar::addTerminal(UGrammar::Terminal * new_terminal)
{
	mTerminals.push_back(new_terminal);
}

UGrammar::Terminal * UGrammar::createTerminal(const char * terminal_full_name)
{
	Terminal * new_term = new Terminal(terminal_full_name);
	addTerminal(new_term);
	return new_term;
}

UGrammar::Terminal * UGrammar::createOrFindTerm(const char * term_full_name)
{
	Terminal * _term = terminal(term_full_name);

	if(!_term)
	{
		_term = new Terminal(term_full_name);
		addTerminal(_term);
	}

	return _term;
}

vector<UGrammar::GrammarObject*> & UGrammar::terminals()
{
	return mTerminals;
}

UGrammar::NonTerminal * UGrammar::nterminal(const char * full_non_terminal_name)
{
	return (NonTerminal*)(objectWithFullName(mNonTerminals, full_non_terminal_name));
}

void UGrammar::addNTerminal(UGrammar::NonTerminal * new_nterminal)
{
	mNonTerminals.push_back(new_nterminal);
}

UGrammar::NonTerminal * UGrammar::createNTerminal(const char * non_terminal_full_name)
{
	NonTerminal * new_nterm = new NonTerminal(non_terminal_full_name);
	addNTerminal(new_nterm);
	return new_nterm;
}

UGrammar::NonTerminal * UGrammar::createOrFindNTerm(const char * nterm_full_name)
{
	NonTerminal * _nterm = nterminal(nterm_full_name);

	if(!_nterm)
	{
		_nterm = new NonTerminal(nterm_full_name);
		addNTerminal(_nterm);
	}

	return _nterm;
}

vector<UGrammar::GrammarObject*> & UGrammar::nterminals()
{
	return mNonTerminals;
}

UGrammar::RuleGroup * UGrammar::ruleGroup(UGrammar::NonTerminal * nterm)
{
	for(auto i = mRuleGroups.begin(); i != mRuleGroups.end(); ++i)
	{
		if((*i)->nterm() == nterm)
			return (*i);
	}

	return 0;
}

void UGrammar::print()
{
	//Non-terminals
	printf("===========================Non-terminals========================\n");
	for(auto i = mNonTerminals.begin(); i != mNonTerminals.end(); ++i)
	{
		printf("%s", (*i)->name());
		printf("\n");
	}
	printf("\n");

	//Terminals
	printf("=============================Terminals=========================\n");
	for(auto i = mTerminals.begin(); i != mTerminals.end(); ++i)
	{
		(*i)->print();
		printf("\n");
	}
	printf("\n");

	//Rules
	printf("===========================Rules (%ld groups)===========================\n", mRuleGroups.size());

	int rule_counter = 1;
	for(auto i = mRuleGroups.begin(); i != mRuleGroups.end(); ++i)
	{

		for(size_t j = 0; j < (*i)->ruleCount(); ++j)
		{
			printf("%ld. ", rule_counter++);
			(*i)->rule(j)->print();
		}
	}
}

void UGrammar::printFirstFollow()
{
	for(auto i = mNonTerminals.begin(); i != mNonTerminals.end(); ++i)
	{
		printf("FIRST(%s) = {", (*i)->name());
		auto firsts = first((*i));
		for(auto i = firsts.begin(); i != firsts.end(); ++i)
		{
			if(i != firsts.begin())
				printf(", ");
			(*i)->print();
		}
		printf("},\t\t");

		printf("FOLLOW(%s) = {", (*i)->name());

		auto follow_s = follow((NonTerminal*)(*i));

		for(auto i = follow_s.begin(); i != follow_s.end(); ++i)
		{
			if(i != follow_s.begin())
				printf(", ");
			(*i)->print();
		}

		printf("}\n");
	}
}

bool UGrammar::isLL1()
{
	for(auto g = mRuleGroups.begin(); g != mRuleGroups.end(); ++g)
	{
		RuleGroup * group = *g;

		for(int r = 0; r < group->ruleCount(); ++r)
		{
			Rule * rule = group->rule(r);
			printf("Getting predecessor for rule");
			rule->print();

			bool unknown;
			auto predecessors = getPredecessorSymbols(rule->rightPart(), &unknown);

			printf(" => {");
			if(unknown)
				printf("####");
			else
			{
				for(auto p = predecessors.begin(); p != predecessors.end(); ++p)
					printf("%s, ", (*p)->name());
			}
			printf("}\n");
		}
	}

	return false;
}

vector<UGrammar::Terminal*> UGrammar::getPredecessorSymbols(GrammarObjectSequence sequence, bool * unknown)
{
	auto s = sequence.begin();
	if((*s)->type() == GrammarObject::Type::Terminal)
	{
		if(((Terminal*)(*s))->isEmpty())
		{
			if(sequence.size() == 1)	//Unknown situation
			{
				if(unknown)
					*unknown = true;

				return vector<UGrammar::Terminal*>();
			}
			else
			{
				GrammarObjectSequence new_seq = sequence;
				new_seq.erase(new_seq.begin());

				return getPredecessorSymbols(new_seq, unknown);
			}
		}
		else
		{
			if(unknown)
				*unknown = false;

			vector<UGrammar::Terminal*> result;
			result.push_back((Terminal*)(*s));
			return result;
		}
	}
	else
	{
		auto current_predecessors = getPredecessorSymbols((NonTerminal*)(*s));
		return vector<UGrammar::Terminal*>();
	}
}

vector<UGrammar::Terminal*> UGrammar::getPredecessorSymbols(NonTerminal * nterm)
{
	vector<UGrammar::Terminal*> result;

	return result;
}

bool UGrammar::containsEmtyTerminal(vector<Terminal*> arr)
{
	return false;
}

vector<UGrammar::Terminal*> UGrammar::first(GrammarObjectSequence seq)
{
	
	GrammarObject * symb = *seq.begin();

	vector<UGrammar::Terminal*> result;

	if(seq.size() == 1)
	{
		if(symb->type() == GrammarObject::Type::Terminal)
		{
			Terminal * term = (Terminal*)symb;
			if(!term->isEmpty())
			{
				result.push_back(term);
			}
		}
		else
		{
			NonTerminal * nterm = (NonTerminal*)symb;

			RuleGroup * rules = ruleGroup(nterm);

			for(size_t i = 0; i < rules->ruleCount(); ++i)
			{
				Rule * rule = rules->rule(i);

				auto first_TAIL = first(rule->rightPart());

				result.insert(result.end(), first_TAIL.begin(), first_TAIL.end());
			}
		}
	}
	else
	{
		//We have ABCDxxxx sequence

		//Add first(A) to result
		auto first_A = first(GrammarObjectSequence(symb));

		if(symb->type() == GrammarObject::Type::Terminal)
			removeEPS(first_A);

		result.insert(result.end(), first_A.begin(), first_A.end());

		//If A can be EPS - add first(BCD...)
		if(canBeReducedToEps(symb) && (seq.size() > 1))
		{
			GrammarObjectSequence tail_seq;
			tail_seq.insert(tail_seq.end(), seq.begin() + 1, seq.end());
		
			auto first_TAIL = first(tail_seq);
			result.insert(result.end(), first_TAIL.begin(), first_TAIL.end());

		}

	}


	auto eps_pos = std::find(result.begin(), result.end(), terminal(SPECIAL_SYMBOL_EMPTY));
	if(eps_pos != result.end())
		result.erase(eps_pos);

	if(canBeReducedToEps(seq))
		result.push_back(terminal(SPECIAL_SYMBOL_EMPTY));

	return result;
}

bool UGrammar::canBeReducedToEps(GrammarObjectSequence seq)
{
	for(auto s = seq.begin(); s != seq.end(); ++s)
	{
		if(!canBeReducedToEps(*s))
			return false;
	}

	return true;
}

bool UGrammar::canBeReducedToEps(GrammarObject * symbol)
{
	if(symbol->type() == GrammarObject::Type::Terminal)	//Terminal
	{
		Terminal * term = (Terminal*)symbol;
		if(term->isEmpty())
			return true;
		else
			return false;
	}
	else	//Non-terminal
	{
		NonTerminal * nterm = (NonTerminal*)symbol;

		//Get all rules
		RuleGroup * rules = ruleGroup(nterm);

		//Find if there is any rule that can be reduces to eps

		for(size_t i = 0; i < rules->ruleCount(); ++i)
		{
			Rule * rule = rules->rule(i);

			auto right_part = rule->rightPart();

			bool rule_can_be_reduced_to_eps = true;

			//Check, that there are only EPS terminals in right part
			for(auto s = right_part.begin(); s != right_part.end(); ++s)
			{
				if((*s)->type() == GrammarObject::Type::Terminal)
				{
					Terminal * term = (Terminal*)(*s);
					if(!term->isEmpty())
					{
						rule_can_be_reduced_to_eps = false;
						break;
					}
				}
			}

			if(!rule_can_be_reduced_to_eps)
				continue;

			//Check that all non-terminals can be also reduced to EPS
			for(auto s = right_part.begin(); s != right_part.end(); ++s)
			{
				if((*s)->type() == GrammarObject::Type::NonTerminal)
				{
					NonTerminal * nterm = (NonTerminal*)(*s);
					if(!canBeReducedToEps(nterm))
					{
						rule_can_be_reduced_to_eps = false;
						break;
					}
				}
			}

			if(rule_can_be_reduced_to_eps)
				return true;
		}

		return false;
	}
}

vector<UGrammar::Terminal*> UGrammar::follow(NonTerminal * nterm)
{
	//Find rules with nterm in right part

	vector<Rule*> rules;

	for(auto g = mRuleGroups.begin(); g != mRuleGroups.end(); ++g)
	{
		for(size_t r = 0; r < (*g)->ruleCount(); ++r)
		{
			Rule * rule = (*g)->rule(r);
			if(rule->rightPart().contains(nterm))
				rules.push_back(rule);
		}
	}
	
	vector<UGrammar::Terminal*> result;

	for(auto r = rules.begin(); r != rules.end(); ++r)
	{
		//(*r)->print();
		//printf("\n");
		auto right_part = (*r)->rightPart();

		auto nterm_pos = std::find(right_part.begin(), right_part.end(), nterm);
		
		GrammarObjectSequence right_seq;
		right_seq.insert(right_seq.end(), nterm_pos + 1, right_part.end());

		if(canBeReducedToEps(right_seq) || (right_seq.size() == 0))	//xxxxxxxxAyy where yy can be reduced to eps OR xxxxxxxA
		{
			if(right_seq.size() > 0)
			{
				auto tail_first = first(right_seq);
				removeEPS(tail_first);
				result.insert(result.end(), tail_first.begin(), tail_first.end());
			}

			if((*r)->nterm() != nterm) //Ignore rule A -> xxxxxxA - they will not help us
			{
				auto left_nterm_follow = follow((*r)->nterm());
				result.insert(result.end(), left_nterm_follow.begin(), left_nterm_follow.end());
			}
		}
		else
		{
			//xxxxxAxxxxx
			auto tail_first = first(right_seq);
			removeEPS(tail_first);
			result.insert(result.end(), tail_first.begin(), tail_first.end());
		}
	}

	//Remove repeats
	std::sort(result.begin(), result.end());
	auto it = std::unique(result.begin(), result.end());
	result.resize(std::distance(result.begin(), it));

	return result;
}

void UGrammar::removeEPS(vector<Terminal*> & arr)
{
	auto eps_pos = std::find(arr.begin(), arr.end(), terminal(SPECIAL_SYMBOL_EMPTY));
	if(eps_pos != arr.end())
		arr.erase(eps_pos);
}

size_t UGrammar::ruleIndex(Rule * rule)
{
	size_t result = 0;
	for(auto gr = mRuleGroups.begin(); gr != mRuleGroups.end(); ++gr)
	{
		if((*gr)->nterm() == rule->nterm())
		{
			return result + (*gr)->ruleIndex(rule);
		}
		else
			result += (*gr)->ruleCount();
	}
    
    return result;
}

void UGrammar::createRule(const char * nterm_name, ...)
{
	string nt_name = removeBrackets(nterm_name);
	NonTerminal * left_nterm = createOrFindNTerm(nt_name.c_str());


	Rule * rule = new Rule(left_nterm);

	va_list args;
	va_start(args, nterm_name);

	std::regex nterm_regex("<.+>");
	while(const char * element_name = va_arg(args, const char *))
	{
		 if(std::regex_match(element_name, nterm_regex)) //Check if current element is nterminal
		 {
			 printf("Nterm %s.\n", element_name);
			 string new_nt_name = removeBrackets(element_name);
			 NonTerminal * new_nterm = createOrFindNTerm(new_nt_name.c_str());
			 rule->appendSymbol(new_nterm);
		 }
		 else //Or terminal
		 {
			 Terminal * new_term = createOrFindTerm(element_name);
			 rule->appendSymbol(new_term);
		 }
	}
	
	va_end(args);

	addRule(rule);
}

void UGrammar::addRule(UGrammar::Rule * new_rule)
{
	RuleGroup * group = ruleGroup(new_rule->nterm());
	if(!group)
	{
		group = new RuleGroup(new_rule->nterm());
		mRuleGroups.push_back(group);
	}

	group->add(new_rule);
}

UGrammar::GrammarObject * UGrammar::objectWithFullName(vector<GrammarObject*> container, const char * full_name)
{
	for(auto i = container.begin(); i != container.end(); ++i)
	{
		if(!strcmp((*i)->name(), full_name))
			return *i;
	}

	return 0;
}
