
#ifndef UltraCore_complex_hashset_base_h
#define UltraCore_complex_hashset_base_h

#include "block_chain.h"
#include "../utils/helpers.h"
#include "../algorithm/algorithm.h"
#include "../algorithm/merge.h"
#include "../cached_file.h"
#include <functional>
#include <map>
#include <stxxl.h>
#include "../avl_tree.h"
#include "block.h"
#include <core/byte_range.h>
#include <core/delayed_buffer.h>
#include <stxxl/unordered_map>
#include <unordered_map>
//#include <tbb/parallel_for.h>
#include <thread>
#include <stdexcept>

class range_map_wrapper
{
public:

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

	chain_info_t & chain_with_hash(size_t hash_val, size_t * p_last_hash = nullptr)
	{
		auto it = m_rtree.find(hash_val);
		if (p_last_hash)
		{
			auto next_it = it + 1;
			if (next_it == m_rtree.end())
				*p_last_hash = std::numeric_limits<size_t>::max();
			else
				*p_last_hash = *next_it;
		}
		return it.data();
	}
	
	size_t interval_begin(size_t hash_val)
	{
		auto it = m_rtree.find(hash_val);
		return *it;
	}

	void create_mapping(size_t hash_val, const chain_info_t & chain_id)
	{
		if(chain_id.first == std::numeric_limits<size_t>::max())
			throw runtime_error("Invalid mapping");
		//cout << "mapping: " << hash_val << ", chain:" << chain_id << std::endl;
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
	int max_chain_length, io_request_count;
	double average_chain_length;
	float density, cache_load;
	size_t block_count;
	cached_file_stats_t file_stats;

	friend std::ostream & operator<<(std::ostream & os, const hashset_stats_t & stats)
	{
		os << "Max chain length: " << stats.max_chain_length << std::endl;
		os << "Average chain length: " << stats.average_chain_length << std::endl;
		os << "Density: " << stats.density * 100 << "%" << std::endl;
		os << "Block Count: " << stats.block_count << std::endl;
		os << "I/O request count: " << stats.io_request_count << std::endl;
		os << "Cache load: " << stats.cache_load * 100.0f << '%' << std::endl;
		os << stats.file_stats;
		return os;
	}
};

template<typename T, typename S, typename W, typename H>
class complex_hashset_base
{
protected:
	typedef T value_type;
	using value_streamer_t = S;
	typedef H hasher_t;
	typedef std::pair<size_t, value_type> combined_value_t;
	using wrapper_t = W;
	using block_t = typename wrapper_t::block_t; //using block_t = hashset_block<BlockSize>;
	
	using index_t = range_map_wrapper; 	//Maps hash_value => block chain where it should be
	//using chain_info_t = typename index_t::chain_info_t;

	static const int MaxBlocksPerChain = 2;

	
	using delayed_buffer_t = delayed_buffer<value_type>;

	static_assert(sizeof(block_t) % 4096 == 0, "Invalid block_t size!");
	
	/*using block_const_iterator = block_iterator<const block_chain_t>;
	using block_reverse_const_iterator = block_reverse_iterator<const block_chain_t>;
	using block_iterator = block_iterator<block_chain_t>;
	using block_reverse_iterator = block_reverse_iterator<block_chain_t>;*/

	/*typedef typename stxxl::VECTOR_GENERATOR<block_t, 1U, 131072U, BlockSize, stxxl::FR, stxxl::random>::result stxxl_paged_vector_t;
	//typedef cached_file<block_t, 65536U> cached_paged_vector_t;
	using direct_paged_vector_t = data_file<block_t>;
	typedef direct_paged_vector_t ext_paged_vector_t;
	typedef std::vector<block_t> int_paged_vector_t;*/
	//typedef typename std::conditional<UseIntMemory, int_paged_vector_t, ext_paged_vector_t>::type paged_vector_t;
	//typedef storage_wrapper<paged_vector_t> wrapper_t;

