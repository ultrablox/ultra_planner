
#ifndef UltraSolver_search_database_h
#define UltraSolver_search_database_h

#include <core/complex_vector.h>
#include <core/direct_complex_hashset.h>
#include <core/buffered_complex_hashset.h>
#include <utility>
#include <unordered_set>
#include <type_traits>
#include <atomic>

template<typename T, typename H = std::hash<T>, typename... Args>
class simple_search_database
{
	typedef T state_t;
public:
	template<typename... ConstrArgs>
	simple_search_database(ConstrArgs... args)
	{}

	bool contains(const state_t & state) const
	{
		return (m_discoveredStates.find(state) != m_discoveredStates.end());
	}

	bool add(const state_t & state)
	{
		auto res = m_discoveredStates.insert(state);
		return res.second;
	}

private:
	std::unordered_set<state_t, H> m_discoveredStates;
};


template<typename T, typename S, typename H = std::hash<T>, bool ExtMemory = false, int StorageCount = 1>
class search_database
{
	typedef T state_t;
	using state_streamer_t = S;
	typedef H hash_t;
	typedef std::tuple<size_t, state_t> record_t;
	//typedef std::unordered_set<state_t> hash_map_t;
	//typedef direct_complex_hashset<state_t, state_streamer_t, hash_t, !ExtMemory> hash_map_t;
	typedef buffered_complex_hashset<state_t, state_streamer_t, hash_t, !ExtMemory> hash_map_t;
	const int storage_count = StorageCount;

	

public:
	//Id + parent search node id + state + init->current path length
	typedef std::tuple<size_t, size_t, state_t, int> search_node_t;
	
	class node_streamer_t
	{
	public:
		node_streamer_t(const state_streamer_t & base_streamer)
			:m_baseStreamer(base_streamer), m_serializedSize(base_streamer.serialized_size() + sizeof(size_t)* 2 + sizeof(int))
		{
		}

		void serialize(void * dst, const search_node_t & node) const
		{
			char * cur_ptr = (char*)dst;
			memcpy(cur_ptr, &get<0>(node), sizeof(size_t));
			memcpy(cur_ptr + sizeof(size_t), &get<1>(node), sizeof(size_t));
			memcpy(cur_ptr + 2 * sizeof(size_t), &get<3>(node), sizeof(int));
			m_baseStreamer.serialize(cur_ptr + 2 * sizeof(size_t)+sizeof(int), get<2>(node));
		}

		void deserialize(const void * src, search_node_t & node) const
		{
			const char * cur_ptr = (const char*)src;
			memcpy(&get<0>(node), cur_ptr, sizeof(size_t));
			memcpy(&get<1>(node), cur_ptr + sizeof(size_t), sizeof(size_t));
			memcpy(&get<3>(node), cur_ptr + 2 * sizeof(size_t), sizeof(int));
			m_baseStreamer.deserialize(cur_ptr + 2 * sizeof(size_t)+sizeof(int), get<2>(node));
		}

		size_t serialized_size() const
		{
			return m_serializedSize;
		}
	private:
		const state_streamer_t m_baseStreamer;
		size_t m_serializedSize;
	};


	//template<typename SerFun, typename DesFun>
	search_database(const state_streamer_t & ss/*, int serialized_state_size, SerFun s_fun, DesFun d_fun*/)
		:	//m_storages(storage_count, storage_factory<hash_map_t>().create_storage(serialized_state_size, s_fun, d_fun)),
			/*m_nodeSerializeFun([=](void * dst, const search_node_t & node){
				char * cur_ptr = (char*)dst;
				memcpy(cur_ptr, &get<0>(node), sizeof(size_t));
				memcpy(cur_ptr + sizeof(size_t), &get<1>(node), sizeof(size_t));
				memcpy(cur_ptr + 2 * sizeof(size_t), &get<3>(node), sizeof(int));
				s_fun(cur_ptr + 2 * sizeof(size_t)+sizeof(int), get<2>(node));
			}), m_nodeDeserializeFun([=](const void * src, search_node_t & node){
				const char * cur_ptr = (const char*)src;
				memcpy(&get<0>(node), cur_ptr, sizeof(size_t));
				memcpy(&get<1>(node), cur_ptr + sizeof(size_t), sizeof(size_t));
				memcpy(&get<3>(node), cur_ptr + 2 * sizeof(size_t), sizeof(int));
				d_fun(cur_ptr + 2 * sizeof(size_t)+sizeof(int), get<2>(node));
			}),*/
				m_searchNodes(node_streamer_t(ss)/*, serialized_state_size + sizeof(size_t)* 2 + sizeof(int), m_nodeSerializeFun, m_nodeDeserializeFun*/), m_nodeCount(0), m_stateStreamer(ss)
	{
		for (int i = 0; i < storage_count; ++i)
			m_storages.emplace_back(m_stateStreamer/*, serialized_state_size, s_fun, d_fun*/);
	}

