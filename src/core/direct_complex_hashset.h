
#ifndef UltraCore_direct_complex_hashset_h
#define UltraCore_direct_complex_hashset_h

#include "complex_hashset_base.h"
#include "algorithm/algorithm.h"
#include "algorithm/merge.h"



template<typename S>
class file_storage_wrapper
{
	using storage_t = S;
	using block_t = typename S::value_type;
public:
	file_storage_wrapper()
		//:m_storage("state_database.dat")//, m_threadPool(200)
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

			//m_storage.write_range(&(*it), it->id, std::distance(it, last_gr_it));
			m_storage.write_range_async(&(*it), it->id, std::distance(it, last_gr_it));
			++write_count;

			it = last_gr_it;
		}

		while (!m_storage.ready())
		{
			cout << "Waiting data to be written..." << std::endl;
			std::this_thread::yield();
		}
		//cout << "Write I/O gain " << (float)write_count / std::distance(begin, end) << std::endl;

		/*tbb::parallel_for(begin, end, 1, [&](const block_t & block){
		m_storage.set(block.id, block);
		});*/
	}
private:
	storage_t m_storage;
	//thread_pool m_threadPool;
};


namespace direct_hashset
{
	template<typename B>
	struct ext_storage_generator
	{
		using block_t = B;
		using result = data_file<block_t>;
	};
};

template<typename T, typename S, typename H = std::hash<T>, bool UseIntMemory = true, unsigned int BlockSize = 8192U>
class direct_complex_hashset : public complex_hashset_base<T, S, H, file_storage_wrapper, direct_hashset::ext_storage_generator, BlockSize>
{
	using _Base = complex_hashset_base<T, S, H, file_storage_wrapper, direct_hashset::ext_storage_generator, BlockSize>;
	using block_t = typename _Base::block_t;
	using combined_value_t = typename _Base::combined_value_t;
public:
	//template<typename SerFun, typename DesFun>
	direct_complex_hashset(const S & ss/*, int serialized_element_size, SerFun s_fun, DesFun d_fun*/)
		:_Base(ss/*, serialized_element_size, s_fun, d_fun*/), m_writeQueue(20480)
	{
	}

	/*
	Inserts range of new elements into hashset. Calls callback for each sucess
	insertion. Is optimized for inserting range.
	*/
	typedef std::tuple<size_t, int, int> block_descr_t; //Block id + first_element_index + (last_element_index+1)

	template<typename It, typename HashFun, typename ValFun, typename CallbackFun>
	void insert_range(It begin, It end, HashFun hash_fun, ValFun val_fun, CallbackFun fun)
	{
		auto success_fun = [&](const typename It::value_type & el){
			++(this->m_size);
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
		auto it = this->m_indicesTree.empty() ? end : std::lower_bound(begin, end, this->m_indicesTree.begin()->first, [=](const typename It::value_type & val1, size_t max_val){
			return hash_fun(val1) < max_val;
		});
		create_chained_blocks(begin, it, [=](const typename It::value_type & val){
			return hash_fun(val);
		}, val_fun);
		for_each(begin, it, success_fun);

		if (!this->m_indicesTree.empty())
		{
			//Element >= max index should be added to highest block
			auto last_it = std::upper_bound(it, end, this->m_indicesTree.rbegin()->first, [=](size_t min_val, const typename It::value_type & val1){
				return min_val < hash_fun(val1) + 1;
			});
			if (last_it != end)
				block_descrs.push_back(block_descr_t(this->m_indicesTree.rbegin()->second, std::distance(begin, last_it), std::distance(begin, end)));

			//Others should be added to their equal ranges
			while (it != last_it)
			{
				auto lb = this->m_indicesTree.upper_bound(hash_fun(*it));

				if (lb == this->m_indicesTree.end())
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
			this->insert_into_chain(get<0>(block_descr));

			std::vector<combined_value_t> old_vals;

			//Deserialize old values from chain
			size_t cur_block_index = get<0>(block_descr);
			while (cur_block_index != std::numeric_limits<size_t>::max())
			{
				block_t block;// = m_blocks[cur_block_index];
				this->m_blocks.read_into(cur_block_index, block);

				size_t last_size = old_vals.size();
				old_vals.resize(last_size + block.item_count);

				for (int i = 0; i < block.item_count; ++i)
				{
					combined_value_t cur_val;
					char * base_ptr = block.data + i * this->m_serializedElementSize;
					old_vals[last_size + i].first = *((size_t*)base_ptr);
					//m_deserializeFun(base_ptr + sizeof(size_t), old_vals[last_size + i].second);
					this->m_valueStreamer.deserialize(base_ptr + sizeof(size_t), old_vals[last_size + i].second);
				}

				m_freedBlocks.push(cur_block_index);
				this->m_indicesTree.erase(block.first_hash());
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
				this->m_indicesTree.insert(make_pair(it->first_hash(), it->id));
		}

		this->m_blocks.write(m_writeQueue.begin(), last_write_it);
	}

	template<typename It, typename HashExtractor, typename ValExtractor>
	void create_chained_blocks(It begin, It end, HashExtractor hash_fun, ValExtractor val_fun)
	{
		auto gr_end_it = begin, last_gr_end = begin;
		while (gr_end_it != end)
		{
			gr_end_it = UltraCore::find_group_end(gr_end_it, end, this->m_maxItemsInBlock, [=](const typename It::value_type & val){return hash_fun(val); });

			int block_id = m_writeQueueIndex++;
			m_writeQueue[block_id].id = request_block();
			m_writeQueue[block_id].prev = std::numeric_limits<size_t>::max();
			m_writeQueue[block_id].item_count = 0;

			size_t group_size = std::distance(last_gr_end, gr_end_it);
			if (group_size > this->m_maxItemsInBlock) //Needs real chaining
			{
				//cout << "Creating chain with length " << std::distance(last_gr_end, gr_end_it) << std::endl;
				//std::vector<block_t> chain(integer_ceil(group_size, m_maxItemsInBlock));

				int prev_block_id = -1;

				for (auto it = last_gr_end; it != gr_end_it; ++it)
				{
					if (m_writeQueue[block_id].item_count == this->m_maxItemsInBlock)
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
			return this->m_blockCount++;
		else
		{
			size_t res = m_freedBlocks.front();
			m_freedBlocks.pop();
			return res;
		}
	}
private:
	std::queue<size_t> m_freedBlocks;

	std::vector<block_t> m_writeQueue;
	int m_writeQueueIndex;
};

#endif
