#include "UParser.h"
#include "../ULanguage.h"
#include "../UPDDLPredicates.h"
#include "../UPDDLDomain.h"
#include "../UPDDLAction.h"
#include <core/transition_system/UTransitionSystem.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <stdarg.h>
#include <assert.h>

using namespace std;

//===================SyntaxTable==========================
UParser::SyntaxTable::SyntaxTable(vector<UGrammar::GrammarObject*> terms, vector<UGrammar::GrammarObject*> nterms, UGrammar * grammar)
	:m_pData(0), mTerminals(terms), mNTerminals(nterms), m_pGrammar(grammar)
{
	size_t total_size = sizeof(UGrammar::Rule*) * terms.size() * nterms.size();
	m_pData = (UGrammar::Rule**)malloc(total_size);
	memset(m_pData, 0, total_size);
}

UParser::SyntaxTable::~SyntaxTable()
{
	free(m_pData);
}

UGrammar::Rule *& UParser::SyntaxTable::cell(UGrammar::Terminal * term, UGrammar::NonTerminal * nterm)
{
	//Row - terminals

	auto term_it = std::find(mTerminals.begin(), mTerminals.end(), term);
	size_t x = std::distance(mTerminals.begin(), term_it);

	auto nterm_it = std::find(mNTerminals.begin(), mNTerminals.end(), nterm);
	size_t y = std::distance(mNTerminals.begin(), nterm_it);

	return m_pData[y * mTerminals.size() + x];
}

void UParser::SyntaxTable::setRule(UGrammar::Terminal * term, UGrammar::NonTerminal * nterm, UGrammar::Rule * rule)
{
	auto prev_rule = cell(term, nterm);
	if(prev_rule != 0)
	{
		if(prev_rule != rule)
		{
			printf("Error building table in cell {%s, %s} - not LL1.\n", term->name(), nterm->name());
			prev_rule->print();
			rule->print();
		}
	}

	cell(term, nterm) = rule;
}

void UParser::SyntaxTable::print()
{
	printf("========================SYNTAX TABLE========================\n");

	printf("####\t");
	for(int x = 0; x < mTerminals.size(); ++x)
	{
		printf("%s\t", mTerminals[x]->name());
	}
	printf("\n");

	for(int y = 0; y < mNTerminals.size(); ++y)
	{
		printf("%s\t", mNTerminals[y]->name());

		for(int x = 0; x < mTerminals.size(); ++x)
		{
			UGrammar::Rule * rule = cell((UGrammar::Terminal*)mTerminals[x], (UGrammar::NonTerminal*)mNTerminals[y]);
			if(rule)
			{
				printf("%ld\t", m_pGrammar->ruleIndex(rule)+1);
			}
			else
				printf("---\t");
		}
		printf("\n");
	}

	printf("\n");
}


//=================================UParser=====================
UParser::UParser(ULanguage * _language)
	:m_pSyntaxTable(0), m_pLanguage(_language)
{
	//buildSyntaxTable();
}

void UParser::buildSyntaxTable()
{
	UGrammar * grammar = m_pLanguage->grammar();

	auto nterms = grammar->nterminals(), terms = grammar->terminals();
	m_pSyntaxTable = new SyntaxTable(terms, nterms, grammar);

	for(auto nt = nterms.begin(); nt != nterms.end(); ++nt)
	{
		auto nterm = (UGrammar::NonTerminal*)*nt;
		auto rg = grammar->ruleGroup(nterm);

		for(int r = 0; r < rg->ruleCount(); ++r)
		{
			auto rule = rg->rule(r);

			//=======Step 2============
			auto firsts = grammar->first(rule->rightPart());

			for(auto f = firsts.begin(); f != firsts.end(); ++f)
			{
				m_pSyntaxTable->setRule(*f, nterm, rule);
			}

			//===========Step 3==========

			auto eps_pos = std::find(firsts.begin(), firsts.end(), grammar->terminal(SPECIAL_SYMBOL_EMPTY));
			if(eps_pos != firsts.end())
			{
				auto follows = grammar->follow(nterm);

				for(auto f = follows.begin(); f != follows.end(); ++f)
				{
					m_pSyntaxTable->setRule(*f, nterm, rule);
				}
			}
		}
	}

	m_pSyntaxTable->print();
}

void printError(InputSymbol symbol)
{
	printf("Parsing error at (%d, %d). Lexem %s\n", symbol.source_row, symbol.source_column, symbol.lexem->name.c_str());
}

