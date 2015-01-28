
#ifndef UltraCore_complex_hashset_base_h
#define UltraCore_complex_hashset_base_h

#include "utils/helpers.h"
#include "algorithm/algorithm.h"
#include "algorithm/merge.h"
#include <functional>
#include <map>
#include <stxxl.h>
#include "cached_file.h"
#include <core/utils/helpers.h>
#include <core/avl_tree.h>
#include <core/byte_range.h>
#include <stxxl/unordered_map>
//#include <tbb/parallel_for.h>
#include <thread>


class range_map_wrapper
{
public:
	struct chain_info_t
	{
		chain_info_t(size_t _first = std::numeric_limits<size_t>::max(), size_t _last = std::numeric_limits<size_t>::max(), size_t _block_count = 0)
		:first(_first), last(_last), block_count(_block_count)
		{}

		size_t first, last, block_count;
		//size_t _placeholder;
	};

	void compute_stats(int & max_chain_length) const
	{
		max_chain_length = 0;
		m_rtree.for_each([&](const chain_info_t & chi){
			max_chain_length = max(max_chain_length, (int)chi.block_count);
		});
	}

	size_t size() const
	{
		return m_rtree.size();
	}

	chain_info_t & chain_with_hash(size_t hash_val)
	{
		auto it = m_rtree.find(hash_val);
		return it.data();
	}
	
	void create_mapping(size_t hash_val, const chain_info_t & chain_id)
	{
		m_rtree.insert(hash_val, chain_id);
	}

	void update_mapping(size_t hash_val, const chain_info_t & chain_id)
	{
		auto it = m_rtree.find(hash_val);
		if (*it != hash_val)
			throw runtime_error("No such node");
		it.data() = chain_id;
	}

private:
	range_map<size_t, chain_info_t> m_rtree;
	size_t m_blockItemCount;
};


struct hashset_stats_t
{
	int max_chain_length;
	double average_chain_length;
	float density;
	size_t block_count;

	friend std::ostream & operator<<(std::ostream & os, const hashset_stats_t & stats)
	{
		os << "Max chain length: " << stats.max_chain_length << std::endl;
		os << "Average chain length: " << stats.average_chain_length << std::endl;
		os << "Density: " << stats.density * 100 << "%" << std::endl;
		os << "Block Count: " << stats.block_count << std::endl;
		return os;
	}
};

template<typename T, typename S, typename H, template<typename> class W, template<typename> class SG, unsigned int BlockSize>//65536U //4096U // 8192U
class complex_hashset_base
{
protected:
	typedef T value_type;
	using value_streamer_t = S;
	typedef H hasher_t;
	typedef std::pair<size_t, value_type> combined_value_t;
	
	using index_t = range_map_wrapper; 	//Maps hash_value => block chain where it should be
	using chain_info_t = typename index_t::chain_info_t;

	static const int MaxBlocksPerChain = 3;
	struct block_t;
	struct block_chain_t;

	class block_iterator
	{
		friend class complex_hashset_base;
		friend class block_chain_t;
	public:
		typedef forward_iterator_tag iterator_category;
		typedef byte_range value_type;
		typedef ptrdiff_t difference_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		block_iterator(block_chain_t & chain, size_t _element_id)
			:m_chain(chain), m_elementId(_element_id)
		{}

		//Returns hash of current element
		byte_range operator*() const
		{
			auto addr = m_chain.element_address(m_elementId);
			block_t & cur_block = m_chain.m_hb.m_blocks[addr.first];
			size_t offset = addr.second * m_chain.m_hb.m_serializedElementSize;
			
			//if (addr.first == 2)
			//	cout << offset << std::endl;
			if (offset >= block_t::DataSize)
				throw out_of_range("Out of block data");

			return std::move(byte_range(cur_block.data + offset, m_chain.m_hb.m_serializedElementSize));
		}

		const size_t & hash() const
		{
			byte_range br = operator*();
			
			size_t* hash_ptr = (size_t*)br.start;
			return *hash_ptr;
		}

		//Prefix
		block_iterator& operator++()
		{
			++m_elementId;
			return *this;
		}

		//Prefix
		block_iterator& operator--()
		{
			--m_elementId;
			return *this;
		}

		block_iterator& operator+=(size_t delta)
		{
			m_elementId += delta;
			return *this;
		}

