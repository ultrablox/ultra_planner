#ifndef UltraPlanner_ULanguage_h
#define UltraPlanner_ULanguage_h

#include "config.h"
#include "translator/UGrammar.h"
#include "translator/common_objects.h"
#include <vector>
#include <string>
#include <map>

class UGrammar;

class ULanguage
{
public:
	typedef std::vector<Lexem*> LexemArray;
	ULanguage(UGrammar * grammar);
	~ULanguage();
	UGrammar * grammar();
	void setGrammar(UGrammar * grammar);

	size_t lexemCount();
	std::vector<Lexem*> lexems();
	LexemArray lexems(Lexem::Type type);
	/*
	Returns lexems of types Operator, Separator, Space.
	*/
	LexemArray separatingLexems();
	//std::string lexemName(size_t index);
	//Lexem lexem(size_t index);
	Lexem * addLexem(Lexem::Type type, const char * name);
	void addKeyword(const char * name);
	void vaddLexems(Lexem::Type type, size_t count, va_list args);
	void vaddLexems(Lexem::Type type, va_list args);
	void addLexems(Lexem::Type type, size_t count, ...);
	void addLexems(Lexem::Type type...);
	void addKeyWords(size_t count, ...);
	Lexem * lexem(const char * name);
private:
	//LexemArray mLexems[LexemType::Count];
    map<Lexem::Type, LexemArray> mLexems;
    UGrammar * m_pGrammar;
};

class ULTRA_PDDL_API UPDDLLanguage : public ULanguage
{
public:
    UPDDLLanguage();
};

#endif