bool checkObligatoryLexem(LexerTape & lexer_tape, const std::string & lexem_name, bool throw_exc = false)
{
	bool r = (lexer_tape.currentName() == lexem_name);
	if(throw_exc && (!r))
		throw parser_exception(lexer_tape.currentSymbol(), lexer_tape.index);

	if(r)
		lexer_tape.next();

	return r;
}

bool checkObligatoryLexems(LexerTape & lexer_tape, bool throw_exc, ...)
{
	int start_index = lexer_tape.index;

	va_list args;
	va_start(args, throw_exc);

	while(const char * arg_data = va_arg(args, const char*))
	{
		bool r = checkObligatoryLexem(lexer_tape, string(arg_data), throw_exc);
		if(!r)
		{
			lexer_tape.index = start_index;
			return false;
		}
	}

	va_end(args);

	return true;
}


Lexem * checkLexemType(LexerTape & lexer_tape, const Lexem::Type type, bool throw_exc = true)
{
	auto cur_lex = lexer_tape.current();
	if(cur_lex->type == type)
	{
		lexer_tape.next();
		return cur_lex;
	}
	else
	{
		if(throw_exc)
			throw parser_exception(lexer_tape.currentSymbol(), lexer_tape.index);
		return nullptr;
	}
}

bool checkObligatoryLexem(LexerTape lexed_symbol, size_t & index, const std::string & lexem_name)
{
    Lexem * lexem = lexed_symbol[index].lexem;
    bool result = lexem->name == lexem_name;
    if(result)
        ++index;
    return result;
}

bool checkLexemType(LexerTape lexed_symbol, size_t & index, Lexem::Type type, Lexem *& _lexem)
{
    Lexem * lexem = lexed_symbol[index].lexem;
    bool result = (lexem->type == type);
    if(result)
    {
        ++index;
        _lexem = lexem;
    }
    return result;
}

#define CHECK_OBLIGATORY_LEXEM(LEXEM_NAME) if(!checkObligatoryLexem(lexed_symbols, index, LEXEM_NAME)){throw parser_exception(lexed_symbols[index], index);}
#define CHECK_OBLIGATORY_LEXEM_TYPE(LEXEM_TYPE, pLEXEM) if(!checkLexemType(lexed_symbols, index, LEXEM_TYPE, pLEXEM)){throw parser_exception(lexed_symbols[index], index);}
//#define PARSER_START size_t index = start_index; bool bResult = true;
//#define PARSER_END if(bResult) {start_index = index;}
#define PARSER_START size_t index = start_index; bool bResult = false;
#define PARSER_END start_index = index;

/*
UPDDLDomain * UParser::parseDomainSafe(LexTree & lex_tree)
{
    UPDDLDomain * domain(nullptr);
    
    try
    {
		domain = parseDomain(&lex_tree.root);
    }
    catch(parser_exception & e)
    {
        cout << e.description() << "\n";
    }

    return domain;
}
*/
bool check_symbol(LexTree::Item * item, const std::string & lex_name)
{
	if(item->type() != LexTree::Item::Type::Symbol)
		return false;
	
	auto sym_it = item->asSymbol();
	return (sym_it->text() == lex_name);
}

UPDDLDomain * UParser::parseDomain(LexTree::TreeItem * domain_tree, const UPDDLDomain * _domain)
{
    /*
     <domain> ::= (define (domain <name>)
         [<require-def>]
         [<types-def>]:typing
         [<constants-def>]
         [<predicates-def>]
         [<functions-def>]:fluents
         [<constraints>]
         <structure-def>*)
    */
    
	auto it = domain_tree->children.begin();

	if(!check_symbol(*it, "define"))
        throw parser_exception("'define' lexem not found.");

    auto domain_branch = (*(++it))->asTree();
    
    if(!check_symbol(domain_branch->children[0], "domain"))
        throw parser_exception("'domain' name branch invalid.");
    auto domain_name = domain_branch->children[1]->asSymbol()->text();
    
    UPDDLDomain * domain = new UPDDLDomain(domain_name);
    
    parseRequirements((*(++it))->asTree(), domain);
    parseTypes((*(++it))->asTree(), domain);
    

	//Functions, constants, predicats
    for(++it; it != domain_tree->children.end(); ++it)
    {
        auto cur_subtree = (*it)->asTree();
        auto main_lexem = cur_subtree->children[0]->asSymbol()->lexem();
        
        if(main_lexem->name == ":predicates")
            parsePredicates(cur_subtree, domain);
        else if(main_lexem->name == ":functions")
            parseFunctions(cur_subtree, domain);
		else if(main_lexem->name == ":constants")
		{
			parseObjects(cur_subtree, domain, domain);
		}
        else
        {
            break;
        }

    }
    
	//Now, action, processes, durative-actions
	while(it != domain_tree->children.end())
	{
		auto cur_subtree = (*it)->asTree();
        auto main_lexem = cur_subtree->children[0]->asSymbol()->lexem();

		if(main_lexem->name == ":action")
		{
			auto action = parseAction(cur_subtree, domain);
			domain->actions.push_back(action);
		}
		else
		{
			throw parser_exception("Unsupported action type.");
		}

		++it;
	}

    return domain;
}

