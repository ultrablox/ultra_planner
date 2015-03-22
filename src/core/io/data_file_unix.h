
#ifndef USERDB_DATA_FILE_H_
#define USERDB_DATA_FILE_H_

#include "../utils/helpers.h"
#include <mutex>
#include <iostream>
#include <list>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <aio.h>


using namespace std;


#define ITERATION_BLOCK_COUNT 16

/*
Linear container of fixed-length data elements.
*/
template<typename T>
class data_file
{


	//using data_vec_t = std::vector<block_type>;

	//    static_assert((sizeof(block_type) % 4096) == 0, "ERROR: user_record_data size is not 4K/divisible");
public:
	using block_type = T;
	using value_type = block_type;
	using block_t = block_type;

	struct read_request
	{
		size_t block_id;
		block_t * dest_ptr;
	};

    data_file(const std::string & file_name = "unnamed")
		:m_fileName(file_name), m_blockCount(0)
    {
		//cout << "Creating data file: " << m_fileName << std::endl;
        m_fileDescriptor = ::open(file_name.c_str(), O_RDWR | O_CREAT, 0644);
        if(m_fileDescriptor == -1)
            throw runtime_error("Unable to open file with unix API");
    }


	data_file(const data_file & rhs)
	{
		throw runtime_error("Can not be copied");
	}

    ~data_file()
    {
        close(m_fileDescriptor);
    }

	int set(size_t index, const block_type & data)
	{
        int n_write = pwrite(m_fileDescriptor, &data, sizeof(block_type), sizeof(block_type)*index);
        if(n_write == -1)
            cout << "Write failed: " << strerror(errno) << std::endl;

        m_blockCount = max(m_blockCount, index + 1);

        return (n_write != sizeof(block_type));
    }

	void write_range(block_type * buf_begin, size_t first_id, size_t block_count)
	{
        /*int n_write = pwrite(m_fileDescriptor, buf_begin, sizeof(block_type) * block_count, sizeof(block_type) * first_id);
        if(n_write == -1)
            cout << "Write failed: " << strerror(errno) << std::endl;*/
        for(int i = 0; i < block_count; ++i)
            set(first_id + i, *(buf_begin + i));
	}

    void write_range_async(block_type * buf_begin, size_t first_id, size_t block_count)
    {
        aiocb * cb = new aiocb;
    
        memset(cb, 0, sizeof(aiocb));
        cb->aio_nbytes = block_count * sizeof(block_type);
        cb->aio_fildes = m_fileDescriptor;
        cb->aio_offset = sizeof(block_type) * first_id;
        cb->aio_buf = buf_begin;

        int r = aio_write(cb);
        while(r == -1)
        {
            cout << r << "," << aio_error(cb) << std::endl;
            //std::this_thread::yield();
            std::chrono::milliseconds dura(200);
            std::this_thread::sleep_for(dura);

            memset(cb, 0, sizeof(aiocb));
            cb->aio_nbytes = block_count * sizeof(block_type);
            cb->aio_fildes = m_fileDescriptor;
            cb->aio_offset = sizeof(block_type) * first_id;
            cb->aio_buf = buf_begin;
        
            r = aio_write(cb);
        }

        /*if(r == -1)
        {
            cout << "Unable to create write async request!" << endl;
            cout << aio_error(cb) << std::endl;
            throw runtime_error("Failed to write");
        }*/

        m_pendingRequests.push_back(cb);
    }

    bool ready()
    {
        if(!m_pendingRequests.empty())
        {
            auto it = m_pendingRequests.begin();
            while(it != m_pendingRequests.end())
            {
                if(aio_error(*it) != 0)
                    ++it;
                else
                {
                    delete *it;
                    it = m_pendingRequests.erase(it);
                }
            }
        }
        
        return m_pendingRequests.empty();
    }

	int get(size_t index, block_type & data)
    {
        if (index >= m_blockCount)
            return 1;

        int n_read = pread(m_fileDescriptor, &data, sizeof(block_type), sizeof(block_type)*index);
        if(n_read == -1)
        {
            cout << "Read failed (index=" << index << "): " << strerror(errno) << std::endl;
            throw runtime_error("XXX");
        }

        return (n_read != sizeof(block_type));
    }

    //http://en.wikipedia.org/wiki/Vectored_I/O
    void read_range(block_type * buf_begin, size_t first_id, size_t block_count)
    {
        /*int n_read = pread(m_fileDescriptor, buf_begin, sizeof(block_type)*block_count, sizeof(block_type)*first_id);
        if(n_read == -1)
        {
            cout << "Read range failed (index=" << first_id << ", count=" << block_count << "): " << strerror(errno) << std::endl;
            throw runtime_error("I/O failed");
        }*/

        for(int i = 0; i < block_count; ++i)
            get(first_id + i, *(buf_begin + i));
    }

	struct read_req_t
	{
		read_req_t()
		{
		}
	};

	void get(const std::vector<read_request> & requests)
	{
		std::vector<read_req_t> rreqs(requests.size());
	}


	void append(const block_type & data)
	{
        //cout << "Appending block with ID=" << data.id << std::endl;
        int r = set(m_blockCount, data);
        if(r != 0)
            cout << "Appending failed" << std::endl;
        else
            ++m_blockCount;
	}

	size_t size() const
    {
        //return lseek(m_fileDescriptor, 0, SEEK_END);
		return m_blockCount;
    }

    void clear()
    {
        int r = truncate(m_fileName.c_str(), 0);
        m_blockCount = 0;
        if(r != 0)
            cout << "Unable to truncate file!" << std::endl;
    }

	int cache_size() const
	{
		return 0;
	}

    void remove()
    {
        close(m_fileDescriptor);
        ::remove(m_fileName.c_str());
    }

    size_t mem_size() const
    {
        struct stat stat_buf;
        int rc = fstat(m_fileDescriptor, &stat_buf);
        return rc == 0 ? stat_buf.st_size : 0;
    }
private:
    std::string m_fileName;
	size_t m_blockCount;
    std::list<aiocb*> m_pendingRequests;
    int m_fileDescriptor;
};

#endif
