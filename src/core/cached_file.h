
#ifndef USERDB_CACHED_FILE_H_
#define USERDB_CACHED_FILE_H_

#include "cache.h"
#include "data_file.h"

template<typename R, int CacheSize = 50>
class cached_file : public cache<R, CacheSize>
{
	using _Base = cache<R, CacheSize>;
    using record_t = R;
    using key_t = size_t;
	using cache_t = cache<record_t>;
public:
    cached_file(const std::string & file_name = "cached_file.dat")
		:_Base([&](record_t & data, size_t int_id){
				return m_file.get(int_id, data);
            },
				[&](const record_t & data, size_t int_id){
				return m_file.set(int_id, data);
            }), m_file(file_name)
    {

    }

    void clear_cache()
    {
        _Base::clear();
    }

    void clear()
    {
        clear_cache();
        m_file.clear();
    }

	void push_back(const record_t & record)
	{
		m_file.append(record);
	}

	size_t size() const
	{
		return m_file.size();
	}

	void open(const std::string file_name)
	{
		//m_file.open(file_name);
	}

	void write_range(const std::vector<record_t> & write_queue)
	{

	}
private:
    data_file<record_t> m_file;
};

#endif
