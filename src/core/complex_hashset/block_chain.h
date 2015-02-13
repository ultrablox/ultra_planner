
#ifndef UltraCore_block_chain_h
#define UltraCore_block_chain_h

#include "../byte_range.h"
#include <limits>
#include <iterator>
#include <vector>
#include <iostream>

struct chain_info_t
{
	chain_info_t(size_t _first = std::numeric_limits<size_t>::max(), size_t _last = std::numeric_limits<size_t>::max(), size_t _block_count = 0)
	:first(_first), last(_last), block_count(_block_count)
	{}

	size_t first, last, block_count;
	//size_t _placeholder;

	friend std::ostream & operator<<(std::ostream & os, const chain_info_t & info)
	{
		os << "F:" << info.first << ", L:" << info.last << ", count=" << info.block_count << std::endl;
		return os;
	}
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

class block_iterator_abstract_base
{
public:
	block_iterator_abstract_base(int _element_id, int elements_per_block)
		:m_address(_element_id / elements_per_block, _element_id % elements_per_block, elements_per_block)
	{}

	const int elementId() const
	{
		return m_address.linear_address();
	}

	block_iterator_abstract_base& operator=(const block_iterator_abstract_base& rhs)
	{
		this->m_address = rhs.m_address;
		return *this;
	}

	friend bool operator!=(const block_iterator_abstract_base & lhs, const block_iterator_abstract_base & rhs)
	{
		return (lhs.m_address != rhs.m_address);
	}

	friend bool operator==(const block_iterator_abstract_base & lhs, const block_iterator_abstract_base & rhs)
	{
		return (lhs.m_address == rhs.m_address);
	}

	friend size_t operator-(const block_iterator_abstract_base & lhs, const block_iterator_abstract_base & rhs)
	{
		return lhs.elementId() - rhs.elementId();
	}

	const chain_address & address() const
	{
		return m_address;
	}
protected:
	chain_address m_address;
};

template<typename C>
class block_iterator_base : public block_iterator_abstract_base
{
	using chain_t = C;
public:
	typedef byte_range value_type;
	typedef ptrdiff_t difference_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::forward_iterator_tag iterator_category;

	block_iterator_base(chain_t * _chain, int _element_id, int elements_per_block)
		:block_iterator_abstract_base(_element_id, elements_per_block), m_pChain(_chain)
	{}

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

		return byte_range(m_pChain->block_ptr(m_address.block)->data + m_address.element * m_pChain->element_size(), m_pChain->element_size());
	}

protected:
	chain_t * m_pChain;
	//size_t m_elementId;
	//int m_elementsPerBlock;
};


template<class C>
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

template<class C>
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


template<typename B, typename V>
class block_chain
{
	//template<typename C>
	//friend class block_iterator_base;
	//using block_storage_t = S;
protected:
	using block_t = B;// typename block_storage_t::block_t;
	using value_streamer_t = V;

	struct block_info_t
	{
		explicit block_info_t(block_t * block_ptr = nullptr, bool is_modified = false)
		:modified(is_modified), pBlock(block_ptr)
		{}

		block_t * pBlock;
		bool modified;
	};

public:
	using block_const_iterator = block_iterator<const block_chain>;
	using block_reverse_const_iterator = block_reverse_iterator<const block_chain>;
	using block_iterator = block_iterator<block_chain>;
	using block_reverse_iterator = block_reverse_iterator<block_chain>;

	block_chain(chain_info_t & chain_info, const value_streamer_t & vs, int max_items_in_block)
		:limits(chain_info), m_totalElements(0), m_maxItemsInBlock(max_items_in_block), m_valueStreamer(vs), m_blocksData(chain_info.block_count)
	{
#if _DEBUG
		if ((limits.first == std::numeric_limits<size_t>::max()) || (limits.last == std::numeric_limits<size_t>::max()))
			throw runtime_error("Invalid chain");
#endif
	}

	size_t block_count() const
	{
		//return m_blockIds.size();
		return limits.block_count;
	}

	size_t element_size() const
	{
		//return m_hb.m_serializedElementSize;
		return m_valueStreamer.serialized_size() + sizeof(size_t);
	}

	/*const block_storage_t & blocks_cache() const
	{
		return m_blocksStorage;
	}

	block_storage_t & blocks_cache()
	{
		return m_blocksStorage;
	}*/

	block_reverse_iterator rbegin()
	{
		return block_reverse_iterator(this, m_totalElements - 1, m_maxItemsInBlock);
	}

	block_reverse_iterator rend()
	{
		return block_reverse_iterator(this, -1, m_maxItemsInBlock);
	}

	block_iterator begin()
	{
		return block_iterator(this, 0, m_maxItemsInBlock);
	}