	//using wrapper_t = typename std::conditional<UseIntMemory, W<int_paged_vector_t>, W<direct_paged_vector_t>>::type;
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
		:m_valueStreamer(vs), m_serializedElementSize(vs.serialized_size() + sizeof(size_t)), m_maxLoadFactor(0.9), m_elementCache(sizeof(size_t)+m_valueStreamer.serialized_size()), m_size(0), m_maxItemsInBlock(block_t::DataSize / m_serializedElementSize), m_blockCount(0)
	{
		if (m_serializedElementSize > block_t::DataSize)
			throw runtime_error("Serialized element is bigger than page size");

		cout << "Hashset block_size: " << sizeof(block_t) << ", DataSize " << block_t::DataSize << std::endl;
		cout << "Hashset page can store " << m_maxItemsInBlock << " elements." << std::endl;
	}

	complex_hashset_base(const complex_hashset_base & rhs)
		:m_serializedElementSize(0), m_maxLoadFactor(0), m_maxItemsInBlock(0), m_valueStreamer(rhs.m_valueStreamer)
	{
	}

	iterator end() const
	{
		return iterator(m_blocks.size(), 0);
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
		res.density = (float)m_size / (block_count() * m_maxItemsInBlock);
		res.block_count = m_blocks.size();

		m_index.compute_stats(res.max_chain_length);
		res.average_chain_length = (double)m_blocks.size() / m_index.size();
		//res.io_request_count = m_blocks.io_request_count();
		res.cache_load = (float)m_blocks.size() / m_blocks.cache_size();
//		res.file_stats = m_blocks.stats();

		return res;
	}

	size_t block_count() const
	{
		return m_blocks.size();
	}

	void print_debug()
	{
/*		auto chains = m_index.chains();
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

		}*/
	}
protected:
	
	/*template<typename It, typename HashExtractor, typename ValueExtractor, typename Callback>
	void insert_small_into_chain(block_chain_t & chain, It begin_it, It end_it, HashExtractor hash_fun, ValueExtractor ser_val_fun, Callback call_fun)
	{
		for (auto it = begin_it; it != end_it; ++it)
		{
			if (insert_into_chain(chain, hash_fun(*it), ser_val_fun(*it)))
				call_fun(*it);
		}
	}*/

/*	template<typename It, typename HashExtractor, typename ValueExtractor, typename Callback>
	void insert_into_chain(block_chain_t & chain, It begin_it, It end_it, HashExtractor hash_fun, ValueExtractor ser_val_fun, Callback call_fun)
	{
		//Remove duplication amongst news
		size_t inserting_size = std::distance(begin_it, end_it);
		auto last_it = UltraCore::unique(chain.begin(), chain.end(), begin_it, end_it, [](const byte_range & br){
			return br.begin_as<size_t>();
		}, [=](const typename It::value_type & val){
			return hash_fun(val);
		}, [=](const byte_range & old_val, const typename It::value_type & new_val){
			return old_val == ser_val_fun(new_val);
		});
		
		//if (last_it != end_it)
		//	cout << "Int unique removed " << 100.0f * (float)std::distance(last_it, end_it) / std::distance(begin_it, end_it) << "% values" << std::endl;
		
		//chain.print();
		
		
		//Add to chain what's left
		size_t old_size = chain.element_count();
		size_t merging_size = std::distance(begin_it, last_it);
		chain.resize(chain.element_count() + merging_size);
*/
		//Copy values
		/*auto dest_it = chain.begin() + old_size;
		for (It it = begin_it; it != last_it; ++it, ++dest_it)
		{
			call_fun(*it);
			*dest_it = ser_val_fun(*it);
		}

		chain.print();*/
	
		//Merge them
		/*cout << "Adding internaly merged: ";
		for (auto it = begin_it; it != last_it; ++it)
			cout << ser_val_fun(*it).begin_as<size_t>() << ',';
		cout << std::endl;*/
//		std::reverse(begin_it, last_it);
		/*cout << "Adding internaly merged: ";
		for (auto it = begin_it; it != last_it; ++it)
		{
			cout << ser_val_fun(*it).begin_as<size_t>() << ',';
			if (hash_fun(*it) == 50)
				cout << "!";
		}
		cout << std::endl;*/

