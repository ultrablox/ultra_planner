#ifndef UltraPlanner_ULexer_h
#define UltraPlanner_ULexer_h

#include "../config.h"
#include "../ULanguage.h"
#include "common_objects.h"

class ULexer;

struct InputSymbol
{
    Lexem * lexem;
    size_t source_row, source_column;
};

class LexerTape : public std::vector<InputSymbol>
{
public:
	LexerTape()
		:index(0)
	{
	}

	void next()
	{
		++index;
	}

	InputSymbol & currentSymbol()
	{
		return at(index);
	}

	Lexem * current()
	{
		return at(index).lexem;
	}

	const std::string & currentName()
	{
		return current()->name;
	}

	size_t index;
};

struct ULTRA_PDDL_API LexTree
{
    struct TreeItem;
    struct SymbolItem;
    
	struct Item
	{
		enum class Type {Symbol, SubTree};
		virtual Type type() const = 0;
        
        TreeItem * asTree()
        {
            return static_cast<TreeItem*>(this);
        }

		const TreeItem * asTree() const
        {
            return static_cast<const TreeItem*>(this);
        }
        
        SymbolItem * asSymbol()
        {
            return static_cast<SymbolItem*>(this);
        }
	};

	struct TreeItem : public Item
	{
		virtual Type type() const
		{
			return Type::SubTree;
		}

		std::vector<Item*> children;
	};

	struct SymbolItem : public Item
	{
		virtual Type type() const
		{
			return Type::Symbol;
		}
        
        const std::string & text()
        {
            return symb.lexem->name;
        }
        
        Lexem * lexem()
        {
            return symb.lexem;
        }
        
		InputSymbol symb;
	};

	TreeItem root;
};


class ULTRA_PDDL_API ULexer
{
public:	

	ULexer(ULanguage * language);
	LexTree parseFile(const std::string & file_name);
	LexerTape performLexAnalize(const char * input_data);
	LexTree buildLexTree(const LexerTape & lex_tape) const;
	void addSymbol(std::vector<InputSymbol> & result, Lexem * lexem, size_t row, size_t column);
private:
	size_t nextSeparatorPos(Lexem *& separator_lexem);
	void movePointer(size_t offset);
	void movePointer();
	void ignoreBadSymbols();
	void movePointerToString(const char * str);
private:
	ULanguage * m_pLanguage;
	char * m_pPointer;
	struct
	{
		size_t row, column;
	} mCurrentInputSymbol;
};



#endif
