
#ifndef UltraCore_buffered_complex_hashset_h
#define UltraCore_buffered_complex_hashset_h

#include "complex_hashset_base.h"
#include <tbb/concurrent_queue.h>
#include <thread>

template<typename S>
class cached_file_wrapper
{
public:
	using storage_t = S;
	using block_t = typename S::value_type;
	static_assert(sizeof(block_t) % 4096 == 0, "Invalid block_t size for file!");

	cached_file_wrapper(const std::string & file_name = "cached_file.dat")
		:m_storage(file_name)
	{}

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

	void reserve(size_t new_size) const
	{}

	std::mutex m_mtx;

private:
	storage_t m_storage;
	tbb::concurrent_queue<chain_info_t> m_cacheRequests;
	
};

template<typename S>
class vector_wrapper
{
public:
	using storage_t = S;
	using block_t = typename S::value_type;
	
	vector_wrapper()
	{
		m_storage.reserve(1000);
	}

	size_t size() const
	{
		return m_storage.size();
	}

	size_t cache_size() const
	{
		return 0;
	}

	const block_t & operator[](size_t index) const
	{
		return m_storage[index];
	}

	block_t & operator[](size_t index)
	{
		return m_storage[index];
	}

	void mark_used(size_t index) const
	{}

	void clear_used(size_t index) const
	{}

	void set_modified(size_t index) const
	{}

	void push_back(const block_t & block)
	{
		m_storage.push_back(block);
	}

	void reserve(size_t new_size)
	{
		m_storage.reserve(new_size);
	}
private:
	storage_t m_storage;
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
	using value_type = typename _Base::value_type;
	using iterator = typename _Base::iterator;
	using block_t = typename _Base::block_t;
	//using value_streamer_t = S;

	class block_chain_t : public block_chain<typename W::block_t, S>
	{
		using _Base = block_chain<typename W::block_t, S>;
		using block_t = typename _Base::block_t;
		using block_info_t = typename _Base::block_info_t;
	public:
		block_chain_t(storage_t & _blocks, chain_info_t & chain_info, const S & vs, int max_items_in_block)
			:_Base(chain_info, vs, max_items_in_block), m_blocksStorage(_blocks)
		{
			//cout << "Creating chain " << limits.first << "->" << limits.second << std::endl;

			size_t block_id = _Base::limits.first, last_block_id = block_id;

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
				_Base::m_blocksData[block_number].pBlock = &block_ref;

				_Base::m_totalElements += block_ref.item_count();
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
			remove_last_blocks(_Base::m_blocksData.size());
			/*for (block_info_t & bd : m_blocksData)
			{
				if (bd.modified)
					m_blocksStorage.set_modified(bd.pBlock->meta.id);
				m_blocksStorage.clear_used(bd.pBlock->meta.id);
			}*/
		}

		void remove_last_blocks(int count)
		{
			for (int i = 0; i < count; ++i)
			{
				int idx = _Base::m_blocksData.size() - i - 1;
				block_info_t & bd = _Base::m_blocksData[idx];

				if (bd.modified)
					m_blocksStorage.set_modified(bd.pBlock->meta.id);
				m_blocksStorage.clear_used(bd.pBlock->meta.id);
			}

			_Base::m_blocksData.resize(_Base::m_blocksData.size() - count);
		}
	private:
		storage_t & m_blocksStorage;
	};
	
public:
	buffered_complex_hashset(const S & ss, const std::string & name = "default_hashset")
		:_Base(ss, name)
	{
		//Create one empty block
		size_t first_block_id = request_block();
		_Base::m_index.create_mapping(0, chain_info_t(first_block_id, first_block_id, 1));
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
		size_t hash_val = _Base::m_hasher(val);

		return insert(val, hash_val);
	}

	bool insert(const value_type & val, size_t hash_val)
	{
		//cout << "Inserting" << std::endl;
		//Find appropriate block, create if it does not exists

		chain_info_t & chain_id = _Base::m_index.chain_with_hash(hash_val);
		//cout << chain_id << std::endl;
		//m_blocks.ensure_chain_in_cache(chain_id);
		_Base::m_blocks.reserve(_Base::m_blocks.size() + 1);
		block_chain_t chain(_Base::m_blocks, chain_id, _Base::m_valueStreamer, _Base::m_maxItemsInBlock);

		_Base::m_valueStreamer.serialize(&_Base::m_elementCache[sizeof(size_t)], val);
		memcpy(&_Base::m_elementCache[0], &hash_val, sizeof(size_t));

		//chain.print();

		auto res = insert_into_chain(chain, hash_val, byte_range(&_Base::m_elementCache[0], _Base::m_serializedElementSize));
		//chain.print();

		if (res)
		{
			chain_info_t new_chain_info;
			if (_Base::try_ballance_chain(chain, new_chain_info))
				_Base::m_index.create_mapping(_Base::m_blocks[new_chain_info.first].first_hash(), new_chain_info);
			++_Base::m_size;
		}

		return res;
	}

	iterator find(const value_type & val)
	{
		size_t hash_val = _Base::m_hasher(val);

		block_chain_t chain(_Base::m_blocks, _Base::m_index.chain_with_hash(hash_val), _Base::m_valueStreamer, _Base::m_maxItemsInBlock);

		return _Base::find_in_chain(chain, val, hash_val);
	}
private:
	bool insert_into_chain(block_chain_t & chain, size_t hash_val, const byte_range & br)
	{
		//auto it = find_first_ge(chain.cbegin(), chain.cend(), hash_val);
		auto it = find_first_ge(chain.cbegin(), chain.cend(), br);


		if (it != chain.cend())	//If we found element with similar hash, check for duplication
		{
			if (br == *it)
				return false;
			/*for (; (it != chain.cend()) && ((*it).template begin_as<size_t>() == hash_val); ++it)
			{
				if (br == *it)	//Duplication detected
					return false;
			}*/
		}

		if (chain.element_count() == _Base::m_maxItemsInBlock * chain.block_count())
		{
			chain.append_new_block([=](){
				size_t block_id = this->request_block();
				this->m_blocks.mark_used(block_id);
				return &(this->m_blocks[block_id]);
			});
		}

		chain.insert(chain.begin() + std::distance(chain.cbegin(), it), br);

		return true;
	}
private:
	size_t request_block()
	{
		if (m_freedBlocks.empty())
		{
			size_t new_id = _Base::m_blockCount++;
			_Base::m_blocks.push_back(block_t(new_id));
			return new_id;
		}
		else
		{
			size_t res = m_freedBlocks.front();
			m_freedBlocks.pop();

			block_t & block = _Base::m_blocks[res];
			block.meta.id = res;
			block.set_next(res);
			block.meta.prev = res;

			return res;
		}
	}

	void delete_block(size_t block_id)
	{
		cout << "Deleting block " << block_id << std::endl;
		block_t & block = _Base::m_blocks[block_id];
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