		block_iterator& operator=(const block_iterator& rhs)
		{
			this->m_elementId = rhs.m_elementId;
			return *this;
		}

		friend bool operator!=(const block_iterator & lhs, const block_iterator & rhs)
		{
			return (lhs.m_elementId != rhs.m_elementId);
		}

		friend bool operator==(const block_iterator & lhs, const block_iterator & rhs)
		{
			return (lhs.m_elementId == rhs.m_elementId);
		}

		friend block_iterator operator+(const block_iterator & lhs, size_t delta)
		{
			return block_iterator(lhs.m_chain, lhs.m_elementId + delta);
		}

		friend block_iterator operator-(const block_iterator & lhs, size_t delta)
		{
			return block_iterator(lhs.m_chain, lhs.m_elementId - delta);
		}

		friend size_t operator-(const block_iterator & lhs, const block_iterator & rhs)
		{
			return lhs.m_elementId - rhs.m_elementId;
		}

		const void * value_ptr() const
		{
			auto addr = m_chain.element_address(m_elementId);
			block_t & cur_block = m_chain.m_hb.m_blocks[addr.first];
			return cur_block.data + addr.second * m_chain.m_hb.m_serializedElementSize + sizeof(size_t);
		}

	private:
		//const block_t & block;
		size_t m_elementId;
		block_chain_t & m_chain;
	};

	struct block_t
	{
		static const size_t Size = BlockSize;
		static const size_t DataSize = Size - 3*sizeof(size_t)-sizeof(int);

		block_t(size_t _id = std::numeric_limits<size_t>::max())
			:item_count(0), id(_id), next(_id), prev(_id)
		{
			memset(data, 0, DataSize);
		}

		int vacant_count(int element_size) const
		{
			return DataSize / element_size - item_count;
		}

		block_iterator begin(int element_size)
		{
			return block_iterator(0, *this, element_size);
		}

		block_iterator end(int element_size)
		{
			return block_iterator(item_count, *this, element_size);
		}

		//Returns iterator to the first element in the sequence of same hashes ~in the middle
		block_iterator middle(int element_size)
		{
			auto m_it = block_iterator(item_count / 2, *this, element_size);
			if (*m_it == *begin(element_size))
			{
				size_t middle_hash = *m_it;
				while (*m_it == middle_hash)
					++m_it;
			}
			else
			{
				size_t middle_hash = *m_it;
				while (*(m_it - 1) == middle_hash)
					--m_it;
			}

			return m_it;
		}

		size_t first_hash() const
		{
			return *((const size_t*)data);
		}

		size_t last_hash(size_t record_size) const
		{
			const char * ptr = data + record_size * (item_count - 1);
			return *((const size_t*)ptr);
		}

		void set_meta(size_t _id, size_t _prev, size_t _next, int _item_count)
		{
			id = _id;
			prev = _prev;
			next = _next;
			item_count = _item_count;
		}

		size_t id, next, prev;
		int item_count;
		char data[DataSize];

	};

	static_assert(sizeof(block_t) % 256 == 0, "Invalid block_t size!");

	struct block_chain_t
	{
		block_chain_t(complex_hashset_base & hb, chain_info_t & chain_info)
			:m_hb(hb), limits(chain_info), m_totalElements(0)
		{
			if ((limits.first == std::numeric_limits<size_t>::max()) || (limits.last == std::numeric_limits<size_t>::max()))
				throw runtime_error("Invalid chain");
			//cout << "Creating chain " << limits.first << "->" << limits.second << std::endl;
			size_t block_id = limits.first, last_block_id;
			do
			{
				last_block_id = block_id;
				m_blockIds.push_back(block_id);
				m_totalElements += m_hb.m_blocks[block_id].item_count;
				block_id = m_hb.m_blocks[block_id].next;
				//cout << block_id << std::endl;
			} while (last_block_id != block_id);
		}

		size_t block_count() const
		{
			return m_blockIds.size();
		}

		block_iterator begin()
		{
			return block_iterator(*this, 0);
		}

		block_iterator end()
		{
			return block_iterator(*this, m_totalElements);
		}

		//Block Id (in global coordinates) + element Id
		std::pair<size_t, size_t> element_address(size_t index) const
		{
			size_t block_number = index / m_hb.m_maxItemsInBlock;
			size_t element_number = index - block_number * m_hb.m_maxItemsInBlock;
			return make_pair(m_blockIds[block_number], element_number);
		}

