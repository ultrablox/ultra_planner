
#ifndef USERDB_DATA_FILE_H_
#define USERDB_DATA_FILE_H_

#include "utils/helpers.h"
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

#ifdef WIN32
    #include <windows.h>
#endif

using namespace std;

/*
Linear container of fixed-length data elements.
*/
template<typename T>
class data_file
{
    using block_type = T;
    //using data_vec_t = std::vector<block_type>;

    static_assert((sizeof(block_type) % 4096) == 0, "ERROR: user_record_data size is not 4K/divisible");
public:
    data_file(const std::string & file_name)
		:m_fileName(file_name), m_blockCount(0)
    #ifdef WIN32
        , m_hFile(0)
    #endif
    {
		cout << "Creating data file: " << m_fileName << std::endl;
    }

	/*data_file(data_file & rhs)
	{
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
    #endif
    }

	int open(const std::string file_name)
	{
    #ifdef WIN32
		m_hFile = CreateFile(to_wstring(file_name).c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_NO_BUFFERING, NULL);

		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			throw runtime_error("Unable to open file with WinAPI");
		}
    #endif
		return 0;
	}

    int set(size_t index, const block_type & data)
    {
    #ifdef WIN32
		//cout << "Writing " << data.val << " to " << index << std::endl;
		OVERLAPPED ol = { 0 };
		

		if (index > m_blockCount)
		{
			//Create some empty blocks
			block_type empty_block;
			memset(&empty_block, 0, sizeof(block_type));

			for (size_t i = m_blockCount; i < index; ++i)
			{
				append(empty_block);
			}

			ol.Offset = 0xFFFFFFFF;
			ol.OffsetHigh = 0xFFFFFFFF;
		}
		else
		{
			ol.Offset = index * sizeof(block_type);
		//	seek(index);
		}

		
		bool r = WriteFile(m_hFile, &data, sizeof(block_type), &m_bytesWritten, &ol);

		return r ? 0 : 1;
    #endif
        return 0;
    }

	int get(size_t index, block_type & data)
    {
    #ifdef WIN32
		if (index >= m_blockCount)
			return 1;

		//seek(index);

		OVERLAPPED ol = { 0 };
		ol.Offset = index * sizeof(block_type);
		DWORD bytes_read(0);
		bool res = ReadFile(m_hFile, &data, sizeof(block_type), &bytes_read, &ol);

		return res ? 0 : 1;
    #endif
        return 0;
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
    int m_fileDescriptor;
	size_t m_blockCount;
#ifdef WIN32
    HANDLE m_hFile;
	DWORD m_bytesWritten;
#endif
};

#endif
