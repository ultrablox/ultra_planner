#include "ULexer.h"
#include "../ULanguage.h"
#include <core/UError.h>
#include <string>
#include <algorithm>
#include <regex>
#include <fstream>

using namespace std;

void movePointer(char *& pointer, size_t & row, size_t & column)
{
	if(pointer[0] == '\n')
	{
		row++;
		column = 0;
	}
	else
		column++;

	pointer++;
}

void movePointer(char *& pointer, size_t offset, size_t & row, size_t & column)
{
	for(size_t i = 0; i < offset; ++i)
		movePointer(pointer, row, column);
}

bool match_lexem(const char * pointer, Lexem * lexem)
{
	char lexem_name[256];
	//sprintf(lexem_name, "%s", lexem->name);

	int r = memcmp(lexem->name.c_str(), pointer, lexem->name.length());
	return (r == 0);
}

vector<Lexem*> getAvalibleLexems(ULanguage * language, const char * pointer)
{
	vector<Lexem*> result;

	auto all_lexems = language->lexems();

	for(size_t i = 0; i < all_lexems.size(); ++i)
	{
		if(match_lexem(pointer, all_lexems[i]))
		{
			result.push_back(all_lexems[i]);
		}
		/*char lexem_name[256];
		sprintf(lexem_name, "%s", all_lexems[i]->name.c_str());

		int r = memcmp(lexem_name, pointer, strlen(lexem_name));
		if(r == 0)
			*/
	}

	return result;
}

ULexer::ULexer(ULanguage * _language)
	:m_pLanguage(_language)
{

}

string remove_comments(const std::string & str)
{
	auto comment_pos = str.find(";");
	if(comment_pos == string::npos)
		return str;
	else
	{
		return str.substr(0, comment_pos);
	}
}

LexTree ULexer::parseFile(const std::string & file_name)
{
	//Open file
	ifstream file(file_name);
	if(!file.is_open())
		throw core_exception("Unable to open file");

	//Preprocess text by removing comments

	string file_data, str;
	while (getline(file, str))
    {
		str = remove_comments(str);
		file_data += str + "\n";
    }
    file.close();

	//Parse to tape
	auto tape = performLexAnalize(file_data.c_str());

	//Build tree based on tape
	return buildLexTree(tape);
}

bool lexem_longer_name(Lexem * l1, Lexem * l2)
{
	return l1->name.length() > l2->name.length();
}

LexerTape ULexer::performLexAnalize(const char * input_data)
{
	LexerTape result;

	mCurrentInputSymbol.column = 0, mCurrentInputSymbol.row = 0;
	m_pPointer = (char*)input_data;

	while(m_pPointer[0] != 0)
	{
		//Pass returns
		//ignoreBadSymbols();

		Lexem * next_separator_lexem = 0;
		size_t separator_pos = nextSeparatorPos(next_separator_lexem);

		if(separator_pos == 0) //Next lexem is separator
		{
			if(next_separator_lexem->type == Lexem::Type::Space)
			{
				movePointer(next_separator_lexem->name.length());
                
                if(!strcmp(next_separator_lexem->name.c_str(), SPECIAL_SYMBOL_COMMENTS_SEPARATOR))
                {
                    movePointerToString("\n");
                    movePointer();
                }
			}
			else
			{
				addSymbol(result, next_separator_lexem, mCurrentInputSymbol.row, mCurrentInputSymbol.column);
				movePointer(next_separator_lexem->name.length());
			}
		}
		else
		{
			//Get symbols till divider
			char symbols[256];
			memset(symbols, 0, sizeof(symbols));
			memcpy(symbols, m_pPointer, separator_pos);

			Lexem * lexem = m_pLanguage->lexem(symbols);


			if(!lexem)
			{
				std::regex identifier_regex("[a-zA-Z][a-zA-Z\-0-9\_]*"), number_regex("[0-9]+");
				if(std::regex_match(symbols, identifier_regex)) //Check if current element is identifier
				{
					lexem = m_pLanguage->addLexem(Lexem::Type::Identifier, symbols);
				}
				else if(std::regex_match(symbols, number_regex)) //Check if current element is number
				{
					lexem = m_pLanguage->addLexem(Lexem::Type::Number, symbols);
				}
				else
				{
					if((symbols[0] == '?') && std::regex_match(symbols+1, identifier_regex)) //Check if current element is variable
					{
						lexem = m_pLanguage->addLexem(Lexem::Type::Variable, symbols);

						//lexem = m_pLanguage->lexem("?");
					}
					else
						printf("Discovered unknown lexem (%d, %d): %s.\n", mCurrentInputSymbol.row + 1, mCurrentInputSymbol.column + 1, symbols);
				}
			}
			
			addSymbol(result, lexem, mCurrentInputSymbol.row, mCurrentInputSymbol.column);
			movePointer(lexem->name.length());
		}
	}

	return result;
}