		std::pair<size_t, size_t> local_element_address(size_t index) const
		{
			size_t block_number = index / m_hb.m_maxItemsInBlock;
			size_t element_number = index - block_number * m_hb.m_maxItemsInBlock;
			return make_pair(block_number, element_number);
		}

		block_iterator insert(block_iterator it, const byte_range & br)
		{
			//Check if insertion will overflow current blocks
			if (m_totalElements + 1 > m_blockIds.size() * m_hb.m_maxItemsInBlock)
			{
				//limits.last = m_hb.m_blocks.size();
				limits.last = m_hb.request_block();
				block_t & new_block = m_hb.m_blocks[limits.last];

				//new_block.next = limits.last;
				//new_block.item_count = 0;

				new_block.prev = *m_blockIds.rbegin();
				m_hb.m_blocks[*m_blockIds.rbegin()].next = new_block.id;

				//m_hb.m_blocks.push_back(std::move(new_block));
				m_blockIds.push_back(new_block.id);

				++limits.block_count;
				//m_hb.m_index.update_mapping(hash_val, limits);
			}

			//m_hb.print_debug();
			++m_hb.m_blocks[limits.last].item_count;
			
			//Move tail right by 1 position
			auto old_end_it = end();
			++m_totalElements;

			
			std::copy_backward(it, old_end_it, end());
			//m_hb.print_debug();

			//Serialize element into freed space
			auto new_addr = element_address(it.m_elementId);
			block_t & cur_block = m_hb.m_blocks[new_addr.first];
			char * dest_ptr = cur_block.data + new_addr.second * m_hb.m_serializedElementSize;
			
			//if (new_addr.first == 13)
			//	cout << new_addr.first << std::endl;

			if (m_hb.m_valueStreamer.serialized_size() + sizeof(size_t) != br.size)
				throw runtime_error("Size incorrect");

			memcpy(dest_ptr, br.start, br.size);
			/* *((size_t*)ptr) = hash_fun(element);
			memcpy(ptr + sizeof(size_t), data_fun(element), m_hb.m_valueStreamer.serialized_size());*/
			//m_hb.print_debug();
			return ++it;
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

		void erase(block_iterator begin, block_iterator last_it)
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
		}

		/*
		Returns index to the block, which first hash does not coincide
		with the last hash of previous block. 0 if there is no such blocks.
		*/
		int partitioned_block()
		{
			for (int i = 1; i < m_blockIds.size(); ++i)
			{
				if (m_hb.m_blocks[m_blockIds[i]].first_hash() != m_hb.m_blocks[m_blockIds[i - 1]].last_hash(m_hb.m_valueStreamer.serialized_size() + sizeof(size_t)))
					return i;
			}

			return 0;
		}

		size_t element_count() const
		{
			return m_totalElements;
		}
		
		chain_info_t & limits;
		complex_hashset_base & m_hb;
		size_t m_totalElements;
		vector<size_t> m_blockIds;
	};

	/*typedef typename stxxl::VECTOR_GENERATOR<block_t, 1U, 131072U, BlockSize, stxxl::FR, stxxl::random>::result stxxl_paged_vector_t;
	//typedef cached_file<block_t, 65536U> cached_paged_vector_t;
	using direct_paged_vector_t = data_file<block_t>;
	typedef direct_paged_vector_t ext_paged_vector_t;
	typedef std::vector<block_t> int_paged_vector_t;*/
	//typedef typename std::conditional<UseIntMemory, int_paged_vector_t, ext_paged_vector_t>::type paged_vector_t;
	//typedef storage_wrapper<paged_vector_t> wrapper_t;

	//using wrapper_t = typename std::conditional<UseIntMemory, W<int_paged_vector_t>, W<direct_paged_vector_t>>::type;
	using wrapper_t = W<typename SG<block_t>::result>;
public:
	class iterator
	{
		friend class complex_hashset_base;
	public:
		iterator(size_t _block_id, size_t _element_id)
			:block_id(_block_id), element_id(_element_id)
		{}

		friend bool operator==(const iterator & lhs, const iterator & rhs)
		{
			return (lhs.element_id == lhs.element_id) && (lhs.block_id == rhs.block_id);
		}

