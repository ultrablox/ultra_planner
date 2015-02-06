
#ifndef UltraCore_direct_complex_hashset_h
#define UltraCore_direct_complex_hashset_h

#include "complex_hashset_base.h"
#include "../algorithm/algorithm.h"
#include "../algorithm/merge.h"


template<typename S>
class file_storage_wrapper
{
	using storage_t = S;
public:
	using block_t = typename S::value_type;

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
		int r = m_storage.get(index, block);
		if (r)
			cout << "Read failed!" << std::endl;
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

template<typename T, typename S, typename W, typename H = std::hash<T>>
class direct_complex_hashset : public complex_hashset_base<T, S, W, H>
{
	using _Base = complex_hashset_base<T, S, W, H>;
	using block_t = typename _Base::block_t;
	using combined_value_t = typename _Base::combined_value_t;
//	using chain_info_t = range_map_wrapper::chain_info_t;
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
	typedef std::tuple<size_t, int, int, size_t, size_t> block_descr_t; //Block id + first_element_index + (last_element_index+1) + block_count

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

		auto it = begin;
		while (it != end)
		{
			/*size_t group_start_hash = this->m_index.expected_block_min_hash(hash_fun(*it));
			auto last_it = std::find_if_not(it, end, [&](const typename It::value_type & el){
				return group_start_hash == this->m_index.expected_block_min_hash(hash_fun(el));
			});

			auto chain = this->m_index.chain_with_hash(group_start_hash);
			block_descrs.push_back(block_descr_t(chain.first, std::distance(begin, it), std::distance(begin, last_it), group_start_hash, chain.block_count));
			
			it = last_it;*/
		}

		for (auto & bd : block_descrs)
		{
			//vector<combined_value_t> elements_buffer(get<2>(bd) - get<1>(bd) + get<4>(bd) * m_maxItemsInBlock);

			resize_if_less(m_elementsBuffer, get<2>(bd) -get<1>(bd) +get<4>(bd) * this->m_maxItemsInBlock);
			auto chain = merge_into_chain(get<0>(bd), begin + get<1>(bd), begin + get<2>(bd), hash_fun, val_fun, success_fun, m_elementsBuffer);
			this->m_index.update_mapping(get<3>(bd), chain);
		}

		//Create new blocks for all elements with hash less then min value (or all - if empty)
		/*auto it = this->m_indicesTree.empty() ? end : std::lower_bound(begin, end, this->m_indicesTree.begin()->first, [=](const typename It::value_type & val1, size_t max_val){
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

		//cout << m_writeQueueIndex << std::endl;*/
		//Process write queue
		auto last_write_it = m_writeQueue.begin() + m_writeQueueIndex;
		sort_wrapper(m_writeQueue.begin(), last_write_it, [](const block_t & b1, const block_t & b2){
			return b1.id < b2.id;
		});

