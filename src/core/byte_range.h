
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
	}

	byte_range(byte_range && rhs)
	{
		cout << "xxx";
	}*/

	byte_range & operator=(const byte_range & rhs)
	{
		if ((this->start == nullptr) || (this->size != rhs.size))
			throw std::out_of_range("Invalid byterange assignment");

		memcpy(this->start, rhs.start, this->size);
		return *this;
	}

	template<typename T>
	byte_range(const T & rhs)
	{
		auto res = rhs.to_byte_range();
		this->start = res.start;
		this->size = res.size;
	}

	template<typename T>
	byte_range & operator=(const T & rhs)
	{
		auto res = rhs.to_byte_range();

		if ((this->start == nullptr) || (this->size != res.size))
			throw std::out_of_range("Invalid byterange assignment");

		memcpy(this->start, res.start, this->size);
		return *this;
	}

	friend bool operator==(const byte_range & lhs, const byte_range & rhs)
	{
		return (lhs.size == rhs.size) && (memcmp(lhs.start, rhs.start, lhs.size) == 0);
	}

	/*friend bool operator==(const byte_range & lhs, size_t val)
	{
		return *((size_t*)lhs.start) == val;
	}*/

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

	friend bool operator<(const byte_range & lhs, const byte_range & rhs)
	{
		if (lhs.begin_as<size_t>() < rhs.begin_as<size_t>())
			return true;
		else if (rhs.begin_as<size_t>() < lhs.begin_as<size_t>())
			return false;
		else
			return (memcmp(lhs.start + sizeof(size_t), rhs.start + sizeof(size_t), lhs.size - sizeof(size_t)) < 0);
	}

	friend bool operator<=(const byte_range & lhs, const byte_range & rhs)
	{
		if (lhs.begin_as<size_t>() < rhs.begin_as<size_t>())
			return true;
		else if (rhs.begin_as<size_t>() < lhs.begin_as<size_t>())
			return false;
		else
			return (memcmp(lhs.start + sizeof(size_t), rhs.start + sizeof(size_t), lhs.size - sizeof(size_t)) <= 0);
	}

	template<typename T>
	const T & begin_as() const
	{
		return *((T*)this->start);
	}


	char * start;
	size_t size;
};

namespace std
{
	template<>
	inline void swap<byte_range>(byte_range & lhs, byte_range & rhs)
	{
		int x = 0;
	}
};

#endif