		friend bool operator!=(const iterator & lhs, const iterator & rhs)
		{
			return (lhs.element_id != lhs.element_id) || (lhs.block_id != rhs.block_id);
		}
	private:
		size_t block_id, element_id;
	};

	complex_hashset_base(const value_streamer_t & vs)
		:m_valueStreamer(vs), m_serializedElementSize(vs.serialized_size() + sizeof(size_t)), m_maxLoadFactor(0.9), m_elementCache(sizeof(size_t) + m_valueStreamer.serialized_size()), m_size(0), m_maxItemsInBlock(block_t::DataSize / m_serializedElementSize), m_blockCount(0)
	{
		if (m_serializedElementSize > block_t::DataSize)
			throw runtime_error("Serialized element is bigger than page size");

		cout << "Hashset block_size: " << BlockSize << " expected, real " << sizeof(block_t) << ", DataSize " << block_t::DataSize << std::endl;
		cout << "Hashset page can store " << m_maxItemsInBlock << " elements." << std::endl;

		//Create one empty block
		size_t first_block_id = request_block();
		m_index.create_mapping(0, chain_info_t(first_block_id, first_block_id, 1));
	}

	iterator end() const
	{
		return iterator(m_blocks.size(), -1);
	}

	//It will work faster, if you know hash in advance
	bool insert(const value_type & val)
	{
		size_t hash_val = m_hasher(val);

		return insert(val, hash_val);
	}

	bool insert(const value_type & val, size_t hash_val)
	{
		//Find appropriate block, create if it does not exists
		
		chain_info_t & chain_id = m_index.chain_with_hash(hash_val);

		/*bool created_new = false;
		if (chain_id.first == std::numeric_limits<size_t>::max())
		{
			chain_id.first = m_blocks.size();
			chain_id.last = chain_id.first;		
			chain_id.block_count = 1;
			m_blocks.push_back(block_t(chain_id.first));
			m_index.create_mapping(hash_val, chain_id);
			created_new = true;
		}*/

		//auto res = write_to_block(chain_id, hash_val, val);
		auto res = insert_into_chain(chain_id, hash_val, val);
		
		if (res)
			++m_size;

		return res;
	}

	size_t size() const
	{
		/*size_t res(0);
		for (auto & block : m_blocks)
			res += block.item_count;
		return res;*/
		return m_size;
	}

	hashset_stats_t get_stats() const
	{
		hashset_stats_t res;
		res.density = (float)m_size / (block_count() * (block_t::DataSize / m_serializedElementSize));
		res.block_count = m_blocks.size();

		m_index.compute_stats(res.max_chain_length);
		res.average_chain_length = (double)m_blocks.size() / m_index.size();

		return res;
	}

	iterator find(const value_type & val)
	{
		size_t hash_val = m_hasher(val);
	
		/*size_t block_id = m_index.block_with_hash(hash_val);
		if (block_id == std::numeric_limits<size_t>::max())
			return end();

		const block_t & block = m_blocks[block_id];

		//print_block(block);

		//Find iterator to element with hash >= hash_val
		auto end_it = block_iterator(block.item_count, block, m_serializedElementSize);
		auto it = std::lower_bound(block_iterator(0, block, m_serializedElementSize), end_it, hash_val);

		//Serialize searching value
		//vector<char> inserting_data(m_serializedValueSize);
		//m_serializeFun(&m_elementCache[0], val);
		m_valueStreamer.serialize(&m_elementCache[0], val);

		for (; (*it == hash_val) && (it != end_it); ++it)
		{
			if (memcmp(&m_elementCache[0], it.value_ptr(), m_valueStreamer.serialized_size()) == 0)
				return iterator(block_id, it.element_id);
		}
		*/

		block_chain_t chain(*this, m_index.chain_with_hash(hash_val));
		auto it = find_first_ge(chain.begin(), chain.end(), hash_val);
		

		if (it != chain.end())	//If we found element with similar hash, check for duplication
		{
			m_valueStreamer.serialize(&m_elementCache[0], val);
			for (; (it != chain.end()) && (*it == hash_val); ++it)
			{
				if (memcmp(&m_elementCache[0], it.value_ptr(), m_valueStreamer.serialized_size()) == 0)	//Duplication detected
				{
					auto addr = chain.element_address(it.m_elementId);
					return iterator(addr.first, addr.second);
				}
			}
		}

		return end();
	}

