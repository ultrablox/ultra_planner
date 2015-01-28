
#ifndef UltraCore_byte_range_h
#define UltraCore_byte_range_h

#include <string>
#include <stdexcept>

struct byte_range
{
	byte_range(char * _start, size_t _size)
	:start(_start), size(_size)
	{}

	/*byte_range(const byte_range & rhs)
	:start(rhs.start), size(rhs.size)
	{
	cout << "xxx";
	}*/

	byte_range & operator=(const byte_range & rhs)
	{
		if (this->size != rhs.size)
			throw std::out_of_range("Invalid byterange assignment");

		memcpy(this->start, rhs.start, this->size);
		return *this;
	}

	friend bool operator==(const byte_range & lhs, const byte_range & rhs)
	{
		return (lhs.size == rhs.size) && (memcmp(lhs.start, rhs.start, lhs.size) == 0);
	}

	friend bool operator==(const byte_range & lhs, size_t val)
	{
		return *((size_t*)lhs.start) == val;
	}

	friend bool operator<(const byte_range & lhs, size_t val)
	{
		return *((size_t*)lhs.start) < val;
	}

	friend bool operator>(size_t val, const byte_range & rhs)
	{
		return val > *((size_t*)rhs.start);
	}

	friend bool operator<=(size_t val, const byte_range & rhs)
	{
		return val <= *((size_t*)rhs.start);
	}

	char * start;
	size_t size;
};

#endif
