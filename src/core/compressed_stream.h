
#ifndef UltraCore_compressed_stream_h
#define UltraCore_compressed_stream_h

#include "bit_container.h"
#include "masked_bit_vector.h"

class compressed_stream
{
public:
	compressed_stream(const void * ptr)
		:m_dataPtr((char*)ptr), m_curBit(0)
	{}

	template<typename It>
	void write(It first, It last, int bits_per_element)
	{
		for (; first != last; ++first)
			write(*first, bits_per_element);
	}

	void write(int el, int bits_per_element)
	{
		int * el_ptr = (int*)m_dataPtr;
		int val = el << m_curBit;
		int new_mask = ((1 << bits_per_element) - 1) << m_curBit;
		int old_mask = ~new_mask;
		*el_ptr = (new_mask & val) | (old_mask & *el_ptr);

		move_ptr(bits_per_element);
	}

	int read(int bits_per_element)
	{
		int mask = ((1 << bits_per_element) - 1) << m_curBit;
		const int * el_ptr = (const int*)m_dataPtr;

		int res = ((*el_ptr) & mask) >> m_curBit;
		move_ptr(bits_per_element);

		return res;
	}

	template<typename It>
	void read(It first, It last, int bits_per_element)
	{
		for (; first != last; ++first)
			*first = read(bits_per_element);
	}

	void write(const masked_bit_vector & mbv)
	{
		write(mbv.mask);
		write(mbv.value);
	}

	void read(masked_bit_vector & mbv)
	{
		read(mbv.mask);
		read(mbv.value);
	}

	void write(const bit_vector & mbv)
	{
		//memcpy(m_dataPtr, mbv.mData.data(), integer_ceil(mbv.bitCount(), 8));
		//m_dataPtr += mbv.byteCount();

		size_t byte_count = mbv.mData.size() * sizeof(bit_vector::base_value_t);
		memcpy(m_dataPtr, mbv.mData.data(), byte_count);
		m_dataPtr += byte_count;
	}

	void read(bit_vector & mbv)
	{
		//memcpy(mbv.mData.data(), m_dataPtr, integer_ceil(mbv.bitCount(), 8));
		//m_dataPtr += mbv.byteCount();

		size_t byte_count = mbv.mData.size() * sizeof(bit_vector::base_value_t);
		memcpy(mbv.mData.data(), m_dataPtr, byte_count);
		m_dataPtr += byte_count;
	}
private:
	void move_ptr(int bits_count)
	{
		m_curBit += bits_count;
		if (m_curBit >= 8)
		{
			++m_dataPtr;
			m_curBit -= 8;
		}
	}
private:
	char * m_dataPtr;
	int m_curBit;
};

#endif