void ULexer::addSymbol(vector<InputSymbol> & result, Lexem * lexem, size_t row, size_t column)
{
	InputSymbol symb;
	symb.lexem = lexem;
	symb.source_row = row + 1;
	symb.source_column = column + 1;

	result.push_back(symb);
}


size_t ULexer::nextSeparatorPos(Lexem *& separator_lexem)
{
	auto separators = m_pLanguage->separatingLexems();
	size_t pos = 0;

	while(m_pPointer[pos] != 0)
	{
		for(auto s : separators)
        {
			if(match_lexem(m_pPointer + pos, s))
			{
				separator_lexem = s;
				return pos;
			}
		}

		++pos;
	}

	return 0;
}


void ULexer::movePointer()
{
	if(m_pPointer[0] == '\n')
	{
		mCurrentInputSymbol.row++;
		mCurrentInputSymbol.column = 0;
	}
	else
		mCurrentInputSymbol.column++;

	m_pPointer++;
}

void ULexer::movePointer(size_t offset)
{
	for(size_t i = 0; i < offset; ++i)
		movePointer();
}

bool ignoreChar(const char symbol)
{
	const char ignoring_chars[] = {'\n', (char)13, (char)9};

	for(int i = 0; i < sizeof(ignoring_chars); ++i)
	{
		if(ignoring_chars[i] == symbol)
			return true;
	}

	return false;
}

void ULexer::ignoreBadSymbols()
{
	while(ignoreChar(m_pPointer[0]))
	{
		movePointer();
	}
}

void ULexer::movePointerToString(const char * str)
{
	while(strncmp(m_pPointer, str, strlen(str)))
	{
		++m_pPointer;
	}
}

void add_symbols(const LexerTape & lex_tape, size_t first_index, size_t last_index, LexTree::TreeItem * subtree)
{
	for(auto i = first_index; i <= last_index; ++i)
	{
		auto new_symb = new LexTree::SymbolItem();
		new_symb->symb = lex_tape[i];
		subtree->children.push_back(new_symb);
	}
}

void build_sub_tree(const LexerTape & lex_tape, size_t first_index, size_t last_index, LexTree::TreeItem * subtree)
{
	auto current_index = first_index;

	//Find first bracket
	bool found = true;
	while(found && (current_index <= last_index))
	{
		found = false;
		auto bracket_it = std::find_if(lex_tape.begin() + current_index, lex_tape.begin() + last_index, [&](const InputSymbol & is){
			if(is.lexem->name == "(")
			{
				found = true;
				return true;
			}
			else
				return false;
		});

		//If nothing found - add all symbols as symbols
		if(!found)
		{
			add_symbols(lex_tape, current_index, last_index, subtree);
		}
		else
		{
			auto left_bracket_index = std::distance(lex_tape.begin(), bracket_it);

			//Add all symbols before bracket to symbols
			add_symbols(lex_tape, current_index, left_bracket_index - 1, subtree);

			//Find right bracket
			int stack_size = 1;
			int right_bracket_index = left_bracket_index;

			while(stack_size > 0)
			{
				++right_bracket_index;

				auto cur_name = lex_tape[right_bracket_index].lexem->name;
				if(cur_name == ")")
					--stack_size;
				else if(cur_name == "(")
					++stack_size;
			}

			auto new_subtree = new LexTree::TreeItem();
			build_sub_tree(lex_tape, left_bracket_index + 1, right_bracket_index - 1, new_subtree);

			subtree->children.push_back(new_subtree);

			current_index = right_bracket_index + 1;
		}
	}
}

LexTree ULexer::buildLexTree(const LexerTape & lex_tape) const
{
	LexTree result;

	//Forget about left and right brackets
	build_sub_tree(lex_tape, 1, lex_tape.size() - 2, &result.root);

	return result;
}