	size_t block_count() const
	{
		return m_blocks.size();
	}

	void print_debug()
	{
		auto chains = m_index.chains();
		cout << "=========================" << std::endl;
		cout << "Hashmap contains " << chains.size() << " chains" << std::endl;
		int i = 0;
		for (auto ch : chains)
		{
			block_chain_t chain(*this, ch.second);
			cout << "#" << i++ << " (hash " << ch.first << ", " << chain.block_count() << " blocks" << "):";
			
			for (auto it = chain.begin(); it != chain.end(); ++it)
			{
				cout << *((size_t*)((*it).start)) << ',';
			}
			cout << std::endl;

		}
	}
protected:
	bool insert_into_chain(chain_info_t & chain_id, size_t hash_val, const value_type & val)
	{
		block_chain_t chain(*this, chain_id);

		auto it = find_first_ge(chain.begin(), chain.end(), hash_val);

		m_valueStreamer.serialize(&m_elementCache[sizeof(size_t)], val);

		if (it != chain.end())	//If we found element with similar hash, check for duplication
		{
			for (; (it != chain.end()) && (*it == hash_val); ++it)
			{
				if (memcmp(&m_elementCache[sizeof(size_t)], it.value_ptr(), m_valueStreamer.serialized_size()) == 0)	//Duplication detected
					return false;
			}
		}

		memcpy(&m_elementCache[0], &hash_val, sizeof(size_t));
		chain.insert(it, byte_range(&m_elementCache[0], m_elementCache.size()));

		//Ballance
		if (chain_id.block_count > MaxBlocksPerChain)
		{
			//cout << "B" << std::endl;

			int partitioned_block_number = chain.partitioned_block();
			if (partitioned_block_number != 0)
			{
				
				//Fast split the chain
				chain_info_t new_chain_id;
				new_chain_id.first = chain.m_blockIds[partitioned_block_number];
				new_chain_id.last = chain_id.last;
				new_chain_id.block_count = chain.m_blockIds.size() - partitioned_block_number;
				m_blocks[new_chain_id.first].prev = new_chain_id.first;

				chain_id.last = chain.m_blockIds[partitioned_block_number - 1];
				chain_id.block_count = partitioned_block_number;
				m_blocks[chain_id.last].next = chain_id.last;

				m_index.create_mapping(m_blocks[new_chain_id.first].first_hash(), new_chain_id);
			}
			//Split chain
			/*auto it = UltraCore::find_group_end(chain.begin(), chain.end(), chain.element_count() / 2, [](const byte_range & br){
				return *((size_t*)br.start);
			});

			size_t median_hash = it.hash();

			chain_info_t new_chain_id;
			new_chain_id.first = request_block();
			new_chain_id.last = new_chain_id.first;
			new_chain_id.block_count = 1;

			block_t & new_chain_block = m_blocks[new_chain_id.first];

			block_chain_t new_chain(*this, new_chain_id);
			new_chain.insert(new_chain.end(), it, chain.end());

			chain.erase(it, chain.end());

			m_index.create_mapping(median_hash, new_chain_id);*/
		}

		return true;
	}

	std::pair<iterator, bool> write_to_block(size_t block_id, size_t hash_val, const value_type & val)
	{
		block_t & block = m_blocks[block_id];

		//Find iterator to element with hash >= hash_val
		auto it = find_first_ge(block.begin(m_serializedElementSize), block.end(m_serializedElementSize), hash_val);

		//Serialize current value
		//vector<char> inserting_data(m_serializedValueSize);
		//m_serializeFun(&m_elementCache[0], val);
		m_valueStreamer.serialize(&m_elementCache[0], val);

		if (it != block.end(m_serializedElementSize))
		{
			//If we found element with similar hash, check for duplication
			for (; (it != block.end(m_serializedElementSize)) && (*it == hash_val); ++it)
			{
				if (memcmp(&m_elementCache[0], it.value_ptr(), m_valueStreamer.serialized_size()) == 0)	//Duplication detected
					return make_pair(end(), false);
			}
		}

		return make_pair(insert_into_block(block_id, block, it.element_id, hash_val, &m_elementCache[0]), true);
		//return make_pair(iterator(0, 0), false);
	}

