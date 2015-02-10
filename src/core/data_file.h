
#ifndef USERDB_DATA_FILE_H_
#define USERDB_DATA_FILE_H_

#include "utils/helpers.h"
#include <mutex>
#include <iostream>

#ifdef WIN32
    #include <windows.h>

    void onWriteComplete(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <aio.h>
#endif


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
    #ifdef WIN32
		, m_hFile(0), m_asyncCache(20000), m_pendingCount(0)
    #endif
    {
		cout << "Creating data file: " << m_fileName << std::endl;
	#ifdef WIN32
		m_hFile = CreateFile(to_wstring(file_name).c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_NO_BUFFERING, NULL);

		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			throw runtime_error("Unable to open file with WinAPI");
		}
    #else
        m_fileDescriptor = ::open(file_name.c_str(), O_RDWR | O_CREAT, 0644);
        if(m_fileDescriptor == -1)
            throw runtime_error("Unable to open file with unix API");

        //Allocate 1GB
        //set(250000, block_type());
    #endif
    }


	data_file(const data_file & rhs)
	{
		throw runtime_error("Can not be copied");
	}
	/*{
		std::swap(m_hFile, rhs.m_hFile);
	}*/

    ~data_file()
    {
    #ifdef WIN32
		if (m_hFile != 0)
		{
			cout << "Deleting data file: " << m_fileName << std::endl;
			CloseHandle(m_hFile);
		}
    #else
        close(m_fileDescriptor);
    #endif
    }

	/*int open(const std::string file_name)
	{
    #ifdef WIN32
		m_hFile = CreateFile(to_wstring(file_name).c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_NO_BUFFERING, NULL);

		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			throw runtime_error("Unable to open file with WinAPI");
		}
    #endif
		return 0;
	}*/

	int set(size_t index, const block_type & data)
	{
#ifdef WIN32
		//cout << "Writing " << data.meta.id << " to " << index << std::endl;


		if (index > m_blockCount)
		{
			size_t empty_count = index - m_blockCount;
/*			//Create some empty blocks
			block_type empty_block;
			memset(&empty_block, 0, sizeof(block_type));

			OVERLAPPED * p_ol = (OVERLAPPED*)malloc(sizeof(OVERLAPPED)* (empty_count + 2));
			memset(p_ol, 0, sizeof(OVERLAPPED)* (empty_count + 2));

			FILE_SEGMENT_ELEMENT * seg_arr = (FILE_SEGMENT_ELEMENT*)malloc(sizeof(FILE_SEGMENT_ELEMENT)* (empty_count + 2));
			memset(seg_arr, 0, sizeof(FILE_SEGMENT_ELEMENT)* (empty_count + 2));

			for (size_t i = 0; i < empty_count; ++i)
			{
				p_ol[i].Offset = (m_blockCount + i) * sizeof(block_type);
				seg_arr[i].Buffer = &empty_block;
			}

			p_ol[empty_count].Offset = (m_blockCount + empty_count) * sizeof(block_type);
			seg_arr[empty_count].Buffer = (void*)&data;


			bool r = WriteFileGather(m_hFile, seg_arr, sizeof(block_type), NULL, p_ol);
			if (!r)
			{
				DWORD err = ::GetLastError();

				if (err != ERROR_IO_PENDING)
					return 1;

				DWORD num_written = 0;
				r = GetOverlappedResult(m_hFile, p_ol, &num_written, true);
				if (!r)
				{
					return 1;
				}
			}*/

			OVERLAPPED ol = { 0 };
			ol.Offset = 0xFFFFFFFF;
			ol.OffsetHigh = 0xFFFFFFFF;

			size_t iter_count = empty_count / ITERATION_BLOCK_COUNT;
			block_t blocks_buffer[ITERATION_BLOCK_COUNT];
			memset(blocks_buffer, 0, sizeof(blocks_buffer));

			std::vector<DWORD> wrtten(iter_count);

			for (size_t i = 0; i < iter_count; i++)
			{
				bool r = WriteFile(m_hFile, blocks_buffer, sizeof(blocks_buffer), &wrtten[i], &ol);
				if (!r)
					cout << "Write failed" << std::endl;
			}

			m_blockCount = m_blockCount + iter_count * ITERATION_BLOCK_COUNT;
		}
		

		size_t first_byte = index * sizeof(block_type);
		OVERLAPPED ol = { 0 };

		if (first_byte >= 4294967296ULL)
		{
			ol.Offset = first_byte & 0xffff;
			ol.OffsetHigh = (first_byte >> 32ULL) & 0xffff;
		}
		else
			ol.Offset = first_byte;

		bool r = WriteFile(m_hFile, &data, sizeof(block_type), &m_bytesWritten, &ol);
		if (!r)
			cout << "Write failed" << std::endl;

		m_blockCount = max(m_blockCount, index + 1);
		return r ? 0 : 1;
    #else
        int n_write = pwrite(m_fileDescriptor, &data, sizeof(block_type), sizeof(block_type)*index);
        if(n_write == -1)
            cout << "Write failed: " << strerror(errno) << std::endl;

        return (n_write != sizeof(block_type));
    #endif
    }

	void write_range(block_type * buf_begin, size_t first_id, size_t block_count)
	{
    #ifdef WIN32
		//cout << "Writing sequence of " << block_count << std::endl;
		OVERLAPPED ol = { 0 };
		ol.Offset = first_id * sizeof(block_type);
		bool r = WriteFile(m_hFile, buf_begin, sizeof(block_type)* block_count, &m_bytesWritten, &ol);
		m_blockCount = max(m_blockCount, first_id + block_count);

    #else
        int n_write = pwrite(m_fileDescriptor, buf_begin, sizeof(block_type) * block_count, sizeof(block_type) * first_id);
        if(n_write == -1)
            cout << "Write failed: " << strerror(errno) << std::endl;
    #endif
	}

    void write_range_async(block_type * buf_begin, size_t first_id, size_t block_count)
    {
        //cout << "Attempt to write " << first_id << " - " << first_id + block_count << std::endl;

#ifdef WIN32
		int request_id = m_pendingCount++;
		m_asyncCache[request_id] = { 0 };
		if (first_id < m_blockCount)
			m_asyncCache[request_id].Offset = first_id * sizeof(block_type);
		else
		{
			m_asyncCache[request_id].Offset = 0xFFFFFFFF;
			m_asyncCache[request_id].OffsetHigh = 0xFFFFFFFF;
		}

		bool r = WriteFileEx(m_hFile, buf_begin, sizeof(block_type)* block_count, &m_asyncCache[request_id], onWriteComplete);
		m_blockCount = max(m_blockCount, first_id + block_count);
		if (!r)
		{
			DWORD err_code = GetLastError();

			cout << "Write async failed = "<< err_code << "\n";
		}
#else
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
#endif
    }

    bool ready()
    {
#ifdef WIN32
		m_pendingCount = 0;
		return true;
#else
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
#endif
    }

	int get(size_t index, block_type & data)
    {
        if (index >= m_blockCount)
            return 1;
    #ifdef WIN32
		//seek(index);

		OVERLAPPED ol = { 0 };
		ol.Offset = index * sizeof(block_type);
		DWORD bytes_read(0);
		bool res = ReadFile(m_hFile, &data, sizeof(block_type), &bytes_read, &ol);

		return res ? 0 : 1;
    #else
        int n_read = pread(m_fileDescriptor, &data, sizeof(block_type), sizeof(block_type)*index);
        if(n_read == -1)
        {
            cout << "Read failed (index=" << index << "): " << strerror(errno) << std::endl;
            throw runtime_error("XXX");
        }

        return (n_read != sizeof(block_type));
    #endif
    }

	struct read_req_t
	{
		read_req_t()
		{
			ol = { 0 };
		}

		OVERLAPPED ol;
		DWORD bytes_read;
	};

	void get(const std::vector<read_request> & requests)
	{
		std::vector<read_req_t> rreqs(requests.size());

		for (int i = 0; i < requests.size(); ++i)
		{
			auto & req = requests[i];
			rreqs[i].ol.Offset = req.block_id * sizeof(block_type);

			bool res = ReadFile(m_hFile, req.dest_ptr, sizeof(block_type), &rreqs[i].bytes_read, &rreqs[i].ol);
		}

		//Wait all opertaions to complete

		//bool r = ReadFileScatter(m_hFile, segs.data(), sizeof(block_t), NULL,  );
	}


	void append(const block_type & data)
	{
    #ifdef WIN32
		DWORD ptr = SetFilePointer(m_hFile, 0, NULL, FILE_END);

		OVERLAPPED ol = { 0 };
		ol.Offset = 0xFFFFFFFF;
		ol.OffsetHigh = 0xFFFFFFFF;
		bool r = WriteFile(m_hFile, &data, sizeof(block_type), &m_bytesWritten, &ol);
		if (r)
			++m_blockCount;
    #else
        //cout << "Appending block with ID=" << data.id << std::endl;
        int r = set(m_blockCount, data);
        if(r != 0)
            cout << "Appending failed" << std::endl;
        else
            ++m_blockCount;
    #endif
	}

	size_t size() const
    {
        //return lseek(m_fileDescriptor, 0, SEEK_END);
		return m_blockCount;
    }

    void clear()
    {
        /*int r = truncate(m_fileName.c_str(), 0);
        if(r != 0)
            cout << "Unable to truncate file!" << std::endl;*/
    }

	int cache_size() const
	{
		return 0;
	}
private:
	void seek(size_t index)
	{
    #ifdef WIN32
		size_t byte_offset = index * sizeof(block_type);
		DWORD ptr = SetFilePointer(m_hFile, LONG(byte_offset), NULL, FILE_BEGIN);
    #endif
	}
private:
    std::string m_fileName;
	size_t m_blockCount;
#ifdef WIN32
    HANDLE m_hFile;
	DWORD m_bytesWritten;
	std::vector<OVERLAPPED> m_asyncCache;
	int m_pendingCount;
#else
    std::list<aiocb*> m_pendingRequests;
    int m_fileDescriptor;
#endif
};

#endif