		/*UltraCore::merge(chain.rbegin() + merging_size, chain.rend(), begin_it, last_it, chain.rbegin(), [](const byte_range & lhs, const byte_range & rhs){
			return lhs.begin_as<size_t>() > rhs.begin_as<size_t>();
		});*/

/*		std::merge(chain.rbegin() + merging_size, chain.rend(), begin_it, last_it, chain.rbegin(), [](const byte_range & lhs, const byte_range & rhs){
			return lhs.begin_as<size_t>() > rhs.begin_as<size_t>();
		});
		
		for (auto it = begin_it; it != last_it; ++it)
			call_fun(*it);
*/		

		/*for (auto it = begin_it; it != last_it; ++it)
		{
			if(insert_into_chain(chain, hash_fun(*it), ser_val_fun(*it)))
				call_fun(*it);
		}*/
		//chain.print();
//	}


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
		if (block.item_count() >= m_maxItemsInBlock)
			throw out_of_range("Trying to append more than block can handle");


		//m_serializeFun(&m_elementCache[0], val);
		m_valueStreamer.serialize(&m_elementCache[0], val);

		char * base_ptr = block.data + block.item_count() * m_serializedElementSize;
		*((size_t*)base_ptr) = hash_val;	//Hash
		memcpy(base_ptr + sizeof(size_t), &m_elementCache[0], m_valueStreamer.serialized_size());	//Value
		block.inc_item_count();
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

	template<typename ChainT>
	iterator find_in_chain(const ChainT & chain, const value_type & val, size_t hash_val) const
	{
		auto it = find_first_ge(chain.cbegin(), chain.cend(), hash_val);


		if (it != chain.cend())	//If we found element with similar hash, check for duplication
		{
			m_valueStreamer.serialize(&m_elementCache[sizeof(size_t)], val);
			memcpy(&m_elementCache[0], &hash_val, sizeof(size_t));
			byte_range searchin_element_br(&m_elementCache[0], m_serializedElementSize);

			for (; (it != chain.cend()) && ((*it).template begin_as<size_t>() == hash_val); ++it)
			{
				if (searchin_element_br == *it)	//Duplication detected
				{
					//auto addr = chain.element_address(it.elementId());
					return iterator(chain.m_blocksData[it.address().block].pBlock->meta.id, it.address().element);
				}
			}
		}

		return end();
	}

	template<typename ChainT>
	bool try_ballance_chain(ChainT & chain, chain_info_t & new_chain_id, size_t * p_new_chain_first_hash = nullptr)
	{
		if (chain.limits.block_count <= MaxBlocksPerChain)
			return false;

		if ((chain.limits.block_count == MaxBlocksPerChain) && (chain.density() < 0.96))
			return false;

		if (chain.m_blocksData.begin()->pBlock->first_hash() == chain.m_blocksData.rbegin()->pBlock->last_hash(chain.element_size()))
			return false;
		
		int partitioned_block_number = chain.partitioned_block();
		if (partitioned_block_number != 0)
		{
			//Fast split the chain
			new_chain_id.first = chain.m_blocksData[partitioned_block_number].pBlock->meta.id;
			new_chain_id.last = chain.limits.last;
			new_chain_id.block_count = chain.m_blocksData.size() - partitioned_block_number;

			chain.m_blocksData[partitioned_block_number].pBlock->meta.prev = new_chain_id.first;	//m_blocks[new_chain_id.first].meta.prev = new_chain_id.first;
			chain.m_blocksData[partitioned_block_number].modified = true; //m_blocks.set_modified(new_chain_id.first);

			chain.limits.last = chain.m_blocksData[partitioned_block_number - 1].pBlock->meta.id;
			chain.limits.block_count = partitioned_block_number;
			chain.m_blocksData[partitioned_block_number - 1].pBlock->set_next(chain.limits.last);//m_blocks[chain.limits.last].set_next(chain.limits.last);
			chain.m_blocksData[partitioned_block_number - 1].modified = true;  //m_blocks.set_modified(chain.limits.last);

			if (p_new_chain_first_hash)
				*p_new_chain_first_hash = chain.m_blocksData[partitioned_block_number].pBlock->first_hash();

			chain.remove_last_blocks(chain.m_blocksData.size() - partitioned_block_number);
			return true;
		}
		else
		{
			//cout << "Failed to partition chain" << std::endl;
			return false;
		}
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

	size_t m_blockCount;
	std::atomic<size_t> m_size;
};


#endif
