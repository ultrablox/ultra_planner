
#ifndef USERDB_CACHE_H_
#define USERDB_CACHE_H_

#include <unordered_map>
#include <mutex>
#include <atomic>
#include <iostream>
#include <cstring>
#include <vector>

using namespace std;

template<typename R>
struct cached_record
{
    using record_t = R;

    cached_record()
		:/*recently_used(false), */key(std::numeric_limits<size_t>::max()), use_count(0)
    {
    }

    //mutex mtx;
    //std::atomic<int> access_counter;
    //bool recently_used;
	int use_count;
    record_t data;
	size_t key;
};

template<typename R, int Size = 50>
class cache
{
    using record_t = R;
    using key_t = size_t;
    using cached_record_t = cached_record<record_t>;
    using ext_read_fun = std::function<bool(record_t &, key_t)>;
    using ext_write_fun = std::function<bool(const record_t &, key_t)>;

    friend class record_accessor;

public:

    template<typename FR, typename FW>
	cache(FR fr, FW fw, int max_size = Size)
		:m_cacheSize(max_size), m_extReadFun(fr), m_extWriteFun(fw), m_currentCacheIndex(0), m_cachedArray(max_size)
    {
    }

	~cache()
	{
		//Flush cache
		//for (auto & rec : m_cachedArray)
		//	m_extWriteFun(rec.data, rec.key);
	}

    //cached_record_t * get_record(const key_t & int_id)
	record_t & operator[](size_t index) const
    {
		//Try to find record inside cache
		auto it = m_data.find(index);

		//If found - return accessor
		if (it != m_data.end())
		{
			//m_cachedArray[it->second].recently_used = true;
			++m_cachedArray[it->second].use_count;
			return m_cachedArray[it->second].data;
		}
		else
		{
			auto res = read_into_cache(index);

			//Remove old mapping
			if (res.second != std::numeric_limits<size_t>::max())
			{
				m_extWriteFun(m_cachedArray[res.first].data, res.second);
				m_data.erase(res.second);
			}

			//Create new
			m_data.insert(make_pair(index, res.first));

			//Read data from file, if it exists
			m_extReadFun(m_cachedArray[res.first].data, index);
			++m_cachedArray[res.first].use_count;

			return m_cachedArray[res.first].data;
		}
    }

    void commit_record(const key_t & int_id, cached_record_t * record)
    {
        m_extWriteFun(record->data, int_id);
    }

    void clear()
    {
        m_data.clear();
    }

private:
    /*
    Inserts cached record with FINUFO and returns index of array.
    */
	std::pair<int, size_t> read_into_cache(size_t block_index) const
    {
        bool inserted = false;
        int res_index = -1;
		size_t old_index = std::numeric_limits<size_t>::max();

        while(!inserted)
        {
            cached_record_t & cur_rec = m_cachedArray[m_currentCacheIndex];
			if (cur_rec.key == std::numeric_limits<size_t>::max())
            {
				m_cachedArray[m_currentCacheIndex].key = block_index;
                res_index = m_currentCacheIndex;
                inserted = true;
            }
            else
            {
				/*if (cur_rec.recently_used)
					cur_rec.recently_used = false;
                else*/
				if ((--cur_rec.use_count) == 0)
                {
					old_index = m_cachedArray[m_currentCacheIndex].key;
					m_cachedArray[m_currentCacheIndex].key = block_index;
                    res_index = m_currentCacheIndex;
                    inserted = true;
                }
            }

            m_currentCacheIndex = (m_currentCacheIndex + 1) % m_cacheSize;
        }

		return make_pair(res_index, old_index);
    }
public:
    int m_cacheSize;
	mutable int m_currentCacheIndex;
    
    ext_read_fun m_extReadFun;
    ext_write_fun m_extWriteFun;

	mutable std::unordered_map<key_t, int> m_data;
    mutable std::vector<cached_record_t> m_cachedArray;
};

#endif
