
#ifndef UltraCore_delayed_complex_hashset_h
#define UltraCore_delayed_complex_hashset_h

#include "complex_hashset_base.h"
#include "../delayed_buffer.h"
#include "../thread_pool.h"
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <thread>

template<typename T>
class double_buffer
{
	struct element_t
	{
		T container;
		std::mutex mtx;
	};
public:
	using container_t = T;

	double_buffer()
		:m_pFirst(new element_t), m_pSecond(new element_t)
	{}

	~double_buffer()
	{
		delete m_pFirst;
		delete m_pSecond;
	}

	void swap_buffers()
	{
		std::lock(m_pFirst->mtx, m_pSecond->mtx);
		std::lock_guard<std::mutex> lk1(m_pFirst->mtx, std::adopt_lock);
		std::lock_guard<std::mutex> lk2(m_pSecond->mtx, std::adopt_lock);
		std::swap(m_pFirst, m_pSecond);
	}

	template<typename Fun>
	void safe_access_first(Fun fun)
	{
		safe_access_container(m_pFirst, fun);
	}

	template<typename Fun>
	void safe_access_second(Fun fun)
	{
		safe_access_container(m_pSecond, fun);
	}
private:
	template<typename Fun>
	void safe_access_container(element_t * el, Fun fun)
	{
		std::lock_guard<std::mutex> lock(el->mtx);
		fun(el->container);
	}
private:
	element_t * m_pFirst, *m_pSecond;
};

template<typename T, typename S, typename W, typename H = std::hash<T>>
class delayed_complex_hashset: public complex_hashset_base<T, S, W, H>
{
	using _Base = complex_hashset_base<T, S, W, H>;
	using storage_t = W;
	using insertion_request_t = insertion_request<T>;
	using request_container_t = std::vector<insertion_request_t>;

	struct merge_range_descr_t
	{
		chain_info_t * p_info;
		int first, last;
		int first_buffer_block;
	};

	struct buffered_block_t
	{
		block_t data;
		bool modified;
	};

	using blocks_buffer_t = std::vector<buffered_block_t>;
	
	class block_chain_t : public block_chain<typename storage_t::block_t, S>
	{
		using _Base = block_chain<typename storage_t::block_t, S>;
	public:
		block_chain_t(blocks_buffer_t & _buffer, size_t first_buffer_index, chain_info_t & chain_info, const S & vs, int max_items_in_block)
			:_Base(chain_info, vs, max_items_in_block), m_buffer(_buffer), m_firstBufferIndex(first_buffer_index), m_initialBlockCount(chain_info.block_count)
		{
			for (int i = 0; i < chain_info.block_count; ++i)
			{
				m_blocksData[i].pBlock = &m_buffer[first_buffer_index + i].data;
				m_totalElements += m_blocksData[i].pBlock->item_count();
			}
		}

		~block_chain_t()
		{
			/*for (int i = 0; i < m_initialBlockCount; ++i)
			{
			if (m_blocksData[i].modified)
			m_buffer[m_firstBufferIndex + i].modified = true;
			}*/

			remove_last_blocks(m_blocksData.size());	
		}

		void remove_last_blocks(int count)
		{
			for (int i = 0; i < count; ++i)
			{
				int idx = m_blocksData.size() - i - 1;

				if (idx < m_initialBlockCount)
				{
					if (m_blocksData[idx].modified)
						m_buffer[m_firstBufferIndex + idx].modified = true;
				}
			}

			m_blocksData.resize(m_blocksData.size() - count);
		}
	private:
		blocks_buffer_t & m_buffer;
		size_t m_firstBufferIndex;
		int m_initialBlockCount;
	};

	struct request_cmp_t
	{
		bool operator()(const insertion_request_t & lhs, const insertion_request_t & rhs) const
		{
			return lhs.val == rhs.val;
		}
	};

