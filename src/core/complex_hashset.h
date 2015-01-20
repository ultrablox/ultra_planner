
#ifndef UltraCore_complex_hashset_h
#define UltraCore_complex_hashset_h

#include "utils/helpers.h"
#include "algorithm/algorithm.h"
#include "algorithm/merge.h"
#include <functional>
#include <map>
#include <stxxl.h>
#include "cached_file.h"
#include <core/utils/helpers.h>

template<typename T, typename H = std::hash<T>, bool UseIntMemory = true, unsigned int BlockSize = 8192U, typename... KeyPart>//65536U //4096U // 8192U
class complex_hashset
{
	typedef T value_type;
	typedef H hasher_t;
	typedef std::pair<size_t, value_type> combined_value_t;
	typedef std::function<void(const void*, value_type &)> deserialize_fun_type;
	typedef std::function<void(void*, const value_type &)> serialize_fun_type;
	typedef std::map<size_t, size_t> map_t;

	struct block_t;

	class block_iterator
	{
		friend class complex_hashset;
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

	typedef typename stxxl::VECTOR_GENERATOR<block_t, 1U, 131072U, BlockSize, stxxl::FR, stxxl::random>::result stxxl_paged_vector_t; //131072U //262144U //65536U
	//typedef cached_file<block_t, 65536U> cached_paged_vector_t;
	using direct_paged_vector_t = data_file<block_t>;
	typedef direct_paged_vector_t ext_paged_vector_t;
	typedef std::vector<block_t> int_paged_vector_t;
	typedef typename std::conditional<UseIntMemory, int_paged_vector_t, ext_paged_vector_t>::type paged_vector_t;

	template<typename S>
	class storage_wrapper
	{
		using storage_t = S;

	public:
		storage_wrapper()
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

	template<>
	class storage_wrapper<direct_paged_vector_t>
	{
	public:
		storage_wrapper()
			:m_storage("state_database.dat")
		{
		}

		size_t size() const
		{
			return m_storage.size();
		}

		void push_back(const block_t & block)
		{
			m_storage.append(block);
		}

		block_t & operator[](size_t index)
		{
			throw runtime_error("Invalid usage");
		}

		void read_into(size_t index, block_t & block)
		{
			m_storage.get(index, block);
		}

		template<typename It>
		void write(It begin, It end)
		{
			//cout << "Writing sequence of " << std::distance(begin, end) << " blocks." << std::endl;
			int write_count(0);

			auto it = begin;
			while (it != end)
			{
				//Find last element, where id is not sequentional
				auto last_gr_it = it + 1;

				size_t index = it->id + 1;
				while ((last_gr_it != end) && (last_gr_it->id == index))
				{
					++last_gr_it;
					++index;
				}

				m_storage.write_range(&(*it), it->id, std::distance(it, last_gr_it));
				++write_count;

				it = last_gr_it;
			}

			//cout << "Write I/O gain " << (float)write_count / std::distance(begin, end) << std::endl;
		}
	private:
		direct_paged_vector_t m_storage;
	};


	typedef storage_wrapper<paged_vector_t> wrapper_t;
public:
	class iterator
	{
		friend class complex_hashset;
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

	template<typename SerFun, typename DesFun>
	complex_hashset(int serialized_element_size, SerFun s_fun, DesFun d_fun)
		:m_serializeFun(s_fun), m_deserializeFun(d_fun), m_serializedValueSize(serialized_element_size), m_serializedElementSize(serialized_element_size + sizeof(size_t)), m_maxLoadFactor(0.9), m_elementCache(serialized_element_size), m_size(0), m_maxItemsInBlock(block_t::DataSize / m_serializedElementSize), m_blockCount(0), m_writeQueue(20480)
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
		
		size_t block_id;

		auto it = index_iterator(hash_val);

		if (it == m_indicesTree.end())
		{
			block_id = m_blocks.size();
			m_blocks.push_back(block_t());
			m_indicesTree.insert(make_pair(hash_val, block_id));
		}
		else
			block_id = it->second;

		auto res = write_to_block(block_id, hash_val, val);
		
