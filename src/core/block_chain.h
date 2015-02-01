
#ifndef UltraCore_block_chain_h
#define UltraCore_block_chain_h

#include "byte_range.h"
#include <limits>
#include <iterator>

struct chain_info_t
{
	chain_info_t(size_t _first = std::numeric_limits<size_t>::max(), size_t _last = std::numeric_limits<size_t>::max(), size_t _block_count = 0)
	:first(_first), last(_last), block_count(_block_count)
	{}

	size_t first, last, block_count;
	//size_t _placeholder;
};


struct chain_address
{
	chain_address(int _block, int _element, int elements_per_block)
		:block(_block), element(_element), elementsPerBlock(elements_per_block)
	{}

	friend bool operator==(const chain_address & lhs, const chain_address & rhs)
	{
		return (lhs.block == rhs.block) && (lhs.element == rhs.element);
	}

	friend bool operator!=(const chain_address & lhs, const chain_address & rhs)
	{
		return (lhs.block != rhs.block) || (lhs.element != rhs.element);
	}

	chain_address & operator++()
	{
		if (++element == elementsPerBlock)
		{
			element = 0;
			++block;
		}

		return *this;
	}

	chain_address& operator--()
	{
		if ((--element == -1) && (block > 0))
		{
			element = elementsPerBlock - 1;
			--block;
		}

		return *this;
	}

	chain_address& operator+=(int delta)
	{
		block += delta / elementsPerBlock;
		element += delta % elementsPerBlock;

		if (element >= elementsPerBlock)
		{
			element -= elementsPerBlock;
			++block;
		}

		return *this;
	}

	int linear_address() const
	{
		return block * elementsPerBlock + element;
	}


	int block, element, elementsPerBlock;
};

template<typename C>
class block_iterator_base
{
	using chain_t = C;
public:
	typedef byte_range value_type;
	typedef ptrdiff_t difference_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::forward_iterator_tag iterator_category;

	

	block_iterator_base(chain_t * _chain, int _element_id, int elements_per_block)
		:m_pChain(_chain), /*m_elementId(_element_id), m_elementsPerBlock(elements_per_block),*/ m_address(_element_id / elements_per_block, _element_id % elements_per_block, elements_per_block)
	{}

	const int elementId() const
	{
		//return m_elementId;
		//return m_address.block * m_elementsPerBlock + m_address.element;
		return m_address.linear_address();
	}

	block_iterator_base& operator=(const block_iterator_base& rhs)
	{
		this->m_address = rhs.m_address;
		return *this;
	}

	//Returns byterange to RAM of current element (hash + val)
	byte_range operator*() const
	{
		/*auto addr = m_pChain->element_address(elementId());
		
		size_t offset = addr.second * m_pChain->element_size();

		auto & cur_block = m_pChain->blocks_cache()[m_pChain->block_global_address(m_address.block)];*/

		//if (addr.first == 2)
		//	cout << offset << std::endl;
		//if (offset >= block_t::DataSize)
		//	throw out_of_range("Out of block data");

		return byte_range(m_pChain->blocks_cache()[m_pChain->block_global_address(m_address.block)].data + m_address.element * m_pChain->element_size(), m_pChain->element_size());
	}

	friend bool operator!=(const block_iterator_base & lhs, const block_iterator_base & rhs)
	{
		return (lhs.m_address != rhs.m_address);
	}

	friend bool operator==(const block_iterator_base & lhs, const block_iterator_base & rhs)
	{
		return (lhs.m_address == rhs.m_address);
	}

	friend size_t operator-(const block_iterator_base & lhs, const block_iterator_base & rhs)
	{
		return lhs.elementId() - rhs.elementId();
	}
protected:
	chain_t * m_pChain;
	//size_t m_elementId;
	//int m_elementsPerBlock;
	chain_address m_address;
};


template<typename C>
class block_iterator : public block_iterator_base<C>
{
	using _Base = block_iterator_base<C>;
	using chain_t = C;

public:

	block_iterator(chain_t * _chain = nullptr, int _element_id = -1, int elements_per_block = 0)
		:_Base(_chain, _element_id, elements_per_block)
	{}

	//Prefix
	block_iterator& operator++()
	{
		++_Base::m_address;
		return *this;
	}

	//Postfix
	block_iterator operator++(int)
	{
		block_iterator old(*this);
		++(*this);
		return std::move(old);
	}

	//Prefix
	block_iterator& operator--()
	{
		--_Base::m_address;
		return *this;
	}

	//Postfix
	block_iterator operator--(int)
	{
		block_iterator old(*this);
		--(*this);
		return std::move(old);
	}


	block_iterator& operator+=(int delta)
	{
		_Base::m_address += delta;
		return *this;
	}

	friend block_iterator operator+(const block_iterator & lhs, int delta)
	{
		return block_iterator(lhs.m_pChain, lhs.elementId() + delta, lhs.m_address.elementsPerBlock);
	}

	friend block_iterator operator-(const block_iterator & lhs, int delta)
	{
		return block_iterator(lhs.m_pChain, lhs.elementId() - delta, lhs.m_address.elementsPerBlock);
	}
};

template<typename C>
class block_reverse_iterator : public block_iterator_base<C>
{
	using _Base = block_iterator_base<C>;
	using chain_t = C;

public:
	block_reverse_iterator(chain_t * _chain = nullptr, int _element_id = -1, int elements_per_block = 0)
		:_Base(_chain, _element_id, elements_per_block)
	{}

	friend block_reverse_iterator operator+(const block_reverse_iterator & lhs, int delta)
	{
		return block_reverse_iterator(lhs.m_pChain, lhs.elementId() - delta, lhs.m_address.elementsPerBlock);
	}

	//Prefix
	block_reverse_iterator& operator++()
	{
		--_Base::m_address;
		return *this;
	}

	//Postfix
	block_reverse_iterator operator++(int)
	{
		block_reverse_iterator old(*this);
		++(*this);
		return std::move(old);
	}
};

#endif