	struct merge_task
	{
		merge_task(delayed_complex_hashset *executor, chain_info_t & info, size_t first_block_index, typename request_container_t::iterator begin, typename request_container_t::iterator end, std::atomic<int> & new_block_counter, tbb::concurrent_vector<std::pair<size_t, chain_info_t>> & new_indices)
		:m_exec(executor), m_chainInfo(info), m_firstBlockIndex(first_block_index), m_begin(begin), m_end(end), m_newBlockCounter(new_block_counter), m_newIndices(new_indices)
		{}

		void operator()()
		{
			m_exec->merge_into_chain(m_chainInfo, m_firstBlockIndex, m_begin, m_end, m_newBlockCounter, m_newIndices);
		}
	private:
		delayed_complex_hashset * m_exec;
		chain_info_t & m_chainInfo;
		typename request_container_t::iterator m_begin, m_end;
		std::atomic<int> & m_newBlockCounter;
		tbb::concurrent_vector<std::pair<size_t, chain_info_t>> & m_newIndices;
		const size_t m_firstBlockIndex;
	};

public:
	delayed_complex_hashset(const S & ss)
		:_Base(ss), m_threadPool(20), m_controllerThread(&delayed_complex_hashset::merge_loop, this)
	{
		m_blocks.append(block_t(0));
		m_index.create_mapping(0, chain_info_t(0, 0, 1));
	}

	delayed_complex_hashset(const delayed_complex_hashset & rhs)
		:_Base(rhs)
	{
		throw runtime_error("Copiing not available");
	}

	~delayed_complex_hashset()
	{
		m_controllerThread.join();
	}

	/*template<typename It, typename HashFun, typename ValFun, typename CallbackFun>
	void insert_range(It begin, It end, HashFun hash_fun, ValFun val_fun, CallbackFun fun)
	{
		for (auto it = begin; it != end; ++it)
		{
			this->insert(val_fun(*it), hash_fun(*it));
		}
	}*/

	void insert_delayed(const value_type & val)
	{
		insert_delayed(val, [](const value_type & _val){});
	}

	template<typename CallbackFun>
	void insert_delayed(const value_type & val, CallbackFun fun)
	{
		insert_delayed(val, m_hasher(val), fun);
	}

	template<typename CallbackFun>
	void insert_delayed(const value_type & val, size_t hash_val, CallbackFun fun)
	{
		m_inputBuffer.safe_access_first([=](request_container_t & cont){
			cont.push_back(insertion_request_t(hash_val, val, fun, m_serializedElementSize));
		});
/*		if (m_blockCount < 64)
		{
			if (insert(val, hash_val))
				fun(val);
		}
		else
		{
			size_t cur_point = m_index.interval_begin(hash_val);

			size_t gr_size = m_delayedBuffer.insert(cur_point, typename delayed_buffer_t::insertion_request_t(hash_val, val, fun, m_serializedElementSize));

			auto chain = m_index.chain_with_hash(cur_point);

			if (gr_size == 10)
				m_blocks.request_chain_in_future(chain);

			if (gr_size > m_maxItemsInBlock * 0.3)
			{

				if (m_blocks.chain_in_cache(chain))
				{
					std::lock_guard<std::mutex> lock(m_blocks.m_mtx);
					if ((chain.first == 10) && (chain.last == 10))
						int x = 0;
					if (m_blocks.chain_in_cache(chain))
					{
						//cout << "Luck! Flushing cached chain" << std::endl;
						flush_delayed_buffer_range(cur_point, m_delayedBuffer[cur_point]);
						m_delayedBuffer.erase(cur_point);
					}
					else
					{
						m_blocks.request_chain_in_future(chain);
						cout << "Cached element lost" << std::endl;
					}
				}
				else
					m_blocks.request_chain_in_future(chain);
			}
			else
			{
				//cout << "Force flushing...";
				if ((m_delayedBuffer.range_count() > 10000) || (m_delayedCounter++ > 100000))
					//if ((m_delayedBuffer.size() > 10000) || (m_delayedCounter++ > 1000000))
				{
					m_delayedCounter = 0;
					flush_delayed_buffer();
				}

				//cout << "...done" << std::endl;
			}

			/*auto group_it = m_delayedBuffer.find(cur_point);
			if (group_it == m_delayedBuffer.end())
			m_delayedBuffer.insert(delayed_buffer_groupt_t(delayed_insertion_request_t(hash_val, val, fun)));
			else
			{
			auto & group = group_it->second;


			}
		}*/
	}