		if (res.second)
			++m_size;
		return res;
	}


	/*
	Inserts range of new elements into hashset. Calls callback for each sucess
	insertion.
	*/
	typedef std::tuple<size_t, int, int> block_descr_t; //Block id + first_element_index + (last_element_index+1)

	template<typename It, typename HashFun, typename ValFun, typename CallbackFun>
	void insert(It begin, It end, HashFun hash_fun, ValFun val_fun, CallbackFun fun)
	{
		auto success_fun = [&](const typename It::value_type & el){
			++m_size;
			fun(el);
		};

		size_t total_count = std::distance(begin, end);

		//Allocate buffer for blocks to be writen
		//resize_if_less(m_writeQueue, block_descrs.size() * 2);
		m_writeQueueIndex = 0;

		//if (total_count == 253)
		//cout << "Inserting " << total_count << " elements into hashset" << std::endl;

		//Get ordered seqeunce of blocks
		vector<block_descr_t> block_descrs;

		//Create new blocks for all elements with hash less then min value (or all - if empty)
		auto it = m_indicesTree.empty() ? end : std::lower_bound(begin, end, m_indicesTree.begin()->first, [=](const typename It::value_type & val1, size_t max_val){
			return hash_fun(val1) < max_val;
		});
		create_chained_blocks(begin, it, [=](const typename It::value_type & val){
			return hash_fun(val);
		}, val_fun);
		for_each(begin, it, success_fun);

		if (!m_indicesTree.empty())
		{
			//Element >= max index should be added to highest block
			auto last_it = std::upper_bound(it, end, m_indicesTree.rbegin()->first, [=](size_t min_val, const typename It::value_type & val1){
				return min_val < hash_fun(val1) + 1;
			});
			if (last_it != end)
				block_descrs.push_back(block_descr_t(m_indicesTree.rbegin()->second, std::distance(begin, last_it), std::distance(begin, end)));
			
			//Others should be added to their equal ranges
			while (it != last_it)
			{
				auto lb = m_indicesTree.upper_bound(hash_fun(*it));
				
				if (lb == m_indicesTree.end())
					throw runtime_error("Impossible");

				size_t max_hash = lb->first;

				--lb;
				size_t block_id = lb->second;
				
				block_descr_t block_descr(block_id, std::distance(begin, it), 0);

				while ((it != last_it) && (hash_fun(*it) < max_hash))
					++it;

				get<2>(block_descr) = std::distance(begin, it);
				block_descrs.push_back(std::move(block_descr));
			}
		}

		//Sort blocks by their ordering
		sort_wrapper(block_descrs.begin(), block_descrs.end(), [](const block_descr_t & lhs, const block_descr_t & rhs){
			return get<0>(lhs) < get<0>(rhs);
		});

		//Now process the blocks
		for (auto & block_descr : block_descrs)
		{
			insert_into_chain(get<0>(block_descr));

			std::vector<combined_value_t> old_vals;

			//Deserialize old values from chain
			size_t cur_block_index = get<0>(block_descr);
			while (cur_block_index != std::numeric_limits<size_t>::max())
			{
				block_t block;// = m_blocks[cur_block_index];
				m_blocks.read_into(cur_block_index, block);
				
				size_t last_size = old_vals.size();
				old_vals.resize(last_size + block.item_count);

				for (int i = 0; i < block.item_count; ++i)
				{
					combined_value_t cur_val;
					char * base_ptr = block.data + i * m_serializedElementSize;
					old_vals[last_size + i].first = *((size_t*)base_ptr);
					m_deserializeFun(base_ptr + sizeof(size_t), old_vals[last_size + i].second);
				}

				m_freedBlocks.push(cur_block_index);
				m_indicesTree.erase(block.first_hash());
				cur_block_index = block.next;
			}

			//Add new ones
			typedef std::pair<combined_value_t, int> temp_val_t; //Combined value + (is_new ? new_item_index : -1)
			std::vector<temp_val_t> candidate_new_vals;

			for (int i = get<1>(block_descr); i < get<2>(block_descr); ++i)
			{
				auto it = begin + i;

				temp_val_t cur_val;
				cur_val.second = i;
				cur_val.first.first = hash_fun(*it);
				cur_val.first.second = val_fun(*it);
				candidate_new_vals.push_back(std::move(cur_val));
			}

			//Remove duplication amongst new tail
			auto last_it = UltraCore::unique(old_vals, candidate_new_vals.begin(), candidate_new_vals.end(), [](const combined_value_t & val){
				return val.first;
			}, [](const temp_val_t & val){
				return val.first.first;
			}, [](const combined_value_t & old_val, const temp_val_t & new_val){
				return old_val.second == new_val.first.second;
			});

			size_t old_vals_count = old_vals.size();
			for (auto it = candidate_new_vals.begin(); it != last_it; ++it)
			{
				old_vals.push_back(it->first);
				success_fun(*(begin + it->second));
			}

			//Merge with old data
			auto cmp = [](const combined_value_t & left, const combined_value_t & right){
				return left.first < right.first;
			};

			std::inplace_merge(old_vals.begin(), old_vals.begin() + old_vals_count, old_vals.end(), cmp);

			create_chained_blocks(old_vals.begin(), old_vals.end(), [](const combined_value_t & val){
				return val.first;
			}, [](const combined_value_t & val){
				return val.second;
			});
		}

		//cout << m_writeQueueIndex << std::endl;
		//Process write queue
		auto last_write_it = m_writeQueue.begin() + m_writeQueueIndex;
		sort_wrapper(m_writeQueue.begin(), last_write_it, [](const block_t & b1, const block_t & b2){
			return b1.id < b2.id;
		});

		for (auto it = m_writeQueue.begin(); it != last_write_it; ++it)
		{
			//Update index only for chain beginnings
			if (it->prev == std::numeric_limits<size_t>::max())
				m_indicesTree.insert(make_pair(it->first_hash(), it->id));
		}

		m_blocks.write(m_writeQueue.begin(), last_write_it);
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
		auto index_it = index_iterator(hash_val);
		
		if (index_it == m_indicesTree.end())
			return end();

		const block_t & block = m_blocks[index_it->second];

		//print_block(block);

		//Find iterator to element with hash >= hash_val
		auto end_it = block_iterator(block.item_count, block, m_serializedElementSize);
		auto it = std::lower_bound(block_iterator(0, block, m_serializedElementSize), end_it, hash_val);

		//Serialize searching value
		//vector<char> inserting_data(m_serializedValueSize);
		m_serializeFun(&m_elementCache[0], val);

		for (; (*it == hash_val) && (it != end_it); ++it)
		{
			if (memcmp(&m_elementCache[0], it.value_ptr(), m_serializedValueSize) == 0)
				return iterator(index_it->second, it.element_id);
		}

		return end();
	}

