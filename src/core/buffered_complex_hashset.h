
#ifndef UltraCore_buffered_complex_hashset_h
#define UltraCore_buffered_complex_hashset_h

#include "complex_hashset_base.h"


template<typename S>
class vector_storage_wrapper
{
	using storage_t = S;
	using block_t = typename S::value_type;

public:
	vector_storage_wrapper()
	{
	}

	size_t size() const
	{
		return m_storage.size();
	}

	void push_back(const block_t & block)
	{
		m_storage.push_back(block);
	}

	block_t & operator[](size_t index)
	{
		return m_storage[index];
	}

	const block_t & operator[](size_t index) const
	{
		return m_storage[index];
	}

	void read_into(size_t index, block_t & block)
	{
		block = m_storage[index];
	}

	template<typename It>
	void write(It begin, It end)
	{
		for (auto it = begin; it != end; ++it)
		{
			if (it->id < m_storage.size())
				m_storage[it->id] = std::move(*it);
			else if (it->id == m_storage.size())
				m_storage.push_back(std::move(*it));
			else
				throw runtime_error("Invalid block indexing");
		}
	}
private:
	storage_t m_storage;
};

namespace buffered_hashset
{
	template<typename B>
	struct ext_storage_generator
	{
		using block_t = B;
		using result = typename stxxl::VECTOR_GENERATOR<block_t, 1U, 65536U, sizeof(block_t), stxxl::FR, stxxl::random>::result; //131072U //262144U //65536U //1048576U
	};

	template<typename B>
	struct int_storage_generator
	{
		using block_t = B;
		using result = std::vector<block_t>;
	};
};

/*
Internal memory - standard std::vector.
External - buffered stxxl vector.
*/

//std::conditional<UseIntMemory, buffered_hashset::int_storage_generator, buffered_hashset::ext_storage_generator>::type

template<typename T, typename S, typename H = std::hash<T>, bool UseIntMemory = true, unsigned int BlockSize = 8192U>//65536U //4096U // 8192U
class buffered_complex_hashset : public complex_hashset_base<T, S, H, vector_storage_wrapper, buffered_hashset::ext_storage_generator, BlockSize>
{
	using _Base = complex_hashset_base<T, S, H, vector_storage_wrapper, buffered_hashset::ext_storage_generator, BlockSize>;
//	
	
public:
	buffered_complex_hashset(const S & ss)
		:_Base(ss)
	{
	}

	template<typename It, typename HashFun, typename ValFun, typename CallbackFun>
	void insert_range(It begin, It end, HashFun hash_fun, ValFun val_fun, CallbackFun fun)
	{
		for (auto it = begin; it != end; ++it)
		{
			this->insert(val_fun(*it), hash_fun(*it));
		}
	}
};

#endif
