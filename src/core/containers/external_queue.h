
#ifndef UltraCore_external_queue_h
#define UltraCore_external_queue_h

#include "../io/data_file.h"
#include <queue>

template<typename T>
class named_queue : public std::queue<T>
{
public:
	named_queue(const std::string & fake_name = "queue")
	{}
};


template<typename T>
class external_queue
{
	const static unsigned cached_block_count = 128;

public:

	using value_type = T;

	external_queue(const std::string & file_name)
		:m_file1(file_name + "_1.dat"), m_file2(file_name + "_2.dat"), m_pReadFile(&m_file1), m_pWriteFile(&m_file2), m_size(0)
	{
		m_frontCache.data.resize(cached_block_count);
		m_frontCache.firstIndex = 0;
		m_frontCache.lastIndex = 0;
		m_frontCache.lastFileIndex = 0;

		m_tailCache.data.resize(cached_block_count);
		m_tailCache.lastIndex = 0;
	}

	~external_queue()
	{
		m_file1.remove();
		m_file2.remove();
	}

	value_type & front()
	{
		return m_frontCache.data[m_frontCache.firstIndex];
	}

	const value_type & front() const
	{
		return m_frontCache.data[m_frontCache.firstIndex];
	}

	value_type & back()
	{
		if (m_tailCache.lastIndex == 0)
			return m_frontCache.data[0];
		else
			return m_tailCache.data[m_tailCache.lastIndex - 1];
	}

	void pop()
	{
		if (m_frontCache.firstIndex == m_frontCache.lastIndex)	//Read from file
		{
			if (m_frontCache.lastFileIndex == m_pReadFile->size())
				swap_buffers();

			int step_size = std::min(m_pReadFile->size() - m_frontCache.lastFileIndex, (size_t)cached_block_count);

			m_pReadFile->read_range(m_frontCache.data.data(), m_frontCache.lastFileIndex, step_size);
			m_frontCache.lastIndex = step_size - 1;
			m_frontCache.lastFileIndex += step_size;
			m_frontCache.firstIndex = 0;
		}
		else
			++m_frontCache.firstIndex;

		--m_size;
	}

	void push(const value_type & val)
	{
		if (m_size == 0)
		{
			m_frontCache.firstIndex = 0;
			m_frontCache.lastIndex = 0;
			m_frontCache.data[0] = val;
		}
		else
		{
			if (m_tailCache.lastIndex == cached_block_count)
				flush_tail();

			m_tailCache.data[m_tailCache.lastIndex++] = val;
		}
		
		++m_size;
	}

	bool empty() const
	{
		return (m_size == 0);
	}
private:
	void flush_tail()
	{
		size_t end_id = m_pWriteFile->size();
		m_pWriteFile->write_range(m_tailCache.data.data(), end_id, m_tailCache.lastIndex);
		m_tailCache.lastIndex = 0;
	}

	void swap_buffers()
	{
		flush_tail();
		m_pReadFile->clear();
		std::swap(m_pReadFile, m_pWriteFile);

		m_frontCache.firstIndex = 0;
		m_frontCache.lastIndex = 0;
		m_frontCache.lastFileIndex = 0;
	}
private:
	struct {
		std::vector<value_type> data;
		int firstIndex, lastIndex;
		size_t lastFileIndex;
	} m_frontCache;

	struct {
		std::vector<value_type> data;
		int lastIndex;
	} m_tailCache;


	data_file<value_type> m_file1, m_file2;
	data_file<value_type> *m_pReadFile, *m_pWriteFile;
	size_t m_size;
};

#endif