	iterator insert_into_block(size_t block_id, block_t & block, int item_index, size_t hash_val, const void * serialized_val)
	{
		//Insert item
		char * base_ptr = block.data + item_index * m_serializedElementSize;

		if (item_index != block.item_count)	//Check if it is inserting
			memmove(base_ptr + m_serializedElementSize, base_ptr, m_serializedElementSize * (block.item_count - item_index));

		//Increase item count
		++block.item_count;

		//Then write
		*((size_t*)base_ptr) = hash_val;	//Hash
		//m_serializeFun(base_ptr + sizeof(size_t), val);	//Data
		memcpy(base_ptr + sizeof(size_t), serialized_val, m_valueStreamer.serialized_size());


		//Rebucket, if overflow
		if (load_factor(block) > m_maxLoadFactor)
		{
			//Split the block

			//Check that block contains only the same hash values
			if (*block.begin(m_serializedElementSize) == *(block.end(m_serializedElementSize) - 1))
			{
				//Then add chainging block
				int x = 0;
				throw runtime_error("Not implemented");

				//And update index tree
			}
			else  //Split the block in the middle
			{
				//print_block(block);

				auto middle_it = block.middle(m_serializedElementSize);

				//Copy all right elements to the new block
				size_t new_block_id = m_blocks.size();
				block_t new_block;
				new_block.item_count = block.item_count - middle_it.element_id;
				memcpy(new_block.data, block.data + m_serializedElementSize * middle_it.element_id, new_block.item_count * m_serializedElementSize);

				//Update old block info
				block.item_count = middle_it.element_id;

				//Update index tree
				//m_indicesTree.insert(make_pair(*middle_it, new_block_id));
				create_index_record(*middle_it, new_block_id);

				//Save the block
				m_blocks.push_back(new_block);

				//cout << "Total " << m_blocks.size() << " blocks" << std::endl;
			}
		}

		//m_blocks[block_id] = std::move(block);

		//return iterator(block_id, item_index);
		return iterator(block_id, item_index);
	}

	void append_to_block(block_t & block, size_t hash_val, const value_type & val)
	{
		if (block.item_count >= m_maxItemsInBlock)
			throw out_of_range("Trying to append more than block can handle");


		//m_serializeFun(&m_elementCache[0], val);
		m_valueStreamer.serialize(&m_elementCache[0], val);

		char * base_ptr = block.data + block.item_count * m_serializedElementSize;
		*((size_t*)base_ptr) = hash_val;	//Hash
		memcpy(base_ptr + sizeof(size_t), &m_elementCache[0], m_valueStreamer.serialized_size());	//Value
		++block.item_count;
	}

	void print_block(const block_t & block) const
	{
		auto end_it = block_iterator(block.item_count, block, m_serializedElementSize);

		for (auto it = block_iterator(0, block, m_serializedElementSize); it != end_it; ++it)
			cout << *it << ',';
		cout << std::endl;
	}
	
	float load_factor(const block_t & block) const
	{
		return (float)block.item_count / (block.vacant_count(m_serializedElementSize) + (float)block.item_count);
	}

	size_t request_block()
	{
		if (m_freedBlocks.empty())
		{
			size_t new_id = m_blockCount++;
			m_blocks.push_back(block_t(new_id));
			return new_id;
		}
		else
		{
			size_t res = m_freedBlocks.front();
			m_freedBlocks.pop();

			block_t & block = m_blocks[res];
			block.id = res;
			block.next = res;
			block.prev = res;

			return res;
		}
	}

	void delete_block(size_t block_id)
	{
		block_t & block = m_blocks[block_id];
		block.item_count = 0;
		block.next = std::numeric_limits<size_t>::max();
		block.prev = std::numeric_limits<size_t>::max();
		block.id = std::numeric_limits<size_t>::max();

		m_freedBlocks.push(block_id);
	}
protected:
	//In bytes
	const int m_serializedElementSize;
	const int m_maxItemsInBlock;
	value_streamer_t m_valueStreamer;
	hasher_t m_hasher;

	

	//map_t m_indicesTree;
	index_t m_index;
	//paged_vector_t m_blocks;
	wrapper_t m_blocks;
	const float m_maxLoadFactor;

	mutable std::vector<char> m_elementCache;

	size_t m_size, m_blockCount;
	
	std::queue<size_t> m_freedBlocks;
	//std::vector<bucket_t> m_buckets;
};


#endif