	block_const_iterator cbegin() const
	{
		return block_const_iterator(this, 0, m_maxItemsInBlock);
	}

	block_iterator end()
	{
		return block_iterator(this, m_totalElements, m_maxItemsInBlock);
	}

	block_const_iterator cend() const
	{
		return block_const_iterator(this, m_totalElements, m_maxItemsInBlock);
	}

	//Block Id (in global coordinates) + element Id
	/*std::pair<size_t, size_t> element_address(size_t index) const
	{
	size_t block_number = index / m_hb.m_maxItemsInBlock;
	size_t element_number = index - block_number * m_hb.m_maxItemsInBlock;
	return make_pair(m_blockIds[block_number], element_number);
	}*/

	/*size_t block_global_address(int local_block_addr) const
	{
		return m_blockIds[local_block_addr];
	}*/

	/*std::pair<size_t, size_t> local_element_address(size_t index) const
	{
	size_t block_number = index / m_hb.m_maxItemsInBlock;
	size_t element_number = index - block_number * m_hb.m_maxItemsInBlock;
	return make_pair(block_number, element_number);
	}*/

	template<typename Fun>
	void append_new_block(Fun request_block_fun)
	{
		block_t * p_new_block = request_block_fun();
		limits.last = p_new_block->meta.id;

		block_info_t & last_block_info = *m_blocksData.rbegin();
		last_block_info.modified = true;

		block_t * p_last_block = last_block_info.pBlock;

		p_new_block->meta.prev = p_last_block->meta.id;
		p_last_block->set_next(p_new_block->meta.id);
		

		m_blocksData.push_back(block_info_t(p_new_block, true));

		++limits.block_count;
	}

	block_iterator insert(const block_iterator & it, const byte_range & br)
	{
		//Check if insertion will overflow current blocks
#if _DEBUG
		if (m_totalElements + 1 > block_count() * m_maxItemsInBlock)
			throw out_of_range("Unable to insert into full chain");
#endif

		//m_hb.print_debug();
		block_t * p_lastBlock = (*m_blocksData.rbegin()).pBlock;
		p_lastBlock->inc_item_count();

		//Move tail right by 1 position
		auto old_end_it = end();
		++m_totalElements;

		//print();
		//std::copy_backward(it, old_end_it, end());
		fast_offset(it, old_end_it);
		

		for (int i = it.address().block; i < block_count(); ++i)
			m_blocksData[i].modified = true;
		//m_hb.print_debug();

		//Serialize element into freed space
		//auto new_addr = element_address(it.elementId());

		block_t * p_curBlock = m_blocksData[it.address().block].pBlock;
		char * dest_ptr = p_curBlock->data + it.address().element * element_size();

		//if (new_addr.first == 13)
		//	cout << new_addr.first << std::endl;

#if _DEBUG
		if (element_size() != br.size)
			throw runtime_error("Size incorrect");
#endif

		memcpy(dest_ptr, br.start, br.size);
		/* *((size_t*)ptr) = hash_fun(element);
		memcpy(ptr + sizeof(size_t), data_fun(element), m_hb.m_valueStreamer.serialized_size());*/
		//m_hb.print_debug();

		//print();

		return (it + 1);
	}

	template<typename It>
	void insert(block_iterator dest_it, It begin_it, It end_it)
	{
		for (auto it = begin_it; it != end_it; ++it)
		{
			//auto br = *it;
			//cout << (void*)br.start << std::endl;
			dest_it = insert(dest_it, *it);
		}
	}

	template<typename F>
	void resize(F fun, size_t new_size)
	{
		if (new_size > m_totalElements)
		{
			size_t max_elements_in_current_chain = m_maxItemsInBlock * limits.block_count;
			if (new_size <= max_elements_in_current_chain)
			{
				m_blocksData.rbegin()->pBlock->inc_item_count(new_size - m_totalElements); // m_hb.m_blocks[limits.last].inc_item_count(new_size - m_totalElements);
				m_totalElements = new_size;
			}
			else
			{
				size_t extra_size = new_size - max_elements_in_current_chain;
				m_totalElements = max_elements_in_current_chain;

				m_blocksData.rbegin()->pBlock->set_item_count(m_maxItemsInBlock);// m_hb.m_blocks[limits.last].set_item_count(m_hb.m_maxItemsInBlock);

				while (extra_size > 0)
				{
					append_new_block(fun);
					block_t & new_block = *m_blocksData.rbegin()->pBlock;//m_hb.m_blocks[limits.last];

					if (extra_size > m_maxItemsInBlock)
					{
						new_block.set_item_count(m_maxItemsInBlock);
						extra_size -= m_maxItemsInBlock;
						m_totalElements += m_maxItemsInBlock;
					}
					else
					{
						new_block.set_item_count(extra_size);
						m_totalElements += extra_size;
						extra_size = 0;
					}
				}
			}
		}
		else if (new_size < m_totalElements)
			m_totalElements = new_size;
	}

