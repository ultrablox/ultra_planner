#include "ULanguage.h"
#include "UPDDLGrammar.h"
#include <stdarg.h>

using namespace std;

ULanguage::ULanguage(UGrammar * _grammar)
    :m_pGrammar(_grammar)
{

}

ULanguage::~ULanguage()
{
	if(m_pGrammar)
		delete m_pGrammar;
}

UGrammar * ULanguage::grammar()
{
	return m_pGrammar;
}

void ULanguage::setGrammar(UGrammar * _grammar)
{
	m_pGrammar = _grammar;
}

size_t ULanguage::lexemCount()
{
	size_t total_count = 0;
    for(auto l : mLexems)
		total_count += l.second.size();

	return total_count;
}

vector<Lexem*> ULanguage::lexems()
{
	vector<Lexem*> result;
    for(auto l : mLexems)
		result.insert(result.end(), l.second.begin(), l.second.end());

	return result;
}

ULanguage::LexemArray ULanguage::lexems(Lexem::Type type)
{
	return mLexems[type];
}

ULanguage::LexemArray ULanguage::separatingLexems()
{
	LexemArray result;

	for(int i = static_cast<int>(Lexem::Type::Separator); i <= static_cast<int>(Lexem::Type::Space); ++i)
    {
        auto & arr = mLexems[static_cast<Lexem::Type>(i)];
		result.insert(result.end(), arr.begin(), arr.end());
    }

	return result;
}

/*
string ULanguage::lexemName(size_t index)
{
	return lexem(index).name;
}

ULanguage::Lexem ULanguage::lexem(size_t index)
{
	return mLexems[index];
}*/

Lexem * ULanguage::addLexem(Lexem::Type type, const char * name)
{
	Lexem * lexem = new Lexem(name, type);
	/*lexem->type = type;
	memcpy(lexem->name, name, strlen(name)+1);
	if(type != LexemType::Identifier)
	{
		UGrammar::Terminal * term = m_pGrammar->terminal(name);
		if(term)
		{
			lexem->terminal = term;
		}
		else
			printf("Error: lexem '%s' not found in terminals.\n", name);

	}*/


	mLexems[type].push_back(lexem);

	return lexem;
}

void ULanguage::addKeyword(const char * name)
{
	addLexem(Lexem::Type::Keyword, name);
}

void ULanguage::vaddLexems(Lexem::Type type, size_t count, va_list args)
{
	for(size_t i = 0; i < count; ++i)
	{
		const char * lex_name = va_arg(args, const char *);
		addLexem(type, lex_name);
	}
}

void ULanguage::vaddLexems(Lexem::Type type, va_list args)
{
	while(const char * lex_name = va_arg(args, const char *))
	{
		addLexem(type, lex_name);
	}
}

void ULanguage::addLexems(Lexem::Type type, size_t count, ...)
{
	va_list args;
	va_start(args, count);

	vaddLexems(type, count, args);
	
	va_end(args);
}

void ULanguage::addLexems(Lexem::Type type...)
{
	va_list args;
	va_start(args, type);

    
	vaddLexems(type, args);

	va_end(args);
}

void ULanguage::addKeyWords(size_t count, ...)
{
	va_list args;
	va_start(args, count);

/*	for(size_t i = 0; i < count; ++i)
	{
		const char * kw_name = va_arg(args, const char *);
		addKeyword(kw_name);
	}
	*/
	
	vaddLexems(Lexem::Type::Keyword, count, args);
	
	va_end(args);
}

Lexem * ULanguage::lexem(const char * name)
{
	for(auto t : mLexems)
	{
		for(auto l : t.second)
		{
            Lexem * lex = l;
			if(!strcmp(lex->name.c_str(), name))
				return l;
		}
	}

	return 0;
}

//================================UPDDLLanguage=========================
UPDDLLanguage::UPDDLLanguage()
    :ULanguage(0)
{
	UPDDLGrammar * gramm = new UPDDLGrammar();
	setGrammar(gramm);

	addLexems(Lexem::Type::Space, " ", "\n", "\t", "\r", SPECIAL_SYMBOL_COMMENTS_SEPARATOR, nullptr);
	addLexems(Lexem::Type::Separator, "(", ")", nullptr);

    //domain
	addKeyWords(13, "domain", ":domain", "define", ":requirements", ":types", ":constants", ":objects", ":predicates", ":functions", ":constraints", ":init", ":goal", ":metric");
	
	//requierments
	addLexems(Lexem::Type::Requierment, ":strips", ":typing", ":negative-preconditions", ":disjunctive-preconditions", ":equality", ":existential-preconditions", ":universal-preconditions", ":quantified-preconditions", ":conditional-effects", ":fluents", ":numeric-fluents", ":adl", ":durative-actions", ":duration-inequalities", ":continuous-effects", ":derived-predicates", ":timed-initial-literals", ":preferences", ":constraints", ":action-costs", nullptr);

	//actions
	addKeyWords(4, ":action", ":parameters", ":precondition", ":effect");
    
	//requierments
	//addKeyWords(20, ":strips", ":typing", ":negative-preconditions", ":disjunctive-preconditions", ":equality", ":existential-preconditions", ":universal-preconditions", ":quantified-preconditions", ":conditional-effects", ":fluents", ":numeric-fluents", ":adl", ":durative-actions", ":duration-inequalities", ":continuous-effects", ":derived-predicates", ":timed-initial-literals", ":preferences", ":constraints", ":action-costs");
    
	//<assign-op>
	addKeyWords(5, "assign", "scale-up", "scale-down", "increase", "decrease");
    
	//Base types
	addLexems(Lexem::Type::Keyword, "object", "number", nullptr);

	//:durative-actions extension
	addLexems(Lexem::Type::Identifier, ":durative-action", ":duration", ":condition", nullptr);
    
	//Logical Operator
	addLexems(Lexem::Type::LogicalOperator, 3, "and", "or", "not");

	addLexems(Lexem::Type::Operator, "-", "/", "*", "+", ">", "<", "=", ">=", "<=", nullptr);
    
	
	addLexems(Lexem::Type::SpecSymbol, 1, "-");

	
}
