
#ifndef UltraCore_external_vector_h
#define UltraCore_external_vector_h

#include <string>
#include "../io/cached_file.h"

template<typename T>
class external_vector
{
public:
	using value_type = T;
	using base_container_t = cached_file<value_type, 1024>;

	external_vector(const std::string & file_name = "exernal_vector.dat")
		:m_cachedFile(file_name)
	{
	}

	bool empty() const
	{
		return m_cachedFile.empty();
	}

	size_t size() const
	{
		return m_cachedFile.size();
	}

	void push_back(const value_type & val)
	{
		m_cachedFile.push_back(val);
	}

	const value_type & operator[](size_t index) const
	{
		try
		{
			const value_type & res = m_cachedFile[index];
			return res;
		}
		catch (...)
		{
			return const_cast<base_container_t&>(m_cachedFile)[index];
		}
	}

	size_t dump_size() const
	{
		return m_cachedFile.file().mem_size();
	}
private:
	base_container_t m_cachedFile;
};
#endif
