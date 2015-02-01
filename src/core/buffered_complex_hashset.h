
#ifndef UltraCore_buffered_complex_hashset_h
#define UltraCore_buffered_complex_hashset_h

#include "complex_hashset_base.h"
#include <tbb/concurrent_queue.h>
#include <thread>

template<typename S>
class vector_storage_wrapper
{
	using storage_t = S;
	using block_t = typename S::value_type;

public:
	vector_storage_wrapper()
		:m_cacheLoader(&vector_storage_wrapper::cache_requests_loader, this)
	{
		m_finished = false;
	}

	vector_storage_wrapper(const vector_storage_wrapper & rhs)
	{
		throw runtime_error("Unable to copy");
	}

	~vector_storage_wrapper()
	{
		m_finished = true;
		m_cacheLoader.join();
	}

	size_t size() const
	{
		return m_storage.size();
	}

	void push_back(const block_t & block)
	{
		m_storage.push_back(block);
	}

	block_t const & operator[](size_t index) const
	{
		return m_storage[index];
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

	bool chain_in_cache(const chain_info_t & chain_info) const
	{
		size_t cur_id = chain_info.first, last_id = cur_id;

		do
		{
			if (!m_storage.is_element_cached(cur_id))
				return false;

			last_id = cur_id;
			cur_id = m_storage[cur_id].next;
		} while (cur_id != last_id);
		
		return true;
	}

	void request_chain_in_future(const chain_info_t & chain_info)
	{
		m_cacheRequests.push(chain_info);
	}

	void cache_requests_loader()
	{
		std::chrono::milliseconds dura(100);

		while (!m_finished)
		{
			if (m_cacheRequests.empty())
			{
				//std::this_thread::yield();
				std::this_thread::sleep_for(dura);
			}
			else
			{
				chain_info_t cur_chain;
				if (m_cacheRequests.try_pop(cur_chain))
				{
					std::lock_guard<std::mutex> lock(m_mtx);
					ensure_chain_in_cache(cur_chain);
				}
			}
		}
	}

	void ensure_chain_in_cache(const chain_info_t & cur_chain)
	{
		if ((cur_chain.block_count <= 2) && m_storage.is_element_cached(cur_chain.last) && m_storage.is_element_cached(cur_chain.first))
			return;
		else
		{
			size_t cur_id = cur_chain.first, last_id = cur_id;
			do
			{
				last_id = cur_id;
				cur_id = m_storage[cur_id].next;

			} while (cur_id != last_id);
		}
	}

	int io_request_count() const
	{
		return m_cacheRequests.unsafe_size();
	}

	size_t cache_size() const
	{
		return storage_t::CacheSize;
	}

	std::mutex m_mtx;

private:
	storage_t m_storage;
	tbb::concurrent_queue<chain_info_t> m_cacheRequests;
	std::thread m_cacheLoader;
	std::atomic<bool> m_finished;
	
};

namespace buffered_hashset
{
	template<typename B>
	struct ext_storage_generator
	{
		using block_t = B;
		//using result = typename stxxl::VECTOR_GENERATOR<block_t, 1U, 8192U, sizeof(block_t), stxxl::RC, stxxl::lru>::result; //131072U //262144U //65536U //1048576U
		using result = cached_file<block_t, 262144U>; //131072U //262144U //65536U //1048576U
	};

	template<typename B>
	struct int_storage_generator
	{
		using block_t = B;
		using result = std::vector<block_t>;
	};
};

/*
Internal memory - standard std::vector.
External - buffered stxxl vector.
*/

//std::conditional<UseIntMemory, buffered_hashset::int_storage_generator, buffered_hashset::ext_storage_generator>::type

template<typename T, typename S, typename H = std::hash<T>, bool UseIntMemory = true, unsigned int BlockSize = 4096U>//65536U //4096U // 8192U
class buffered_complex_hashset : public complex_hashset_base<T, S, H, vector_storage_wrapper, buffered_hashset::ext_storage_generator, BlockSize>
{
	using _Base = complex_hashset_base<T, S, H, vector_storage_wrapper, buffered_hashset::ext_storage_generator, BlockSize>;
//	
	
public:
	buffered_complex_hashset(const S & ss)
		:_Base(ss)
	{
	}

	template<typename It, typename HashFun, typename ValFun, typename CallbackFun>
	void insert_range(It begin, It end, HashFun hash_fun, ValFun val_fun, CallbackFun fun)
	{
		for (auto it = begin; it != end; ++it)
		{
			this->insert(val_fun(*it), hash_fun(*it));
		}
	}
};

#endif
