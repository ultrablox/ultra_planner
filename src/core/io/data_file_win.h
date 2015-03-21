
#ifndef USERDB_DATA_FILE_H_
#define USERDB_DATA_FILE_H_

#include "../utils/helpers.h"
#include <mutex>
#include <iostream>
#include <windows.h>
void onWriteComplete(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);



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
		:m_fileName(file_name), m_blockCount(0), m_hFile(0), m_asyncCache(20000), m_pendingCount(0)
    {
		//cout << "Creating data file: " << m_fileName << std::endl;
		m_hFile = CreateFileW(to_wstring(file_name).c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_NO_BUFFERING, NULL);

		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			throw runtime_error("Unable to open file with WinAPI");
		}
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
		close();
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

	OVERLAPPED to_win_address(size_t index) const
	{
		size_t first_byte = index * sizeof(block_type);
		OVERLAPPED ol = { 0 };
		ol.Offset = first_byte & 0xffffffff;
		ol.OffsetHigh = (first_byte >> 32ULL) & 0xffffffff;
		return std::move(ol);
	}

	int set(size_t index, const block_type & data)
	{
		if (index > m_blockCount)
		{
			size_t empty_count = index - m_blockCount;

			OVERLAPPED ol = { 0 };
			ol.Offset = 0xFFFFFFFF;
			ol.OffsetHigh = 0xFFFFFFFF;

			block_t empty_block;

			FILE_SEGMENT_ELEMENT any_seg;
			any_seg.Buffer = &empty_block;

			size_t iter_block_count = 4294967296ULL / sizeof(block_t) / 2;

			std::vector<FILE_SEGMENT_ELEMENT> segments(iter_block_count + 1, any_seg);
			memset(&segments[iter_block_count], 0, sizeof(FILE_SEGMENT_ELEMENT));

			size_t iter_count = integer_ceil(empty_count, iter_block_count);
			for (int i = 0; i < iter_count; ++i)
			{
				bool r = WriteFileGather(m_hFile, segments.data(), iter_block_count * sizeof(block_type), NULL, &ol);
			}
		}
	
		OVERLAPPED ol = to_win_address(index);

		bool r = WriteFile(m_hFile, &data, sizeof(block_type), &m_bytesWritten, &ol);
		if (!r)
			cout << "Write failed" << std::endl;

		m_blockCount = max(m_blockCount, index + 1);
		return r ? 0 : 1;
    }

	void write_range(block_type * buf_begin, size_t first_id, size_t block_count)
	{
		//cout << "Writing sequence of " << block_count << std::endl;
		OVERLAPPED ol = to_win_address(first_id);
		bool r = WriteFile(m_hFile, buf_begin, sizeof(block_type)* block_count, &m_bytesWritten, &ol);
		m_blockCount = max(m_blockCount, first_id + block_count);
	}

    void write_range_async(block_type * buf_begin, size_t first_id, size_t block_count)
    {
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
    }

    bool ready()
    {
		m_pendingCount = 0;
		return true;
    }

	int get(size_t index, block_type & data)
    {
        if (index >= m_blockCount)
            return 1;
		//seek(index);

		OVERLAPPED ol = to_win_address(index);
		DWORD bytes_read(0);
		bool res = ReadFile(m_hFile, &data, sizeof(block_type), &bytes_read, &ol);

		return res ? 0 : 1;
    }

	void read_range(block_type * buf_begin, size_t first_id, size_t block_count)
	{
		OVERLAPPED ol = to_win_address(first_id);
		DWORD bytes_read(0);
		bool res = ReadFile(m_hFile, buf_begin, sizeof(block_type)* block_count, &bytes_read, &ol);

		if (!res)
			cout << "Read failed" << std::endl;
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
		DWORD ptr = SetFilePointer(m_hFile, 0, NULL, FILE_END);

		OVERLAPPED ol = { 0 };
		ol.Offset = 0xFFFFFFFF;
		ol.OffsetHigh = 0xFFFFFFFF;
		bool r = WriteFile(m_hFile, &data, sizeof(block_type), &m_bytesWritten, &ol);
		if (r)
			++m_blockCount;
	}

	size_t size() const
    {
        //return lseek(m_fileDescriptor, 0, SEEK_END);
		return m_blockCount;
    }

    void clear()
    {
		DWORD ptr = SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
		bool r = SetEndOfFile(m_hFile);
		m_blockCount = 0;
        /*int r = truncate(m_fileName.c_str(), 0);
        if(r != 0)
            cout << "Unable to truncate file!" << std::endl;*/
    }

	int cache_size() const
	{
		return 0;
	}

	bool empty() const
	{
		return (m_blockCount == 0);
	}

	void remove()
	{
		close();
		::remove(m_fileName.c_str());
	}

	void close()
	{
		if (m_hFile != 0)
		{
			//cout << "Deleting data file: " << m_fileName << std::endl;
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
	}

	size_t mem_size() const
	{
		LARGE_INTEGER lFileSize;
		bool res = GetFileSizeEx(m_hFile, &lFileSize);
		return static_cast<size_t>(lFileSize.QuadPart);
	}
private:
	void seek(size_t index)
	{
		size_t byte_offset = index * sizeof(block_type);
		DWORD ptr = SetFilePointer(m_hFile, LONG(byte_offset), NULL, FILE_BEGIN);
	}
private:
    std::string m_fileName;
	size_t m_blockCount;
    HANDLE m_hFile;
	DWORD m_bytesWritten;
	std::vector<OVERLAPPED> m_asyncCache;
	int m_pendingCount;
};

#endif
