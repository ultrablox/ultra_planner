
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
		:empty(true), recently_used(1) /*key(std::numeric_limits<size_t>::max()), use_count(0)*/
    {
    }

    //mutex mtx;
    //std::atomic<int> access_counter;
    
	//int use_count;
	record_t data;
	size_t key;
	mutable int recently_used;
	bool empty;
	mutable bool modified;
    
};

template<typename R, unsigned int _CacheSize = 100>
class cache
{
    using value_t = R;
    using key_t = size_t;
	using cached_record_t = cached_record<value_t>;
	using ext_read_fun = std::function<bool(value_t &, key_t)>;
	using ext_write_fun = std::function<bool(const value_t &, key_t)>;

    friend class record_accessor;

public:

	static const int CacheSize = _CacheSize;

    template<typename FR, typename FW>
	cache(FR fr, FW fw)
		:/*m_cacheSize(max_size),*/ m_extReadFun(fr), m_extWriteFun(fw), m_currentCacheIndex(0), m_cachedArray(CacheSize)
    {
    }

	~cache()
	{
		//Flush cache
		//for (auto & rec : m_cachedArray)
		//	m_extWriteFun(rec.data, rec.key);
	}

    //cached_record_t * get_record(const key_t & int_id)
	const value_t & operator[](size_t index) const
	{
		auto it = m_index.find(index);
		if (it == m_index.end())
			throw runtime_error("Not cached record const-requested");
		//++m_cachedArray[it->second].recently_used;
		return m_cachedArray[it->second].data;
	}

	value_t & operator[](size_t index)
    {
		//Try to find record inside cache
		auto it = m_index.find(index);

		//If found - return accessor
		if (it != m_index.end())
		{
			m_cachedArray[it->second].modified = true;
			//++m_cachedArray[it->second].recently_used;
			return m_cachedArray[it->second].data;
		}
		else
		{
			int cache_index = read_into_cache(index);
			m_index.insert(make_pair(index, cache_index));

			//Read data from file, if it exists
			int r = m_extReadFun(m_cachedArray[cache_index].data, index);
            if(!r)
            {
                m_cachedArray[cache_index].data = value_t();
                m_extWriteFun(m_cachedArray[cache_index].data, index);
            }
			return m_cachedArray[cache_index].data;
		}
    }

    void clear()
    {
        m_index.clear();
    }

	bool is_element_cached(size_t element_id) const
	{
		return (m_index.find(element_id) != m_index.end());
	}
private:
    /*
    Inserts cached record with FINUFO and returns index of array.
    */
	int read_into_cache(size_t block_index)
    {
        bool inserted = false;
        int res_index = -1;
		size_t old_index = std::numeric_limits<size_t>::max();

        while(!inserted)
        {
            cached_record_t & cur_rec = m_cachedArray[m_currentCacheIndex];
			if (cur_rec.empty)
            {
				cur_rec.empty = false;
				cur_rec.recently_used = 1;
				cur_rec.key = block_index;
				cur_rec.modified = false;
                res_index = m_currentCacheIndex;
                inserted = true;
            }
            else
            {
				if (cur_rec.recently_used > 0)
					--cur_rec.recently_used;
				else
				{
					kick_element(m_currentCacheIndex);
					cur_rec.empty = false;
					cur_rec.recently_used = 1;
					cur_rec.key = block_index;
					cur_rec.modified = false;
					res_index = m_currentCacheIndex;
					inserted = true;
				}
            }

			m_currentCacheIndex = (m_currentCacheIndex + 1) % CacheSize;

			if (m_currentCacheIndex == 0)
				cout << "Cache loop" << std::endl;
        }

		m_extReadFun(m_cachedArray[res_index].data, block_index);

		return res_index;
    }

	void kick_element(int cache_index)
	{
		if (m_cachedArray[cache_index].modified)
			m_extWriteFun(m_cachedArray[cache_index].data, m_cachedArray[cache_index].key);

		m_index.erase(m_cachedArray[cache_index].key);
		m_cachedArray[cache_index].empty = false;
	}
public:
	unsigned int m_currentCacheIndex;
    
    ext_read_fun m_extReadFun;
    ext_write_fun m_extWriteFun;

	std::unordered_map<key_t, int> m_index;
	std::vector<cached_record_t> m_cachedArray;
};

#endif