UPDDLProblem * UParser::parseProblem(LexTree::TreeItem * problem_tree, const UPDDLDomain * domain)
{
	UPDDLProblem * problem = new UPDDLProblem();

	//Objects
	parseObjects(problem_tree->children[3]->asTree(), domain, problem);

	//Merge objects with constantsdomainz
	problem->objects.insert(problem->objects.end(), domain->constants.begin(), domain->constants.end());

	//Initial state
	parseInitialState(problem_tree->children[4]->asTree(), problem, domain);

	//Goal
	parseGoals(problem_tree->children[5]->asTree(), problem, domain);
	
	return problem;
}

void UParser::parseRequirements(LexTree::TreeItem * req_subtree, UPDDLDomain * domain)
{
	/*
	<require-def> ::= (:requirements <require-key>+)
	*/
    
    if(!check_symbol(req_subtree->children[0], ":requirements"))
        throw parser_exception("'requirements' lexem not found.");
    
    for(int r = 1; r < req_subtree->children.size(); ++r)
    {
        auto req = req_subtree->children[r]->asSymbol();
        
        UPDDLRequierment::Type type = UPDDLRequierment::Type::Unknown;
        if(req->text() == ":strips")
            type = UPDDLRequierment::Type::Strips;
        else if(req->text() == ":typing")
            type = UPDDLRequierment::Type::Typing;
        else if(req->text() == ":negative-preconditions")
            type = UPDDLRequierment::Type::NegativePreconditions;
        else if(req->text() == ":disjunctive-preconditions")
            type = UPDDLRequierment::Type::DisjunctivePreconditions;
        else if(req->text() == ":equality")
            type = UPDDLRequierment::Type::Typing;
		else if(req->text() == ":numeric-fluents")
            type = UPDDLRequierment::Type::NumericFluents;
		else if(req->text() == ":action-costs")
			type = UPDDLRequierment::Type::ActionCosts;
		else if(req->text() == ":durative-actions")
			type = UPDDLRequierment::Type::DurativeActions;
		else if(req->text() == ":derived-predicates")
			type = UPDDLRequierment::Type::DerivedPredicates;
        
		domain->requerments.push_back(type);
    }
}

void UParser::parseTypes(LexTree::TreeItem * req_subtree, UPDDLDomain * domain)
{
	/*
	<types-def> ::= (:types <typed list (name)>)
	*/

    TypedList result_types(parseTypedList(req_subtree->children.begin() + 1, req_subtree->children.end(), Lexem::Type::Identifier, true));

	/*checkObligatoryLexems(lexer_tape, true, "(", ":types", nullptr);
		checkObligatoryLexem(lexer_tape, ")", true);
	
*/
	while(!result_types.data.empty())
	{
		auto i = result_types.data.begin();
		while(i != result_types.data.end())
		{
			if(domain->typeTree.addTypes(i->second, i->first))
			{
				result_types.data.erase(i);
				break;
			}
			else
				++i;
		}
	}
}

void UParser::parsePredicates(LexTree::TreeItem * pred_subtree, UPDDLDomain * domain)
{
	/*
	<predicates-def> ::= (:predicates <atomic formula skeleton>+)
	*/

    for(auto pit = pred_subtree->children.begin() + 1; pit != pred_subtree->children.end(); ++pit)
    {
        AtomicFormulaSkeleton formula = parseAtomicFormulaSkeleton((*pit)->asTree());
        UPDDLPredicate * new_pred = domain->createPredicate(formula.name_lexem->name);
        new_pred->loadParams(formula.typed_list, domain->typeTree);
    }
}

