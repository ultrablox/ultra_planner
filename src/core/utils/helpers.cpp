#include "helpers.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <fstream>
#include <codecvt>

#ifdef WIN32
    #include <windows.h>
    #include <psapi.h>
#endif

using namespace std;
using namespace Serializing;

void string_split(vector<string> & result, const char * c_str, const char * c_separator)
{
	string str(c_str);
	string separator(c_separator);

	while(true)
	{
		size_t separator_pos = str.find(separator);
		if(separator_pos == string::npos)
			break;

		string str_start = str.substr(0, separator_pos);
		result.push_back(str_start);

		int new_pos = separator_pos + separator.size();

		str = str.substr(new_pos);
	}

	if(str != "")
		result.push_back(str);
}

std::string &ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
std::string &rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
std::string trim(std::string s)
{
	return ltrim(rtrim(s));
}

std::string removeBrackets(string str)
{
	return str.substr(1, str.length() - 2);
}

size_t Serializing::serialize_uchar(char * dest, const unsigned char value)
{
	return serialize<unsigned char>(dest, value);
}

size_t Serializing::serialize_int(char * dest, const int value)
{
	return serialize<int32_t>(dest, value);
}

size_t Serializing::deserialize_int(char * data, int & value)
{
	int32_t val;
	memcpy(&val, data, sizeof(int32_t));
	value = val;
	return sizeof(int32_t);
}

//================URatio======================
URatio::URatio(double num_, double denom_)
	:m_num(num_), m_denom(denom_)
{

}

double URatio::value() const
{
	return m_num / m_denom;
}

void URatio::add(double num_, double denom_)
{
	m_num += num_;
	m_denom += m_denom;
}

std::ostream & operator<<(std::ostream & os, const URatio & ratio)
{
	return os << ratio.value();
}

double get_internal_memory_usage()
{
#ifdef WIN32
	HANDLE hProcess = GetCurrentProcess();
	
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
	return static_cast<double>(pmc.PagefileUsage) / 1024 / 1024;
#else
    return 0.0;
#endif
}

void serialize_int(std::ofstream & fout, int32_t value)
{
	//fout.write((char*)&value, sizeof(value));
	serialize_value(fout, value);
}

int32_t deserialize_int(std::ifstream & fin)
{
	return deserialize_value<int32_t>(fin);
}

void serialize_string(std::ofstream & fout, const std::string & str)
{
	serialize_vector(fout, str);
	/*serialize_int(fout, str.size());
	
	for(int i = 0; i < str.size(); ++i)
		serialize(fout, str[i]);*/
}

std::string deserialize_string(std::ifstream & fin)
{
	return deserialize_vector<string>(fin);
	/*int str_size = deserialize_int(fin);

	string res;
	res.resize(str_size);

	return res;*/
}

int integer_ceil(int x, int y)
{
	return x/y + (x % y != 0);
}

std::wstring to_wstring(const std::string & str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//std::string narrow = converter.to_bytes(wide_utf16_source_string);
	return converter.from_bytes(str);
}

std::pair<size_t, bool> different_radix(size_t lhs, size_t rhs)
{
	size_t digit = 1ULL << (sizeof(size_t)*8 - 1);

	size_t res = 0;
	while ((lhs & digit) == (rhs & digit))
	{
		digit = digit >> 1;
		++res;
	}
	return make_pair(res, (lhs & digit) < (rhs & digit));
}

bool check_radix(size_t val, size_t radix_id)
{
	size_t digit = 1ULL << (sizeof(size_t)* 8 - 1 - radix_id);
	return (val & digit) != 0;
}

std::ostream & operator<<(std::ostream & os, const std::pair<unsigned char, unsigned char> & pair)
{
	os << '{' << pair.first << ',' << pair.second << '}';
	return os;
}