	void flush_delayed_buffer()
	{
/*		while (m_successQueue.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		int x = 0;
		if (m_delayedBuffer.range_count() == 0)
			return;

		cout << "Flushing (" << m_delayedBuffer.range_count() << " groups)..." << std::endl;
		size_t els_count = 0, groups_count = 0;

		m_delayedBuffer.for_each_range([&](size_t key, typename delayed_buffer_t::groupt_t & range){
			++groups_count;
			els_count += range.size();

			std::lock_guard<std::mutex> lock(m_blocks.m_mtx);
			flush_delayed_buffer_range(key, range);
		});

		m_delayedBuffer.clear();
		*/
		//cout << 100.0f * (float)els_count / groups_count << "% gained" << std::endl;
	}

	template<typename RangeT>
	void flush_delayed_buffer_range(size_t cur_point, RangeT & range)
	{
		bool something_added = false;

		chain_info_t & chain_id = m_index.chain_with_hash(cur_point);
		m_blocks.ensure_chain_in_cache(chain_id);
		block_chain_t chain(*this, chain_id);

		if (range.size() < 10)
		{
			//cout << "Single insert" << std::endl;
			//insert(range[0].val, range[0].hash_val);
			insert_small_into_chain(chain, range.begin(), range.end(), [](const typename delayed_buffer_t::insertion_request_t & req){
				return req.hash_val;
			}, [=](const typename delayed_buffer_t::insertion_request_t & req){
				return req.to_byte_range();
			}, [&](const typename delayed_buffer_t::insertion_request_t & req){
				req.callback(req.val);
				++m_size;
				something_added = true;
			});

		}
		else
		{
			//Apply internal merge
			sort_wrapper(range.begin(), range.end(), [](const typename delayed_buffer_t::insertion_request_t & lhs, const typename delayed_buffer_t::insertion_request_t & rhs){
				return lhs.hash_val < rhs.hash_val;
			});

			typename delayed_buffer_t::request_cmp_t cmp;
			auto last_it = UltraCore::unique(range.begin(), range.end(), [](const typename delayed_buffer_t::insertion_request_t & req){
				return req.hash_val;
			}, cmp);

			//Serialize candidates
			for (auto it = range.begin(); it != last_it; ++it)
			{
				memcpy(&it->serialized_val[0], &it->hash_val, sizeof(size_t));
				m_valueStreamer.serialize(&it->serialized_val[sizeof(size_t)], it->val);
			}

			/*if (last_it != range.end())
			cout << "Internal merge removed " << std::distance(last_it, range.end()) << " elements." << std::endl;*/

			//cout << "Flushing range with length " << std::distance(range.first, range.second) << std::endl;

			insert_into_chain(chain, range.begin(), last_it, [](const typename delayed_buffer_t::insertion_request_t & req){
				return req.hash_val;
			}, [=](const typename delayed_buffer_t::insertion_request_t & req){
				return req.to_byte_range();
			}, [&](const typename delayed_buffer_t::insertion_request_t & req){
				req.callback(req.val);
				++m_size;
				something_added = true;
			});

			/*for_each(range.begin(), range.end(), [&](delayed_insertion_request_t & x){
			if (insert_into_chain(chain, x.hash_val, x.val))
			{
			x.callback(x.val);
			something_added = true;
			++m_size;
			}
			});*/
		}

		if (something_added)
			try_ballance_chain(chain);
	}

	iterator find(const value_type & val)
	{
		size_t hash_val = m_hasher(val);
		chain_info_t & info = m_index.chain_with_hash(hash_val);

		//Read sequentional all the chain
		size_t cur_block_id = info.first, last_block_id;
		size_t buffer_index = 0;
		do
		{
			last_block_id = cur_block_id;
			m_blocks.get(cur_block_id, m_blocksBuffer[buffer_index].data);
			cur_block_id = m_blocksBuffer[buffer_index].data.meta.next;
			++buffer_index;
		} while (last_block_id != cur_block_id);

		//Build chain
		block_chain_t chain(m_blocksBuffer, 0, info, m_valueStreamer, m_maxItemsInBlock);

		return find_in_chain(chain, val, hash_val);
	}