void UParser::parseFunctions(LexTree::TreeItem * funcs_subtree, UPDDLDomain * domain)
{
	/*
	(:fluents) <functions-def> ::= (:functions <function typed list (atomic function skeleton)>)
	*/
	
	auto it = funcs_subtree->children.begin() + 1;

	while(it != funcs_subtree->children.end())
	{
		AtomicFormulaSkeleton formula = parseAtomicFormulaSkeleton((*it)->asTree());
		++it;

		//'-'
		++it;

		//Function type (number)
		++it;

		//Create function
		UPDDLFunction * function = new UPDDLFunction(formula.name_lexem->name);
		function->loadParams(formula.typed_list, domain->typeTree);
		domain->functions.push_back(function);
	}
}

TypedList UParser::parseTypedList(std::vector<LexTree::Item*>::iterator first, std::vector<LexTree::Item*>::iterator last, Lexem::Type item_lexem_type, bool ext_typing)
{
	/*
	<typed list (x)> ::= x*
	(:typing) <typed list (x)> ::= x+ - <type> <typed list(x)>
	*/
    
    /*
     <type> ::= <primitive-type>
     <primitive-type> ::= <name>
     <primitive-type> ::= object
     */

    //PARSER_START
    
    TypedList result;
    for(auto it = first; it != last; ++it)
    {
        vector<string> sub_types;
        
        while((*it)->asSymbol()->text() != "-")
        {
            Lexem * type_lexem = (*it)->asSymbol()->lexem();
            if(type_lexem->type != item_lexem_type)
                throw parser_exception("Type lexem invalid type.");
            
            sub_types.push_back(type_lexem->name);
            ++it;
        }
        
        ++it;
        
        Lexem * base_type_lexem = (*it)->asSymbol()->lexem();
        
        auto & dest = result[base_type_lexem->name];
		dest.insert(dest.end(), sub_types.begin(), sub_types.end());
        
    }

    return result;
}

AtomicFormulaSkeleton UParser::parseAtomicFormulaSkeleton(LexTree::TreeItem * af_subtree)
{
	/*
	<atomic formula skeleton> ::= (<predicate> <typed list (variable)>)
	*/

	AtomicFormulaSkeleton result;
    result.name_lexem = af_subtree->children[0]->asSymbol()->lexem();
	result.typed_list = parseTypedList(af_subtree->children.begin() + 1, af_subtree->children.end(), Lexem::Type::Variable, true);
	
	return result;
}

UPDDLAction * UParser::parseAction(LexTree::TreeItem * act_subtree, const UPDDLDomain * domain)
{
	/*
	<structure-def> ::= <action-def>
	(:durative−actions) <structure-def> ::= <durative-action-def>
	(:derived−predicates) <structure-def> ::= <derived-def>
	*/

	auto name_lex = act_subtree->children[1]->asSymbol()->lexem();

	UPDDLAction * result = new UPDDLAction(name_lex->name);

	//Parameters
	check_symbol(act_subtree->children[2], ":parameters");
	auto params_subtree = act_subtree->children[3]->asTree();
	auto params = parseTypedList(params_subtree->children.begin(), params_subtree->children.end(), Lexem::Type::Variable, true);
	result->loadParams(params, domain->typeTree);

	check_symbol(act_subtree->children[4], ":precondition");
	parseConditions(act_subtree->children[5]->asTree(), result, domain);
	check_symbol(act_subtree->children[6], ":effect");
	parseEffects(act_subtree->children[7]->asTree(), result, domain);

	/*checkObligatoryLexems(lexer_tape, true, "(", ":action", nullptr);

	//Action name
	auto name_lex = checkLexemType(lexer_tape, Lexem::Type::Identifier, true);
	

	//Parameters
	checkObligatoryLexems(lexer_tape, true, ":parameters", "(", nullptr);
	
	

	checkObligatoryLexem(lexer_tape, ")", true);

	//Precondition
	checkObligatoryLexems(lexer_tape, true, ":precondition", "(", nullptr);
	checkObligatoryLexem(lexer_tape, "and", true);

	while(lexer_tape.currentName() != ")")
	{
		AtomicFormula af = parseAtomicFormula(lexer_tape);

		UPDDLAtomicFormula * new_af = new UPDDLAtomicFormula();
		
		new_af->predicate = domain->predicate(af.name_lexem->name);
		for(auto param : af.param_list)
		{
			auto par = result->parameter(param->name);
			new_af->params.push_back(par);
		}

		UPDDLGoalDescription gd;
		gd.root = new_af;

		result->precondition.push_back(gd);
	}

	checkObligatoryLexem(lexer_tape, ")", true);

	//Effect
	checkObligatoryLexems(lexer_tape, true, ":effect", "(", nullptr);
	checkObligatoryLexem(lexer_tape, "and", true);


	checkObligatoryLexem(lexer_tape, ")", true);
*/
	 /*= parseActionDef(index, lexed_symbols, types, predicates);

	if(!result && req_durative_actions)
		result = parseDurativeActionDef(index, lexed_symbols);

	if(!result && req_derived_predicates)
		result = parseDerivedDef(index, lexed_symbols);

	if(result)
	{
		PARSER_END;
	}

	checkObligatoryLexem(lexer_tape, ")", true);
*/
	return result;
}

