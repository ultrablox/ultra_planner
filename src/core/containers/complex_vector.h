
#ifndef UltraCore_complex_vector_h
#define UltraCore_complex_vector_h

#include "../utils/helpers.h"
#include "../io/streamer.h"
#include "external_vector.h"
#include <functional>
#include <vector>
#include <type_traits>
#include <stxxl.h>

using namespace std;

template<typename T>
class named_vector : public std::vector<T>
{
public:
	named_vector(const std::string & fake_name)
	{}
};

template<bool ExtMem, int BS>
class complex_vector_base
{
protected:
	static const int BlockSize = BS;

	struct block_t
	{
		char data[BlockSize];
	};

	//using ext_base_vec_t = typename stxxl::VECTOR_GENERATOR<block_t>::result;
	using ext_base_vec_t = external_vector<block_t>;
	using int_ext_base_vec_t = named_vector<block_t>;
	using base_vec_t = typename std::conditional<ExtMem, ext_base_vec_t, int_ext_base_vec_t>::type;

public:

	complex_vector_base(const std::string & file_name = "")
		:m_data(file_name)
	{}

	/*template<typename... Args>
	complex_vector_base(Args... args)
		:m_data(args...)
	{}*/

	void clear()
	{
		m_data.clear();
	}

	bool empty() const
	{
		return m_data.empty();
	}

protected:
	base_vec_t m_data;
};

template<bool ExtMem, bool BlockLargerThanElement> class grained_vector_base {};

#define VECTOR_BLOCK_SIZE 32768

template<bool ExtMem>
class grained_vector_base<ExtMem, true> : public complex_vector_base<ExtMem, VECTOR_BLOCK_SIZE>
{
	using _Base = complex_vector_base<ExtMem, VECTOR_BLOCK_SIZE>;
protected:
	using block_t = typename _Base::block_t;
public:
	grained_vector_base(size_t serialized_value_size, const std::string & file_name = "grained_vector.dat")
		:_Base(file_name), m_elementsPerBlock(_Base::BlockSize / serialized_value_size), m_size(0)
	{
		//cout << "Creating complex vector with block_size=" << _Base::BlockSize << ", " << m_elementsPerBlock << ", " << m_elementsPerBlock << " elements per block" << std::endl;
		m_cache.item_count = 0;
	}

	//Return {BlockIndex, Offset}
	std::pair<size_t, size_t> element_address(size_t index) const
	{
		return make_pair(index / m_elementsPerBlock, index % m_elementsPerBlock);
	}

	size_t size() const
	{
		return m_size;
	}

	template<typename S, typename V>
	void push_back(const S & streamer, const V & new_val)
	{
		//cout << "Pushing..." << std::endl;
		streamer.serialize(m_cache.block.data + m_cache.item_count * streamer.serialized_size(), new_val);
		++m_cache.item_count;
		++m_size;

		if(m_cache.item_count == m_elementsPerBlock)
		{
			this->m_data.push_back(m_cache.block);
			m_cache.item_count = 0;
		}
	}

	template<typename S, typename V>
	void get(size_t index, const S & streamer, V & val) const
	{
		auto address = element_address(index);
		const char * ptr = 0;
		if(address.first < this->m_data.size())
		{
			const block_t & block_ref = this->m_data[address.first];
			ptr = block_ref.data + address.second * streamer.serialized_size();
		}
		else
			ptr = m_cache.block.data + address.second * streamer.serialized_size();

		streamer.deserialize(ptr, val);
	}

private:
	const size_t m_elementsPerBlock;
	size_t m_size;
	struct 
	{
		block_t block;
		size_t item_count;
	} m_cache;
};

template<bool ExtMem>
class grained_vector_base<ExtMem, false> : public complex_vector_base<ExtMem, 8>
{
protected:
	using _Base = complex_vector_base<ExtMem, 8>;
	using block_t = typename _Base::block_t;
public:
	grained_vector_base(size_t serialized_value_size, const std::string & file_name = "")
		:m_elementSize(integer_ceil(serialized_value_size, _Base::BlockSize)), m_valueTmp(m_elementSize)
	{
		//cout << "Creating complex vector with element_size = " << serialized_value_size << ", block_size = " << _Base::BlockSize << " (" << m_elementSize << " blocks per element)" << std::endl;
	}

	//Return {BlockIndex, 0}
	std::pair<size_t, size_t> element_address(size_t index) const
	{
		return make_pair(index / m_elementSize, 0);
	}

	template<typename S, typename V>
	void push_back(const S & streamer, const V & new_val)
	{
		//cout << "Pushing mini..." << std::endl;

		streamer.serialize(&m_valueTmp[0], new_val);

		for(int i = 0; i < m_elementSize; ++i)
			this->m_data.push_back(m_valueTmp[i]);
	}

	template<typename S, typename V>
	void get(size_t index, const S & streamer, V & val) const
	{
		auto address = element_address(index);
		const block_t & block_ref = this->m_data[address.first];

		const char * ptr = (const char *)&block_ref;
		streamer.deserialize(ptr + address.second, val);
	}

	size_t size() const
	{
		return this->m_data.size() / m_elementSize;
	}

private:
	//Number of blocks per one value_type
	const int m_elementSize;
	mutable std::vector<block_t> m_valueTmp;
};