		this->m_blocks.write(m_writeQueue.begin(), last_write_it);
	}
	template<typename It, typename HashExtractor, typename ValExtractor, typename Callback>
	chain_info_t merge_into_chain(size_t start_block_id, It begin, It end, HashExtractor hash_fun, ValExtractor val_fun, Callback cb_fun, vector<combined_value_t> & chain_elements_buffer)
	{
		if (start_block_id == std::numeric_limits<size_t>::max())
		{
			size_t block_count = 1, last_block_global_id = start_block_id;
			start_block_id = this->request_block();
			size_t local_block_id = m_writeQueueIndex++;
			m_writeQueue[local_block_id].set_meta(start_block_id, start_block_id, start_block_id, 0);

			auto it = begin;
			while (it != end)
			{
				append_to_block(m_writeQueue[local_block_id], hash_fun(*it), val_fun(*it));
				if (m_writeQueue[local_block_id].item_count() == this->m_maxItemsInBlock)
				{
					last_block_global_id = m_writeQueue[local_block_id].id;
					size_t new_block_id = this->request_block();
					size_t prev_block_id = local_block_id;
					local_block_id = m_writeQueueIndex++;
					m_writeQueue[local_block_id].set_meta(new_block_id, prev_block_id, new_block_id, 0);
					m_writeQueue[prev_block_id].set_next(new_block_id);
					++block_count;
				}

				cb_fun(*it++);
			}

			return chain_info_t(start_block_id, last_block_global_id, block_count);
		}
		else
		{
			vector<size_t> local_blocks;

			//Deserialize all the chain....

			size_t block_id = start_block_id, last_block_id = start_block_id;
			size_t element_buffer_index = 0;
			do
			{
				size_t local_block_id = m_writeQueueIndex++;
				local_blocks.push_back(local_block_id);

				block_t & block = m_writeQueue[local_block_id];
				memset(&block, 0, sizeof(block_t));
				this->m_blocks.read_into(block_id, block);
				

				for (int i = 0; i < block.item_count(); ++i)
				{
					combined_value_t & val = chain_elements_buffer[element_buffer_index++];
					const char * base_ptr = block.data + i * this->m_serializedElementSize;
					val.first = *((size_t*)base_ptr);
					this->m_valueStreamer.deserialize(base_ptr + sizeof(size_t), val.second);
				}

				m_writeQueue[local_block_id].set_item_count(0);
				last_block_id = block_id;
				block_id = m_writeQueue[local_block_id].next();
			} while (block_id != last_block_id);

			//Remove duplication amongst news
			auto last_it = begin;/* UltraCore::unique(chain_elements_buffer.begin(), chain_elements_buffer.begin() + element_buffer_index, begin, end, [](const combined_value_t & val){
				return val.first;
			}, [=](const typename It::value_type & val){
				return hash_fun(val);
			}, [=](const combined_value_t & old_val, const typename It::value_type & new_val){
				return old_val.second == val_fun(new_val);
			});*/

			//Merge
			size_t old_vals_count = element_buffer_index;
			for (auto it = begin; it != last_it; ++it)
			{
				cb_fun(*it);
				combined_value_t & val = chain_elements_buffer[element_buffer_index++];
				//chain_elements.push_back(make_pair(hash_fun(*it), val_fun(*it)));		
				val.first = hash_fun(*it);
				val.second = val_fun(*it);
			}

			//Merge with old data
			std::inplace_merge(chain_elements_buffer.begin(), chain_elements_buffer.begin() + old_vals_count, chain_elements_buffer.begin() + element_buffer_index, [](const combined_value_t & left, const combined_value_t & right){
				return left.first < right.first;
			});

			//And serialize back
			auto local_block_it = local_blocks.begin();
			bool writing_new = false;
			size_t local_block_id = *local_block_it;

			size_t block_count = 1, last_block_global_id = m_writeQueue[local_block_id].id;

			//for (auto & el : chain_elements)
			for (auto el_it = chain_elements_buffer.begin(); el_it != chain_elements_buffer.begin() + element_buffer_index; ++el_it)
			{
				auto & el = *el_it;
				append_to_block(m_writeQueue[local_block_id], el.first, el.second);
				if (m_writeQueue[local_block_id].item_count() == this->m_maxItemsInBlock)
				{
					size_t last_local_block_id = local_block_id;

					if (writing_new)
					{
						size_t new_block_id = this->request_block();
						local_block_id = m_writeQueueIndex++;
						m_writeQueue[local_block_id].set_meta(new_block_id, m_writeQueue[last_local_block_id].id, new_block_id, 0);
						m_writeQueue[last_local_block_id].set_next(m_writeQueue[local_block_id].id);
					}
					else
					{
						++local_block_it;
						if (local_block_it == local_blocks.end())
						{
							writing_new = true;
							size_t new_block_id = this->request_block();
							local_block_id = m_writeQueueIndex++;
							m_writeQueue[local_block_id].set_meta(new_block_id, m_writeQueue[last_local_block_id].id, new_block_id, 0);
							m_writeQueue[last_local_block_id].set_next(m_writeQueue[local_block_id].id);
						}
						else
							local_block_id = *local_block_it;
					}

					last_block_global_id = m_writeQueue[local_block_id].id;

					++block_count;
				}
			}
			
			return chain_info_t(local_blocks[0], last_block_global_id, block_count);	//make_pair(local_blocks[0], local_block_id);
				
		}
		
	}

	template<typename It, typename HashExtractor, typename ValExtractor>
	void create_chained_blocks(It begin, It end, HashExtractor hash_fun, ValExtractor val_fun)
	{
		auto gr_end_it = begin, last_gr_end = begin;
		while (gr_end_it != end)
		{
			gr_end_it = UltraCore::find_group_end(gr_end_it, end, this->m_maxItemsInBlock, [=](const typename It::value_type & val){return hash_fun(val); });

			int block_id = m_writeQueueIndex++;
			m_writeQueue[block_id].id = this->request_block();
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
						m_writeQueue[block_id].id = this->request_block();
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

private:

	std::vector<block_t> m_writeQueue;
	int m_writeQueueIndex;
	vector<combined_value_t> m_elementsBuffer;
};

#endif