	/*void erase(block_iterator begin, block_iterator last_it)
	{
		if (last_it != end())
			throw runtime_error("Not implemented");

		auto addr = local_element_address(begin.m_elementId);

		m_hb.m_blocks[m_blockIds[addr.first]].item_count = addr.second;

		int new_block_count = addr.first + 1;
		if (addr.second == 0)
		{
			m_hb.delete_block(m_blockIds[addr.first]);
			--new_block_count;
		}

		for (int i = addr.first + 1; i < m_blockIds.size(); ++i)
			m_hb.delete_block(m_blockIds[i]);

		m_blockIds.resize(new_block_count);
		limits.last = m_blockIds[m_blockIds.size() - 1];
		limits.block_count = m_blockIds.size();
		m_hb.m_blocks[limits.last].next = limits.last;
	}*/

	/*
	Returns index to the block, which first hash does not coincide
	with the last hash of previous block. 0 if there is no such blocks.
	*/
	int partitioned_block()
	{
		int best_succ_block = 0, best_block = m_blocksData.size() / 2;
		for (int i = 1; i < m_blocksData.size(); ++i)
		{
			if (m_blocksData[i].pBlock->first_hash() != m_blocksData[i - 1].pBlock->last_hash(element_size()))
				best_succ_block = abs(i - best_block) < abs(best_succ_block - best_block) ? i : best_succ_block;
		}

		return best_succ_block;
		/*

		*/

		/*for (int i = m_blocksData.size() - 1; i > 0; --i)
		{
			if (m_blocksData[i].pBlock->first_hash() != m_blocksData[i - 1].pBlock->last_hash(element_size()))
				return i;
		}*/

		return 0;
	}

	size_t element_count() const
	{
		return m_totalElements;
	}

	void print()
	{
		std::cout << "Chain (" << m_blocksData.size() << " blocks): ";
		int last_block = -1;
		for (auto it = begin(); it != end(); ++it)
		{
			if (it.address().block != last_block)
			{
				std::cout << '|';
				last_block = it.address().block;
			}
			else
				std::cout << ',';
			std::cout << (*it).template begin_as<size_t>();
		}
		std::cout << std::endl;
		//for (auto bid : m_blockIds)
		//	m_hb.print_block(m_hb.m_blocks[bid]);

	}

	float density() const
	{
		return (float)m_totalElements / (limits.block_count * m_maxItemsInBlock);
	}


	block_t * block_ptr(size_t local_index) const
	{
		return m_blocksData[local_index].pBlock;
	}

	void fast_offset(const block_iterator & it, const block_iterator & last_it)
	{
		if (it.address() == last_it.address())
			return;

		int block_index = block_count() - 1;
		
		//Offset tail - just move the whole tail
		if (block_index == it.address().block)
		{
			char * data_ptr = m_blocksData[block_index].pBlock->data;
			memmove(data_ptr + (it.address().element + 1) * element_size(), data_ptr + it.address().element * element_size(), (last_it.address().element - it.address().element) * element_size());
		}
		else
		{
			//Offset last block
			char * r_data_ptr = m_blocksData[block_index].pBlock->data, *l_data_ptr = m_blocksData[block_index-1].pBlock->data;
			memmove(r_data_ptr + 1 * element_size(), r_data_ptr, last_it.address().element * element_size());
			memcpy(r_data_ptr, l_data_ptr + (m_maxItemsInBlock - 1) * element_size(), element_size());
			--block_index;

			//Offset full blocks
			while (block_index != it.address().block)
			{
				r_data_ptr = m_blocksData[block_index].pBlock->data;
				l_data_ptr = m_blocksData[block_index - 1].pBlock->data;

				memmove(r_data_ptr + 1 * element_size(), r_data_ptr, (m_maxItemsInBlock - 1) * element_size());
				memcpy(r_data_ptr, l_data_ptr + (m_maxItemsInBlock - 1) * element_size(), element_size());
				--block_index;
			}

			//Offset first block
			char * data_ptr = m_blocksData[block_index].pBlock->data;
			memmove(data_ptr + (it.address().element + 1) * element_size(), data_ptr + it.address().element * element_size(), (m_maxItemsInBlock - it.address().element - 1) * element_size());
		}
	}

	chain_info_t & limits;
	//complex_hashset_base & m_hb;
	size_t m_totalElements;
	//vector<size_t> m_blockIds;
	std::vector<block_info_t> m_blocksData;
	int m_maxItemsInBlock;
	const value_streamer_t & m_valueStreamer;
//	block_storage_t & m_blocksStorage;
};

#endif
