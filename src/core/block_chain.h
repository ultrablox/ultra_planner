
#ifndef UltraCore_block_chain_h
#define UltraCore_block_chain_h

struct chain_info_t
{
	chain_info_t(size_t _first = std::numeric_limits<size_t>::max(), size_t _last = std::numeric_limits<size_t>::max(), size_t _block_count = 0)
	:first(_first), last(_last), block_count(_block_count)
	{}

	size_t first, last, block_count;
	//size_t _placeholder;
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
	typedef forward_iterator_tag iterator_category;

	block_iterator_base(chain_t * _chain = nullptr, size_t _element_id = std::numeric_limits<size_t>::max())
		:m_pChain(_chain), m_elementId(_element_id)
	{}

	const size_t & elementId() const
	{
		return m_elementId;
	}

	block_iterator_base& operator=(const block_iterator_base& rhs)
	{
		this->m_elementId = rhs.m_elementId;
		return *this;
	}

	//Returns byterange to RAM of current element (hash + val)
	byte_range operator*() const
	{
		auto addr = m_pChain->element_address(m_elementId);
		auto & cur_block = m_pChain->blocks_cache()[addr.first];
		size_t offset = addr.second * m_pChain->element_size();

		//if (addr.first == 2)
		//	cout << offset << std::endl;
		//if (offset >= block_t::DataSize)
		//	throw out_of_range("Out of block data");

		return byte_range(cur_block.data + offset, m_pChain->element_size());
	}

	friend bool operator!=(const block_iterator_base & lhs, const block_iterator_base & rhs)
	{
		return (lhs.m_elementId != rhs.m_elementId);
	}

	friend bool operator==(const block_iterator_base & lhs, const block_iterator_base & rhs)
	{
		return (lhs.m_elementId == rhs.m_elementId);
	}

	friend size_t operator-(const block_iterator_base & lhs, const block_iterator_base & rhs)
	{
		return lhs.m_elementId - rhs.m_elementId;
	}
protected:
	chain_t * m_pChain;
	size_t m_elementId;
};


template<typename C>
class block_iterator : public block_iterator_base<C>
{
	using _Base = block_iterator_base<C>;
	using chain_t = C;

public:

	block_iterator(chain_t * _chain = nullptr, size_t _element_id = std::numeric_limits<size_t>::max())
		:_Base(_chain, _element_id)
	{}

	//Prefix
	block_iterator& operator++()
	{
		++m_elementId;
		return *this;
	}

	//Postfix
	block_iterator operator++(int)
	{
		block_iterator old(*this);
		++m_elementId;
		return std::move(old);
	}

	//Prefix
	block_iterator& operator--()
	{
		--m_elementId;
		return *this;
	}

	//Postfix
	block_iterator operator--(int)
	{
		block_iterator old(*this);
		--m_elementId;
		return std::move(old);
	}


	block_iterator& operator+=(size_t delta)
	{
		m_elementId += delta;
		return *this;
	}

	friend block_iterator operator+(const block_iterator & lhs, size_t delta)
	{
		return block_iterator(lhs.m_pChain, lhs.m_elementId + delta);
	}

	friend block_iterator operator-(const block_iterator & lhs, size_t delta)
	{
		return block_iterator(lhs.m_pChain, lhs.m_elementId - delta);
	}
};

template<typename C>
class block_reverse_iterator : public block_iterator_base<C>
{
	using _Base = block_iterator_base<C>;
	using chain_t = C;

public:
	block_reverse_iterator(chain_t * _chain = nullptr, size_t _element_id = std::numeric_limits<size_t>::max())
		:_Base(_chain, _element_id)
	{}

	friend block_reverse_iterator operator+(const block_reverse_iterator & lhs, size_t delta)
	{
		return block_reverse_iterator(lhs.m_pChain, lhs.m_elementId - delta);
	}

	//Prefix
	block_reverse_iterator& operator++()
	{
		--m_elementId;
		return *this;
	}

	//Postfix
	block_reverse_iterator operator++(int)
	{
		block_reverse_iterator old(*this);
		--m_elementId;
		return std::move(old);
	}
};

#endif