void UParser::parseConditions(LexTree::TreeItem * conditions_subtree, UPDDLAction * action, const UPDDLDomain * domain)
{
	auto gd_parser = [=](const LexTree::TreeItem * conditions_subtree){
		UPDDLGoalDescription * gd = parseCondition(conditions_subtree, action, domain);
		action->conditions.push_back(gd);
	};

	if(conditions_subtree->children[0]->asSymbol()->text() == "and")
	{
		for(auto it = conditions_subtree->children.begin() + 1; it != conditions_subtree->children.end(); ++it)
		{
			//UPDDLGoalDescription * gd = parseCondition((*it)->asTree(), action, domain);
			//action->conditions.push_back(gd);
			gd_parser((*it)->asTree());
		}
	}
	else if(conditions_subtree->children[0]->asSymbol()->lexem()->type == Lexem::Type::Identifier)
	{
		gd_parser(conditions_subtree);
	}
	else
	{
		throw parser_exception("Unsupported conditions.");
	}
}

UPDDLGoalDescription * UParser::parseCondition(const LexTree::TreeItem * conditions_subtree, UPDDLAction * action, const UPDDLDomain * domain)
{
	UPDDLGoalDescription * result = new UPDDLGoalDescription();

	AtomicFormula af = parseAtomicFormula(conditions_subtree->asTree());

	result->predicate = domain->predicate(af.name_lexem->name);
	for(auto param_lex : af.param_list)
	{
		if(param_lex->type == Lexem::Type::Variable)
		{
			auto param = action->parameter(param_lex->name);
			result->params.push_back(param);
		}
		else if(param_lex->type == Lexem::Type::Identifier)
		{
			auto const_obj = domain->constantObject(param_lex->name);
			result->params.push_back(new UPDDLConstantParameter(const_obj));
		}
		else
			throw parser_exception("Invalid paramer.");
		
	}

	return result;
}

void UParser::parseEffects(LexTree::TreeItem * effects_subtree, UPDDLAction * action, const UPDDLDomain * domain)
{

	if(effects_subtree->children[0]->asSymbol()->text() == "and")
	{
		for(auto it = effects_subtree->children.begin() + 1; it != effects_subtree->children.end(); ++it)
		{
			UPDDLEffect * effect = parseEffect((*it)->asTree(), action, domain);
			action->effects.push_back(effect);
		}
	}
	else
	{
		throw parser_exception("Unsupported effect.");
	}

}