	void merge_loop()
	{
		while (true)
		{
			//cout << "Starting merging..." << std::endl;
			m_inputBuffer.swap_buffers();
			m_inputBuffer.safe_access_second([=](request_container_t & cont){
				if (cont.empty())
					return;
				cout << "Merging " << cont.size() << " requests..." << std::endl;
//
//				cout << "Sorting... ";
				sort_wrapper(cont.begin(), cont.end(), [](const insertion_request_t & lhs, const insertion_request_t & rhs){
					return lhs.hash_val < rhs.hash_val;
				});

				merge(cont.begin(), cont.end());

				cont.resize(0);
				cout << "Finished." << std::endl;
			});

			
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}

	template<typename It>
	void merge(It begin, It end)
	{
		//Get all descriptions
		vector<merge_range_descr_t> range_descrs;

		size_t total_old_blocks = 0;
		It it = begin;
		while( it != end)
		{
			size_t group_last_hash = 0;
			chain_info_t * p_chain = &m_index.chain_with_hash(it->hash_val, &group_last_hash);

			auto last_it = std::find_if_not(it, end, [=](const insertion_request_t & el){
				return el.hash_val < group_last_hash;
			});

			merge_range_descr_t new_desc;
			new_desc.p_info = p_chain;
			new_desc.first = std::distance(begin, it);
			new_desc.last = std::distance(begin, last_it);
			new_desc.first_buffer_block = total_old_blocks;

			range_descrs.push_back(std::move(new_desc));
			
			it = last_it; 

			total_old_blocks += p_chain->block_count;
		}

		//Allocate buffers
		resize_if_less(m_blocksBuffer, total_old_blocks);

		size_t total_merging_items = std::distance(begin, end);
		resize_if_less(m_newBlocksBuffer, integer_ceil(total_merging_items, m_maxItemsInBlock));

		//Read all chain beginnings and ends in one API
/*		vector<storage_t::read_request> first_read_req;
		for (auto rr : range_descrs)
		{
			storage_t::read_request first_req;
			first_req.block_id = rr.p_info->first;
			first_req.dest_ptr = &blocks_buffer[rr.first_buffer_block];
			first_read_req.push_back(first_req);

			if (rr.p_info->block_count > 1)
			{
				storage_t::read_request last_req;
				last_req.block_id = rr.p_info->last;
				last_req.dest_ptr = &blocks_buffer[rr.first_buffer_block + rr.p_info->block_count - 1];
				first_read_req.push_back(last_req);
			}
		}

		m_blocks.get(first_read_req);
		*/

		std::atomic<int> new_block_buf_index(0);

		tbb::concurrent_vector<std::pair<size_t, chain_info_t>> new_indices;

		//Process them
		for (auto & rd : range_descrs)
			m_threadPool.submit(new merge_task(this, *rd.p_info, rd.first_buffer_block, begin + rd.first, begin + rd.last, new_block_buf_index, new_indices));
		
		//merge_into_chain(*rd.p_info, rd.first_buffer_block, begin + rd.first, begin + rd.last, new_block_buf_index, new_indices);
		
		//Wait all to finish
		m_threadPool.wait_all();

		//Write new blocks
		if (new_block_buf_index.load() > 0)
			m_blocks.write_range(m_newBlocksBuffer.data(), m_newBlocksBuffer.data()->meta.id, new_block_buf_index.load());

		//Update indices
		for (auto & el : new_indices)
			m_index.create_mapping(el.first, el.second);
	}

	template<typename It>
	void merge_into_chain(chain_info_t & info, size_t first_block_index, It begin, It end, std::atomic<int> & new_block_counter, tbb::concurrent_vector<std::pair<size_t, chain_info_t>> & new_indices)
	{
//		cout << "Applying internal merge...";
		//Apply internal merge
		request_cmp_t cmp;
		auto last_it = UltraCore::unique(begin, end, [](const insertion_request_t & req){
			return req.hash_val;
		}, cmp);
//		cout << std::distance(begin, last_it) << " requests left" << std::endl;

		//Serialize candidates
		for (auto it = begin; it != last_it; ++it)
		{
			memcpy(&it->serialized_val[0], &it->hash_val, sizeof(size_t));
			m_valueStreamer.serialize(&it->serialized_val[sizeof(size_t)], it->val);
		}
	

//		cout << "Reading blocks from file..." << std::endl;
		//Read sequentional all the chain
		size_t cur_block_id = info.first, last_block_id;
		size_t buffer_index = first_block_index;
		do
		{
			last_block_id = cur_block_id;
			m_blocks.get(cur_block_id, m_blocksBuffer[buffer_index].data);
			cur_block_id = m_blocksBuffer[buffer_index].data.meta.next;
			++buffer_index;
		} while (last_block_id != cur_block_id);

		int initial_chain_length = info.block_count;

		{
			//Build chain
			block_chain_t chain(m_blocksBuffer, first_block_index, info, m_valueStreamer, m_maxItemsInBlock);
			const size_t old_block_count = m_blocks.size();

			//Merge into it
			insert_into_chain(chain, begin, last_it, [&](){
				int new_buf_index = new_block_counter++;
				block_t * new_block_ptr = &m_newBlocksBuffer[new_buf_index];
				size_t new_global_id = old_block_count + new_buf_index;
				new_block_ptr->set_meta(new_global_id, new_global_id, new_global_id, 0);
				return new_block_ptr;
			});

			//Ballance chain
			chain_info_t new_chain_info;
			size_t new_chain_first_hash;

			bool r;
			do
			{
				r = try_ballance_chain(chain, new_chain_info, &new_chain_first_hash);
				if (r)
					new_indices.push_back(make_pair(new_chain_first_hash, new_chain_info));
			} while (r);				
		}

		//Write modified blocks
		for (size_t i = 0; i < initial_chain_length; ++i)
		{
			if (m_blocksBuffer[first_block_index + i].modified)
				m_blocks.set(m_blocksBuffer[first_block_index + i].data.meta.id, m_blocksBuffer[first_block_index + i].data);
		}
	}

	template<typename It, typename NewBlockFun>
	void insert_into_chain(block_chain_t & chain, It begin_it, It end_it, NewBlockFun new_block_fun)
	{
		//Remove duplication amongst news
		size_t inserting_size = std::distance(begin_it, end_it);
		auto last_it = UltraCore::unique(chain.begin(), chain.end(), begin_it, end_it, [](const byte_range & br){
				return br.begin_as<size_t>();
			}, [=](const insertion_request_t & val){
				return val.hash_val;
			}, [=](const byte_range & old_val, const insertion_request_t & new_val){
				return old_val == new_val.to_byte_range();
			});

		//Add to chain what's left
		size_t old_size = chain.element_count();
		size_t merging_size = std::distance(begin_it, last_it);
		chain.resize(new_block_fun, chain.element_count() + merging_size);

		
		std::reverse(begin_it, last_it);
		
		std::merge(chain.rbegin() + merging_size, chain.rend(), begin_it, last_it, chain.rbegin(), [](const byte_range & lhs, const byte_range & rhs){
			return lhs.begin_as<size_t>() > rhs.begin_as<size_t>();
		});

		m_size += std::distance(begin_it, last_it);

		for (auto it = begin_it; it != last_it; ++it)
			m_successQueue.push(*it);
	}

	void get_merged()
	{
		bool r = false;
		
		insertion_request_t req;
		
		do
		{
			r = m_successQueue.try_pop(req);
			if (r)
				req.callback(req.val);
		} while (r);
	}
private:
	double_buffer<request_container_t> m_inputBuffer;
	tbb::concurrent_queue<insertion_request_t> m_successQueue;
	std::thread m_controllerThread;
	blocks_buffer_t m_blocksBuffer;
	std::vector<block_t> m_newBlocksBuffer;
	thread_pool<merge_task> m_threadPool;
};

#endif
