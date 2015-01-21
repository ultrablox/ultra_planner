
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
//#include <tbb/parallel_for.h>
#include <thread>


template<typename T, typename S, typename H, template<typename> class W, template<typename> class SG, unsigned int BlockSize>//65536U //4096U // 8192U
class complex_hashset_base
{
protected:
	typedef T value_type;
	using value_streamer_t = S;
	typedef H hasher_t;
	typedef std::pair<size_t, value_type> combined_value_t;
	typedef std::map<size_t, size_t> map_t;
	using bucket_t = map_t;
	static const int MaxNodesPerBucket = 500;
	struct block_t;

	class block_iterator
	{
		friend class complex_hashset_base;
	public:
		typedef forward_iterator_tag iterator_category;
		typedef size_t value_type;
		typedef ptrdiff_t difference_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		block_iterator(int _element_id, const block_t & _block, int el_size)
			:element_id(_element_id), block(_block), element_size(el_size)
		{}

		//Returns hash of current element
		const size_t & operator*() const
		{
			return *(size_t*)(block.data + element_id * element_size);
		}

		//Prefix
		block_iterator& operator++()
		{
			++element_id;
			return *this;
		}

		//Prefix
		block_iterator& operator--()
		{
			--element_id;
			return *this;
		}

		block_iterator& operator+=(size_t delta)
		{
			element_id += delta;
			return *this;
		}

		block_iterator& operator=(const block_iterator& rhs)
		{
			this->element_id = rhs.element_id;
			return *this;
		}

		friend bool operator!=(const block_iterator & lhs, const block_iterator & rhs)
		{
			return lhs.element_id != rhs.element_id;
		}

		friend bool operator==(const block_iterator & lhs, const block_iterator & rhs)
		{
			return lhs.element_id == rhs.element_id;
		}

		friend block_iterator operator+(const block_iterator & lhs, size_t delta)
		{
			return block_iterator(lhs.element_id + delta, lhs.block, lhs.element_size);
		}

		friend block_iterator operator-(const block_iterator & lhs, size_t delta)
		{
			return block_iterator(lhs.element_id - delta, lhs.block, lhs.element_size);
		}

		friend size_t operator-(const block_iterator & lhs, const block_iterator & rhs)
		{
			return lhs.element_id - rhs.element_id;
		}

		const void * value_ptr() const
		{
			return block.data + element_id*element_size + sizeof(size_t);
		}
	private:
		int element_id;
		const int element_size;
		const block_t & block;
	};

	struct block_t
	{
		static const size_t Size = BlockSize;
		static const size_t DataSize = Size - 3*sizeof(size_t)-sizeof(int);

		block_t()
			:item_count(0), next(numeric_limits<size_t>::max()), prev(numeric_limits<size_t>::max())
		{
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

		size_t id, next, prev;
		int item_count;
		char data[DataSize];

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
		iterator(size_t _block_id, int _element_id)
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
		size_t block_id;
		int element_id;
	};

	complex_hashset_base(const value_streamer_t & vs)
		:m_valueStreamer(vs), m_serializedElementSize(m_valueStreamer.serialized_size() + sizeof(size_t)), m_maxLoadFactor(0.9), m_elementCache(m_valueStreamer.serialized_size()), m_size(0), m_maxItemsInBlock(block_t::DataSize / m_valueStreamer.serialized_size()), m_blockCount(0)//, m_buckets(1)
	{
		if (m_serializedElementSize > block_t::DataSize)
			throw runtime_error("Serialized element is bigger than page size");

		cout << "Hashset page can store " << m_maxItemsInBlock << " elements." << std::endl;
	}

	iterator end() const
	{
		return iterator(m_blocks.size(), -1);
	}

	//It will work faster, if you know hash in advance
	std::pair<iterator, bool> insert(const value_type & val)
	{
		size_t hash_val = m_hasher(val);

		return insert(val, hash_val);
	}

	std::pair<iterator, bool> insert(const value_type & val, size_t hash_val)
	{
		//Find appropriate block, create if it does not exists
		
		size_t block_id = block_with_hash(hash_val);

		if (block_id == std::numeric_limits<size_t>::max())
		{
			block_id = m_blocks.size();
			m_blocks.push_back(block_t());
			create_index_record(hash_val, block_id);
		}

		auto res = write_to_block(block_id, hash_val, val);
		
		if (res.second)
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

	float bucket_factor() const
	{
		return (float)m_size / (block_count() * (block_t::DataSize / m_serializedElementSize));
	}

	iterator find(const value_type & val) const
	{
		size_t hash_val = m_hasher(val);
		/*auto index_it = index_iterator(hash_val);
		
		if (index_it == m_indicesTree.end())
			return end();*/
		size_t block_id = block_with_hash(hash_val);
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
			if (memcmp(&m_elementCache[0], it.value_ptr(), m_valueStreamer.serialized_size()/*m_serializedValueSize*/) == 0)
				return iterator(block_id, it.element_id);
		}

		return end();
	}

	size_t block_count() const
	{
		return m_blocks.size();
	}
protected:
	void insert_into_chain(size_t first_block_id)
	{

	}
	
	size_t block_with_hash(size_t hash_val) const
	{
		//bucket_t & bucket = bucket_with_hash(hash_val);

		auto it = m_indicesTree.lower_bound(hash_val);

		if (it == m_indicesTree.end())
			return std::numeric_limits<size_t>::max();
		else
		{
			if (it == m_indicesTree.begin())
				return std::numeric_limits<size_t>::max();
			else
			{
				--it;
				return it->second;
			}
		}
	}

	void create_index_record(size_t hash_val, size_t block_id)
	{
		/*bucket_t & bucket = bucket_with_hash(hash_val);

		bucket.insert(make_pair(hash_val, block_id));

		if (bucket.size() > MaxNodesPerBucket)
			split_buckets();*/
		m_indicesTree.insert(make_pair(hash_val, block_id));
	}
	
	bucket_t & bucket_with_hash(size_t hash_val)
	{
		int bucket_id = hash_val % m_buckets.size();
		return m_buckets[bucket_id];
	}

	void split_buckets()
	{
		int x = 0;
	}

	/*map_t::const_iterator index_iterator(size_t hash_val) const
	{
		auto it = m_indicesTree.lower_bound(hash_val);

		if (it == m_indicesTree.end())
			it = m_indicesTree.end();//create_new = true;
		else if (it->first == hash_val)
		{
			//block_id = it->second;
		}
		else
		{
			if (it == m_indicesTree.begin())
				it = m_indicesTree.end();
			else
			{
				--it;
				//block_id = it->second;
			}
		}

		return it;
	}*/

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

protected:
	value_streamer_t m_valueStreamer;
	hasher_t m_hasher;

	//In bytes
	const int m_serializedElementSize;

	map_t m_indicesTree;
	//paged_vector_t m_blocks;
	wrapper_t m_blocks;
	const float m_maxLoadFactor;

	mutable std::vector<char> m_elementCache;

	size_t m_size, m_blockCount;
	const int m_maxItemsInBlock;

	//std::vector<bucket_t> m_buckets;
};


#endif
