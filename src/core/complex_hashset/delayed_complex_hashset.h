
#ifndef UltraCore_delayed_complex_hashset_h
#define UltraCore_delayed_complex_hashset_h

#include "complex_hashset_base.h"
#include "../delayed_buffer.h"
#include <tbb/concurrent_queue.h>
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
	};
//	
	
	class block_chain_t : public block_chain<typename storage_t::block_t, S>
	{
		using _Base = block_chain<typename storage_t::block_t, S>;
	public:
		block_chain_t(chain_info_t & chain_info, const S & vs, int max_items_in_block)
			:_Base(chain_info, vs, max_items_in_block)//, m_blocksStorage(_blocks)
		{
			
		}

		~block_chain_t()
		{

		}
	private:
	};

	struct request_cmp_t
	{
		bool operator()(const insertion_request_t & lhs, const insertion_request_t & rhs) const
		{
			return lhs.val == rhs.val;
		}
	};
public:
	delayed_complex_hashset(const S & ss)
		:_Base(ss)
	{
		m_pControllerThread = new std::thread(&delayed_complex_hashset::merge_loop, this);

		m_blocks.append(block_t(0));
		m_index.create_mapping(0, chain_info_t(0, 0, 1));
	}

	~delayed_complex_hashset()
	{
		m_pControllerThread->join();
		delete m_pControllerThread;
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
		while (m_successQueue.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		int x = 0;
/*		if (m_delayedBuffer.range_count() == 0)
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

	void merge_loop()
	{
		while (true)
		{
			cout << "Starting merging..." << std::endl;
			m_inputBuffer.swap_buffers();
			m_inputBuffer.safe_access_second([=](request_container_t & cont){
				if (cont.empty())
					return;
				cout << "Merging " << cont.size() << " requests..." << std::endl;

				cout << "Applying internal merge... ";
				//Apply internal merge
				sort_wrapper(cont.begin(), cont.end(), [](const insertion_request_t & lhs, const insertion_request_t & rhs){
					return lhs.hash_val < rhs.hash_val;
				});

				request_cmp_t cmp;
				auto last_it = UltraCore::unique(cont.begin(), cont.end(), [](const insertion_request_t & req){
					return req.hash_val;
				}, cmp);

				//Serialize candidates
				for (auto it = cont.begin(); it != last_it; ++it)
				{
					memcpy(&it->serialized_val[0], &it->hash_val, sizeof(size_t));
					m_valueStreamer.serialize(&it->serialized_val[sizeof(size_t)], it->val);
				}

				merge(cont.begin(), last_it);
				cout << std::distance(cont.begin(), last_it) << " requests left" << std::endl;
			});

			cout << "Finished." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
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
			chain_info_t * p_chain = &m_index.chain_with_hash(it->hash_val);

			auto last_it = std::find_if_not(it, end, [=](const insertion_request_t & el){
				return p_chain == &this->m_index.chain_with_hash(el.hash_val);
			});

			merge_range_descr_t new_desc;
			new_desc.p_info = p_chain;
			new_desc.first = std::distance(begin, it);
			new_desc.last = std::distance(begin, last_it);

			range_descrs.push_back(std::move(new_desc));
			
			it = last_it; 

			total_old_blocks += p_chain->block_count;
		}

		//Process them
		std::vector<block_t> blocks_buffer(total_old_blocks);
		for (auto & rd : range_descrs)
			merge_into_chain(*rd.p_info, begin + rd.first, begin + rd.last);

		//Commit writes
	}

	template<typename It>
	void merge_into_chain(chain_info_t & info, It begin, It end)
	{
		//Reserve space in read buffer

		//Read sequentional

		//Build chain

		//Merge into it
	}
private:
	double_buffer<request_container_t> m_inputBuffer;
	tbb::concurrent_queue<insertion_request_t> m_successQueue;
	std::thread * m_pControllerThread;
};

#endif
