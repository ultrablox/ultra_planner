
#ifndef USERDB_CACHED_FILE_H_
#define USERDB_CACHED_FILE_H_

#include "../config.h"
#include "cache.h"
#include "data_file.h"

struct cached_file_stats_t
{
	cached_file_stats_t()
	:io_reads(0), io_writes(0)
	{}

	friend std::ostream & operator<<(std::ostream & os, const cached_file_stats_t & stats)
	{
		os << "I/O operations (reads/writes): " << stats.io_reads << '/' << stats.io_writes << std::endl;
		return os;
	}

	size_t io_reads, io_writes;
};


template<typename R, unsigned int Cache_Size = 50>
class cached_file : public cache<R, Cache_Size>
{
	
	using _Base = cache<R, Cache_Size>;
    using record_t = R;
    using key_t = size_t;
	using cache_t = cache<record_t>;
public:
	using value_type = R;

    cached_file(const std::string & file_name = "cached_file.dat")
		:_Base([&](record_t & data, size_t int_id){
				++m_stats.io_reads;
				return m_file.get(int_id, data);
            },
			[&](const record_t & data, size_t int_id){
				++m_stats.io_writes;
				return m_file.set(int_id, data);
			}), m_file(file_name), m_count(0)
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
        /*size_t new_id = size();
		m_file.append(record);
		++m_stats.io_writes;		
        _Base::load(new_id, record);*/

		_Base::load(m_count++, record);
	}

	size_t size() const
	{
		return m_count;
	}

	void open(const std::string file_name)
	{
		//m_file.open(file_name);
	}

	void write_range(const std::vector<record_t> & write_queue)
	{

	}

	const cached_file_stats_t & stats() const
	{
		return m_stats;
	}

	bool empty() const
	{
		return (m_count == 0);
	}

	const data_file<record_t> & file() const
	{
		return m_file;
	}
private:
    data_file<record_t> m_file;
	cached_file_stats_t m_stats;
	size_t m_count;
};

#endif