	bool contains(const state_t & state) const
	{
		size_t hash_val = m_hasher(state);
		auto & storage = m_storages[hash_val % storage_count];
		return (storage.find(state) != storage.end());
	}

	bool add(const state_t & state)
	{
		size_t hash_val = m_hasher(state);
		auto & storage = m_storages[hash_val % storage_count];
		auto res = storage.insert(state, hash_val);
		
		if(res.second)
			++m_nodeCount;

		return res.second;
	}

	template<typename CallbackFun>
	void add(const state_t & state, CallbackFun call_fun)
	{
		auto res = add(state);
		if (res)
			call_fun(state);

		/*size_t hash_val = m_hasher(state);
		std::vector<state_t> tmp_vec(1, state);
		add_range(tmp_vec.begin(), tmp_vec.end(), [=](const state_t &){
			return hash_val;
		}, [=](const state_t & state){
			return state;
		}, call_fun);*/
	}

	/*
	Adds unsorted sequence of new states to database. Callback is called
	for each successfully added state.
	*/
	template<typename It, typename HashFun, typename ValFun, typename CallbackFun>
	void add_range(It begin, It end, HashFun hash_fun, ValFun val_fun, CallbackFun call_fun)
	{
		size_t total_count = std::distance(begin, end);
		//cout << "Adding range of " << total_count << " elements do database." << std::endl;
		size_t storage_count = m_storages.size();

		//Sort elements by storage + by hash inside each group
		std::sort(begin, end, [=](const typename It::value_type & lhs, const typename It::value_type & rhs){
			if ((hash_fun(lhs) % storage_count) == (hash_fun(rhs) % storage_count))
				return hash_fun(lhs) < hash_fun(rhs);
			else
				return (hash_fun(lhs) % storage_count) < (hash_fun(rhs) % storage_count);
		});


		//Determine group ranges
		std::vector<int> group_start(storage_count + 1, total_count);

		int cur_id = hash_fun(*begin) % storage_count;
		for (int i = 0; i <= cur_id; ++i)
			group_start[i] = 0;

		int last_id = cur_id;

		for (auto it = begin; it != end; ++it)
		{
			cur_id = hash_fun(*it) % storage_count;

			if (cur_id != last_id)
			{
				int val = std::distance(begin, it);
				for (int i = last_id; i <= cur_id; ++i)
					group_start[i] = val;
				last_id = cur_id;
			}
		}

		//Call each storage with its group
		for (int i = 0; i < storage_count; ++i)
		{
			auto gr_begin = begin + group_start[i],
				gr_end = begin + group_start[i+1];

			if (gr_begin != gr_end)
				m_storages[i].insert_range(gr_begin, gr_end, hash_fun, val_fun, call_fun);
		}
	}

	template<typename KeyPart>
	void compress_keys_less(KeyPart key_part)
	{
		cout << "Compressing database (" << m_storages.size() << ") storages..." << std::endl;
	}

	search_node_t create_node(const state_t & state, size_t parent_id)
	{
		int path_len = m_searchNodes.empty() ? 0 : get<3>(m_searchNodes[parent_id]) + 1;
		return create_node(state, parent_id, path_len);
	}

	//Faster, if we know path length in advance
	search_node_t create_node(const state_t & state, size_t parent_id, int path_len)
	{
		search_node_t new_node(m_searchNodes.size(), parent_id, state, path_len);
		m_searchNodes.push_back(new_node);
		return std::move(new_node);
	}

	search_node_t parent_node(const search_node_t & node) const
	{
		return m_searchNodes[get<1>(node)];
	}

	size_t state_count() const
	{
		size_t res(0);
		for (auto & storage : m_storages)
			res += storage.size();
		return res;
	}

	size_t block_count() const
	{
		size_t res(0);
		for (auto & storage : m_storages)
			res += storage.block_count();
		return res;
	}

	float density() const
	{
		float res(0.0f);
		for (auto & storage : m_storages)
			res += storage.bucket_factor();
		return res / m_storages.size();
	}
public:
	//std::function<void(void*, const search_node_t&)> m_nodeSerializeFun;
	//std::function<void(const void*, search_node_t&)> m_nodeDeserializeFun;
private:
	state_streamer_t m_stateStreamer;
	//complex_vector<record_t> m_data;
	hash_t m_hasher;

	std::vector<hash_map_t> m_storages;
	//std::array<hash_map_t, StorageCount> m_storages;

	complex_vector<search_node_t, node_streamer_t, ExtMemory, true> m_searchNodes;

	std::atomic<size_t> m_nodeCount;
	
};

#endif