template<typename T, typename S = base_type_streamer<T>, bool ExtMemory = false, bool UseLargeBlocks = false>
class complex_vector : public grained_vector_base<ExtMemory, UseLargeBlocks>
{
	friend class proxy;
	friend class iterator_base;

	using _Base = grained_vector_base<ExtMemory, UseLargeBlocks>;

public:
	typedef complex_vector<T, S> _Self;
	typedef T value_type;
	using streamer_t = S;
	using block_t = typename _Base::block_t;
	using base_type_t = block_t;

	/*template<typename VecT>
	struct iterator_base
	{
		typedef forward_iterator_tag iterator_category;
		typedef T value_type;
		typedef ptrdiff_t difference_type;
		typedef difference_type distance_type;	// retained
		typedef value_type* pointer;
		typedef value_type& reference;


		struct proxy : public value_type
		{
			proxy(void * _ptr, VecT & _vec)
				:m_ptr(_ptr), m_vec(_vec)
			{
				//m_vec.m_deserializeFun(m_ptr, *this);
				m_vec.m_streamer.deserialize(m_ptr, *this);
			}

			proxy& operator=(const value_type &val)
			{
				m_vec.m_streamer.serialize(m_ptr, val);
				return *this;
			}

			proxy& operator=(const proxy & rhs)
			{
				m_vec.m_streamer.serialize(m_ptr, rhs);
				return *this;
			}

			bool operator==(const value_type & rhs) const
			{
				value_type tmp;
				m_vec.m_streamer.deserialize(m_ptr, tmp);
				return tmp == rhs;
			}

			bool operator<(const proxy & rhs) const
			{
				value_type v1, v2;
				m_vec.m_streamer.deserialize(m_ptr, v1);
				rhs.m_vec.m_streamer.deserialize(rhs.m_ptr, v2);
				return v1 < v2;
			}
		
		private:
			void * m_ptr;
			VecT & m_vec;
		};

		typedef proxy proxy_t;

		iterator_base(size_t _step, size_t _pos, VecT & vec)
			:step(_step), pos(_pos), m_vec(vec)
		{
		}

		proxy operator*()
		{
			return proxy(&m_vec.m_data[pos], m_vec);
		}

		//Prefix
		iterator_base& operator++()
		{
			pos += step;
			return *this;
		}

		//Postfix
		iterator_base operator++(int)
		{
			iterator_base tmp(*this);
			operator++();
			return tmp;
		}

		//Prefix
		iterator_base& operator--()
		{
			pos -= step;
			return *this;
		}

		//Postfix
		iterator_base operator--(int)
		{
			iterator_base tmp(*this);
			operator--();
			return tmp;
		}


		iterator_base& operator=(const iterator_base &rhs)
		{
			this->pos = rhs.pos;
			return *this;
		}

		friend bool operator==(const iterator_base & lhs, const iterator_base & rhs)
		{
			return (lhs.pos == rhs.pos);// && (lhs.step == rhs.step);
		}

		friend bool operator!=(const iterator_base & lhs, const iterator_base & rhs)
		{
			return !(lhs == rhs);
		}

		friend size_t operator-(const iterator_base & lhs, const iterator_base & rhs)
		{
			return (lhs.pos - rhs.pos) / lhs.step;
		}

		friend iterator_base operator-(const iterator_base & lhs, size_t delta)
		{
			return iterator_base(lhs.step, lhs.pos - delta * lhs.step, lhs.m_vec);
		}

		friend iterator_base operator+(const iterator_base & lhs, size_t delta)
		{
			return iterator_base(lhs.step, lhs.pos + delta * lhs.step, lhs.m_vec);
		}

		friend bool operator<(const iterator_base & lhs, const iterator_base & rhs)
		{
			return (lhs.pos < rhs.pos);
		}

		const size_t step;
		size_t pos;
		VecT & m_vec;
	};

	typedef iterator_base<_Self> iterator;
	typedef iterator_base<const _Self> const_iterator;
	*/

	complex_vector(const streamer_t & _steamer, const std::string & file_name = "vector.dat")
		:_Base(_steamer.serialized_size(), file_name), m_streamer(_steamer)
	{}

	value_type operator[](size_t index) const
	{
		value_type res;
		this->get(index, m_streamer, res);
		return std::move(res);
	}

	void push_back(const value_type & new_val)
	{
		_Base::push_back(m_streamer, new_val);
	}
/*	iterator begin()
	{
		return iterator(m_elementSize, 0, *this);
	}

	iterator end()
	{
		return iterator(m_elementSize, this->m_data.size(), *this);
	}

	const_iterator begin() const
	{
		return const_iterator(m_elementSize, 0, *this);
	}

	const_iterator end() const
	{
		return const_iterator(m_elementSize, this->m_data.size(), *this);
	}

	void insert(iterator iter, const value_type & new_val)
	{
		this->m_data.insert(this->m_data.begin() + iter.pos, m_elementSize, 0);
		//m_serializeFun(&m_data[iter.pos], new_val);
		m_streamer.serialize(&this->m_data[iter.pos], new_val);
	}
*/
private:
	streamer_t m_streamer;
};


#endif