	size_t block_count() const
	{
		return m_blocks.size();
	}
private:
	void insert_into_chain(size_t first_block_id)
	{

	}
	
	map_t::const_iterator index_iterator(size_t hash_val) const
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
	}

	std::pair<iterator, bool> write_to_block(size_t block_id, size_t hash_val, const value_type & val)
	{
		block_t & block = m_blocks[block_id];

		//Find iterator to element with hash >= hash_val
		auto it = find_first_ge(block.begin(m_serializedElementSize), block.end(m_serializedElementSize), hash_val);

		//Serialize current value
		//vector<char> inserting_data(m_serializedValueSize);
		m_serializeFun(&m_elementCache[0], val);

		if (it != block.end(m_serializedElementSize))
		{
			size_t cur_hash = *it;
			if (cur_hash == hash_val)//If we found element with similar hash
			{
				for (; (it != block.end(m_serializedElementSize)) && (*it == hash_val); ++it)//Check for duplication
				{
					if (memcmp(&m_elementCache[0], it.value_ptr(), m_serializedValueSize) == 0)	//Duplication detected
						return make_pair(end(), false);
				}
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
		memcpy(base_ptr + sizeof(size_t), serialized_val, m_serializedValueSize);


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
				m_indicesTree.insert(make_pair(*middle_it, new_block_id));

				//Save the block
				m_blocks.push_back(new_block);

				//cout << "Total " << m_blocks.size() << " blocks" << std::endl;
			}
		}

		//m_blocks[block_id] = std::move(block);

		//return iterator(block_id, item_index);
		return iterator(block_id, item_index);
	}

