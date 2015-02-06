
#ifndef UltraCore_buffered_complex_hashset_h
#define UltraCore_buffered_complex_hashset_h

#include "complex_hashset_base.h"
#include <tbb/concurrent_queue.h>
#include <thread>

template<typename S>
class vector_storage_wrapper
{
	using storage_t = S;
public:
	using block_t = typename S::value_type;

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

	const block_t & operator[](size_t index) const
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

	void mark_used(size_t index)
	{
		m_storage.mark_used(index);
	}

	void clear_used(size_t index)
	{
		m_storage.clear_used(index);
	}

	void set_modified(size_t index)
	{
		m_storage.set_modified(index);
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

	bool element_cached(size_t index) const
	{
		return m_storage.is_element_cached(index);
	}

	bool chain_in_cache(const chain_info_t & chain_info) const
	{
		size_t cur_id = chain_info.first, last_id = cur_id;

		do
		{
			if (!m_storage.is_element_cached(cur_id))
				return false;

			last_id = cur_id;
			cur_id = m_storage[cur_id].next();
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
		/*if ((cur_chain.block_count <= 2) && m_storage.is_element_cached(cur_chain.last) && m_storage.is_element_cached(cur_chain.first))
			return;
		else
		{*/
			const storage_t & const_storage = m_storage;

			size_t cur_id = cur_chain.first, last_id = cur_id;
			do
			{
				last_id = cur_id;

				if (m_storage.is_element_cached(cur_id))
					cur_id = const_storage[cur_id].next();
				else
					cur_id = m_storage[cur_id].next();

			} while (cur_id != last_id);
		//}
	}

	int io_request_count() const
	{
		return m_cacheRequests.unsafe_size();
	}

	size_t cache_size() const
	{
		return storage_t::CacheSize;
	}

	const cached_file_stats_t & stats() const
	{
		return m_storage.stats();
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
		using result = cached_file<block_t, 256>; //131072U //262144U //65536U //1048576U
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

template<typename T, typename S, typename W, typename H = std::hash<T>>
class buffered_complex_hashset : public complex_hashset_base<T, S, W, H>
{
	using _Base = complex_hashset_base<T, S, W, H>;
	using storage_t = W;
	//using value_streamer_t = S;

	class block_chain_t : public block_chain<typename wrapper_t::block_t, S>
	{
		using _Base = block_chain<typename wrapper_t::block_t, S>;
	public:
		block_chain_t(storage_t & _blocks, chain_info_t & chain_info, const S & vs, int max_items_in_block)
			:_Base(chain_info, vs, max_items_in_block), m_blocksStorage(_blocks)
		{
			//cout << "Creating chain " << limits.first << "->" << limits.second << std::endl;

			size_t block_id = limits.first, last_block_id = block_id;

			int block_number = 0;
			do
			{
				block_t & block_ref = _blocks[block_id];
#if _DEBUG
				if (block_ref.meta.prev != last_block_id)
					throw runtime_error("Invalid chain");
				if (block_ref.meta.id != block_id)
					throw runtime_error("Invalid chain");
#endif

				last_block_id = block_id;

				_blocks.mark_used(block_id);
				m_blocksData[block_number].pBlock = &block_ref;

				m_totalElements += block_ref.item_count();
				block_id = block_ref.next();
				++block_number;
			} while (last_block_id != block_id);

#if _DEBUG
			if (limits.block_count != block_number)
				throw runtime_error("Invalid chain");
			for (auto & bd : m_blocksData)
			{
				if (bd.pBlock == nullptr)
					throw runtime_error("Invalid chain");
			}
#endif
		}

		~block_chain_t()
		{
			for (block_info_t & bd : m_blocksData)
			{
				if (bd.modified)
					m_blocksStorage.set_modified(bd.pBlock->meta.id);
				m_blocksStorage.clear_used(bd.pBlock->meta.id);
			}
		}
	private:
		storage_t & m_blocksStorage;
	};
	
public:
	buffered_complex_hashset(const S & ss)
		:_Base(ss)
	{
		//Create one empty block
		size_t first_block_id = request_block();
		m_index.create_mapping(0, chain_info_t(first_block_id, first_block_id, 1));
	}

	template<typename It, typename HashFun, typename ValFun, typename CallbackFun>
	void insert_range(It begin, It end, HashFun hash_fun, ValFun val_fun, CallbackFun fun)
	{
		for (auto it = begin; it != end; ++it)
		{
			this->insert(val_fun(*it), hash_fun(*it));
		}
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
		//cout << chain_id << std::endl;
		//m_blocks.ensure_chain_in_cache(chain_id);
		block_chain_t chain(m_blocks, chain_id, m_valueStreamer, m_maxItemsInBlock);

		m_valueStreamer.serialize(&m_elementCache[sizeof(size_t)], val);
		memcpy(&m_elementCache[0], &hash_val, sizeof(size_t));

		//chain.print();

		auto res = insert_into_chain(chain, hash_val, byte_range(&m_elementCache[0], m_serializedElementSize));
		//chain.print();

		if (res)
		{
			try_ballance_chain(chain);
			++m_size;
		}

		return res;
	}

	iterator find(const value_type & val)
	{
		size_t hash_val = m_hasher(val);

		block_chain_t chain(*this, m_index.chain_with_hash(hash_val));
		auto it = find_first_ge(chain.begin(), chain.end(), hash_val);


		if (it != chain.end())	//If we found element with similar hash, check for duplication
		{
			m_valueStreamer.serialize(&m_elementCache[sizeof(size_t)], val);
			memcpy(&m_elementCache[0], &hash_val, sizeof(size_t));
			byte_range searchin_element_br(&m_elementCache[0], m_serializedElementSize);

			for (; (it != chain.end()) && ((*it).template begin_as<size_t>() == hash_val); ++it)
			{
				if (searchin_element_br == *it)	//Duplication detected
				{
					auto addr = chain.element_address(it.elementId());
					return iterator(addr.first, addr.second);
				}
			}
		}

		return end();
	}
private:
	bool insert_into_chain(block_chain_t & chain, size_t hash_val, const byte_range & br)
	{
		auto it = find_first_ge(chain.cbegin(), chain.cend(), hash_val);


		if (it != chain.cend())	//If we found element with similar hash, check for duplication
		{
			for (; (it != chain.cend()) && ((*it).template begin_as<size_t>() == hash_val); ++it)
			{
				if (br == *it)	//Duplication detected
					return false;
			}
		}

		if (chain.element_count() == m_maxItemsInBlock * chain.block_count())
		{
			chain.append_new_block([=](){
				size_t block_id = this->request_block();
				block_t & block_ref = m_blocks[block_id];
				m_blocks.mark_used(block_id);
				return &block_ref;
			});
		}

		chain.insert(chain.begin() + std::distance(chain.cbegin(), it), br);

		return true;
	}

	void try_ballance_chain(block_chain_t & chain)
	{
		//Ballance
		if ((chain.limits.block_count > MaxBlocksPerChain) && (chain.density() > 0.96))
		{
			//cout << "B" << std::endl;

			int partitioned_block_number = chain.partitioned_block();
			if (partitioned_block_number != 0)
			{

				/*cout << "Ballancing chain: " << chain.limits << std::endl;
				for(auto bi : chain.m_blockIds)
				cout << bi << "," << std::endl;*/

				//Fast split the chain
				chain_info_t new_chain_id;
				new_chain_id.first = chain.m_blocksData[partitioned_block_number].pBlock->meta.id;
				new_chain_id.last = chain.limits.last;
				new_chain_id.block_count = chain.m_blocksData.size() - partitioned_block_number;
				m_blocks[new_chain_id.first].meta.prev = new_chain_id.first;
				m_blocks.set_modified(new_chain_id.first);

				chain.limits.last = chain.m_blocksData[partitioned_block_number - 1].pBlock->meta.id;
				chain.limits.block_count = partitioned_block_number;
				m_blocks[chain.limits.last].set_next(chain.limits.last);
				m_blocks.set_modified(chain.limits.last);

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
	}
private:
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
			block.meta.id = res;
			block.set_next(res);
			block.meta.prev = res;

			return res;
		}
	}

	void delete_block(size_t block_id)
	{
		cout << "Deleting block " << block_id << std::endl;
		block_t & block = m_blocks[block_id];
		block.item_count = 0;
		block.set_next(std::numeric_limits<size_t>::max());
		block.prev = std::numeric_limits<size_t>::max();
		block.id = std::numeric_limits<size_t>::max();

		m_freedBlocks.push(block_id);
	}

private:
	std::queue<size_t> m_freedBlocks;
};

#endif