UPDDLEffect * UParser::parseEffect(LexTree::TreeItem * effect_subtree, UPDDLAction * action, const UPDDLDomain * domain)
{
	UPDDLEffect * result(nullptr);

	auto main_lex = effect_subtree->children[0]->asSymbol()->lexem();
	if((main_lex->name == "not") || (main_lex->type == Lexem::Type::Identifier))
	{
		UPDDLAtomicEffect * effect = new UPDDLAtomicEffect();
		AtomicFormula af;
		if(main_lex->name == "not")
		{
			af = parseAtomicFormula(effect_subtree->children[1]->asTree());
			effect->negative = true;
		}
		else
		{
			af = parseAtomicFormula(effect_subtree);
			effect->negative = false;
		}

		effect->predicate = domain->predicate(af.name_lexem->name);
		for(auto param_lex : af.param_list)
		{
			if(param_lex->type == Lexem::Type::Variable)
			{
				auto param = action->parameter(param_lex->name);
				if(!param)
					throw parser_exception("Parameter not found!");
				effect->params.push_back(param);
			}
			else if(param_lex->type == Lexem::Type::Identifier)
			{
				auto const_obj = domain->constantObject(param_lex->name);
				effect->params.push_back(new UPDDLConstantParameter(const_obj));
			}
			else
				throw parser_exception("Invalid parameter lexem!");
		}

		result = effect;

	}
	else if((main_lex->name == "assign") || (main_lex->name == "scale-up") || (main_lex->name == "scale-down") || (main_lex->name == "increase") || (main_lex->name == "decrease"))
	{
		UPDDLNumericEffect * effect = new UPDDLNumericEffect();
		if(main_lex->name == "assign")
			effect->numericType = UNumericEffect::Type::Increase;
		else if(main_lex->name == "scale-up")
			effect->numericType = UNumericEffect::Type::ScaleUp;
		else if(main_lex->name == "scale-down")
			effect->numericType = UNumericEffect::Type::ScaleDown;
		else if(main_lex->name == "increase")
			effect->numericType = UNumericEffect::Type::Increase;
		else if(main_lex->name == "decrease")
			effect->numericType = UNumericEffect::Type::Decrease;
		
		AtomicFormula af = parseAtomicFormula(effect_subtree->children[1]->asTree());
		effect->function = domain->function(af.name_lexem->name);

		for(auto param_lex : af.param_list)
		{
			auto param = action->parameter(param_lex->name);
			effect->params.push_back(param);
		}

		//Expression
		if(effect_subtree->children[2]->type() == LexTree::Item::Type::Symbol)	//If it is constant
		{
			effect->expression.func = 0;
			effect->expression.value = atof(effect_subtree->children[2]->asSymbol()->text().c_str());
		}
		else //Function value
		{
			AtomicFormula eaf = parseAtomicFormula(effect_subtree->children[2]->asTree());
			effect->expression.func = domain->function(eaf.name_lexem->name);

			for(auto param_lex : eaf.param_list)
			{
				auto param = action->parameter(param_lex->name);
				effect->expression.params.push_back(param);
			}
		}

		result = effect;
	}
	else
	{
		throw parser_exception("Unsupported effect.");
	}

	return result;
}

AtomicFormula UParser::parseAtomicFormula(const LexTree::TreeItem * atomic_formula_subtree) const
{
	/*
	<atomic formula(t)> ::= (<predicate> t*)
	<predicate> ::= <name>
	<variable> ::= ?<name>

	(:equality) <atomic formula(t)> ::= (= t t)
	*/

	AtomicFormula result;
	result.name_lexem = atomic_formula_subtree->children[0]->asSymbol()->lexem();
	for(auto it = atomic_formula_subtree->children.begin() + 1; it != atomic_formula_subtree->children.end(); ++it)
		result.param_list.push_back((*it)->asSymbol()->lexem());

	return result;
}

void UParser::parseInitialState(const LexTree::TreeItem * goals_subtree, UPDDLProblem * problem, const UPDDLDomain * domain)
{
	for(auto atom = goals_subtree->children.begin() + 1; atom != goals_subtree->children.end(); ++atom)
	{
		auto atom_subtree = (*atom)->asTree();

		if(atom_subtree->children[0]->asSymbol()->text() == "=")
		{
			FunctionTerm fterm = parseFunctionTerm(atom_subtree);
			problem->initialState.functions.push_back(UPDDLFunctionInstance(fterm, domain, problem));
		}
		else
		{
			AtomicFormula af = parseAtomicFormula(atom_subtree);
			problem->initialState.predicates.push_back(UPDDLPredicateInstance(af, domain, problem));
		}
	}
}

void UParser::parseGoals(const LexTree::TreeItem * goals_subtree, UPDDLProblem * problem, const UPDDLDomain * domain)
{
	auto goals_list = goals_subtree->children[1]->asTree();
	for(auto goal_it = goals_list->children.begin() + 1; goal_it != goals_list->children.end(); ++goal_it)
	{
		AtomicFormula af = parseAtomicFormula((*goal_it)->asTree());
		
		UPDDLPredicateInstance pi(af, domain, problem);
//		pi.negative = false;
		problem->goal.predicates.push_back(pi);
	}
}

FunctionTerm UParser::parseFunctionTerm(const LexTree::TreeItem * atomic_formula_subtree)
{
	FunctionTerm fterm;

	AtomicFormula af = parseAtomicFormula(atomic_formula_subtree->children[1]->asTree());

	fterm.value_lexem = atomic_formula_subtree->children[2]->asSymbol()->lexem();
	fterm.function_name = af.name_lexem;
	fterm.param_list = af.param_list;
	

	return fterm;
}