	template<typename It, typename HashExtractor, typename ValExtractor>
	void create_chained_blocks(It begin, It end, HashExtractor hash_fun, ValExtractor val_fun)
	{
		auto gr_end_it = begin, last_gr_end = begin;
		while (gr_end_it != end)
		{
			gr_end_it = UltraCore::find_group_end(gr_end_it, end, m_maxItemsInBlock, [=](const typename It::value_type & val){return hash_fun(val); });
		
			int block_id = m_writeQueueIndex++;
			m_writeQueue[block_id].id = request_block();
			m_writeQueue[block_id].prev = std::numeric_limits<size_t>::max();
			m_writeQueue[block_id].item_count = 0;

			size_t group_size = std::distance(last_gr_end, gr_end_it);
			if (group_size > m_maxItemsInBlock) //Needs real chaining
			{
				//cout << "Creating chain with length " << std::distance(last_gr_end, gr_end_it) << std::endl;
				//std::vector<block_t> chain(integer_ceil(group_size, m_maxItemsInBlock));

				int prev_block_id = -1;
				
				for(auto it = last_gr_end; it != gr_end_it; ++it)
				{
					if (m_writeQueue[block_id].item_count == m_maxItemsInBlock)
					{
						prev_block_id = block_id;
						block_id = m_writeQueueIndex++;
						m_writeQueue[block_id].id = request_block();
						if (prev_block_id != -1)
						{
							m_writeQueue[block_id].item_count = 0;
							m_writeQueue[block_id].prev = m_writeQueue[prev_block_id].id;
							m_writeQueue[prev_block_id].next = m_writeQueue[block_id].id;
						}
					}
	
					append_to_block(m_writeQueue[block_id], hash_fun(*it), val_fun(*it));
				}
			}
			else
			{
				m_writeQueue[block_id].next = std::numeric_limits<size_t>::max();

				for (auto it = last_gr_end; it != gr_end_it; ++it)
					append_to_block(m_writeQueue[block_id], hash_fun(*it), val_fun(*it));
			}

			last_gr_end = gr_end_it;
		}
	}

	size_t request_block()
	{
		if (m_freedBlocks.empty())
			return m_blockCount++;
		else
		{
			size_t res = m_freedBlocks.front();
			m_freedBlocks.pop();
			return res;
		}
	}
	void append_to_block(block_t & block, size_t hash_val, const value_type & val)
	{
		if (block.item_count >= m_maxItemsInBlock)
			throw out_of_range("Trying to append more than block can handle");

		m_serializeFun(&m_elementCache[0], val);

		char * base_ptr = block.data + block.item_count * m_serializedElementSize;
		*((size_t*)base_ptr) = hash_val;	//Hash
		memcpy(base_ptr + sizeof(size_t), &m_elementCache[0], m_serializedValueSize);	//Value
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

/*
	template<typename St>
	void init_storage(St & str)
	{}

	template<>
	void init_storage<cached_paged_vector_t>(cached_paged_vector_t & str)
	{
		str.open("state_database.dat");
	}
*/
private:
	hasher_t m_hasher;
	serialize_fun_type m_serializeFun;
	deserialize_fun_type m_deserializeFun;

	//In bytes
	const int m_serializedElementSize, m_serializedValueSize;

	map_t m_indicesTree;
	//paged_vector_t m_blocks;
	wrapper_t m_blocks;
	const float m_maxLoadFactor;

	mutable std::vector<char> m_elementCache;

	size_t m_size, m_blockCount;
	const int m_maxItemsInBlock;
	std::queue<size_t> m_freedBlocks;

	std::vector<block_t> m_writeQueue;
	int m_writeQueueIndex;
};

#endif
