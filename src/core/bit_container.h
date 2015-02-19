#ifndef UltraCore_UBitset_h
#define UltraCore_UBitset_h

#include <vector>
#include <iostream>
#include <sstream>
#include <cmath>
//#include <xstddef>
#include <fstream>
#include "config.h"
#include "utils/helpers.h"

using namespace std;
using namespace Serializing;

//#define BITSET_TYPE int
/*
http://mystic2000.newmail.ru/

http://www.rsdn.ru/forum/flame.comp/2720100.flat.2

const int       lsz64_tbl[64] =
{
    0, 31,  4, 33, 60, 15, 12, 34,
   61, 25, 51, 10, 56, 20, 22, 35,
   62, 30,  3, 54, 52, 24, 42, 19,
   57, 29,  2, 44, 47, 28,  1, 36,
   63, 32, 59,  5,  6, 50, 55,  7,
   16, 53, 13, 41,  8, 43, 46, 17,
   26, 58, 49, 14, 11, 40,  9, 45,
   21, 48, 39, 23, 18, 38, 37, 27,
};

int  bitScan (const U64 bb)
{
   const U64 lsb = (bb & -(long long) bb) - 1;
   const unsigned int foldedLSB = ((int) lsb) ^ ((int) (lsb >> 32));
   return lsz64_tbl[foldedLSB * 0x78291ACF >> 26];
}

*/

struct UBitContainer
{
	typedef int value_type;
	friend class UMaskedBitVector;

	//typedef T byte;
	typedef UBitContainer BC;

	struct BitReference
	{
		BitReference(value_type & data_reference, const int bit_index);
		BitReference & operator=(const bool value);
		operator bool() const;
		bool value() const;

		value_type & mDataReference;
		const int mBitIndex;
	};

	class positive_iterator
	{
		friend class UBitContainer;
	private:
		positive_iterator(const UBitContainer & container_, size_t bit_index)
			:m_container(container_), m_bitIndex(bit_index)
		{
		}

		const UBitContainer & m_container;
		size_t m_bitIndex;

	public:
		size_t index() const
		{
			return m_bitIndex;
		}

		positive_iterator & operator++();

		friend bool operator!=(const positive_iterator & i1, const positive_iterator & i2)
		{
			return i1.m_bitIndex != i2.m_bitIndex;
		}
	};

	UBitContainer(const size_t bit_count = 0)
	{
		//cout << "Creating bitset with " << elementBitCount() << "-bit base type\n";
		resize(bit_count);
	}

	UBitContainer(const std::initializer_list<bool> & _value)
	{
		resize(_value.size());

		size_t i = 0;
		for (auto _bit : _value)
			set(i++, _bit);
	}

	~UBitContainer()
	{
	}

	positive_iterator pbegin() const;

	positive_iterator pend() const
	{
		return positive_iterator(*this, mData.size() * sizeof(value_type) * 8);
	}

	void clear()
	{
		setValues(false);
	}

	//Sets all last bytes to zeros for preventig errors
	void clearTail()
	{
		const size_t data_size = mData.size();
		if(!data_size)
			return;

		const size_t start_bit = mBitCount - (data_size - 1) * elementBitCount();

		value_type mask;
		memset(&mask, 0xff, sizeof(value_type));
		mask = mask >> start_bit;

		value_type & res = mData[data_size - 1];
		res = res & mask;
	}

	void resize(const int bit_count, const bool value = false)
	{
		mBitCount = bit_count;

		const size_t byte_count = ceil(static_cast<double>(bit_count) / elementBitCount());
		mData.resize(byte_count);

		setValues(value);
	}

	void resizeContainer(int element_count)
	{
		mData.resize(element_count);
	}

	/*
	Returns bit count in array.
	*/
	size_t size()
	{
		return mBitCount;
	}

	int bitCount() const
	{
		return mBitCount;
	}

	/*
	Returns total byte count spent to bitset.
	*/
	size_t byteCount() const
	{
		return sizeof(value_type) * mData.size();
	}

	static int elementBitCount()
	{
		return elementByteCount() * 8;
	}

	static int elementByteCount()
	{
		return sizeof(value_type);
	}

	/*
	Returns local global byte index and local bit index in pair.
	*/
	static std::pair<size_t, int> getBitAddress(size_t global_bit_index)
	{
		//Get byte index
		const size_t byte_index = global_bit_index / elementBitCount();//floor(static_cast<double>(global_bit_index) / elementBitCount());
		
		//Calculate local bit index
		const int local_bit_index = global_bit_index % elementBitCount();//global_bit_index - (elementBitCount() * byte_index);

		return std::make_pair(byte_index, local_bit_index);
	}

	void set(const size_t bit_index, const bool value);

	void setValues(const bool value);

	//template<typename Stream>
	void print(ostream & os = std::cout, bool add_line_return = true) const
	{
		for(size_t i = 0; i < mBitCount; ++i)
		{
			if(operator[](i))
				os << "1";
			else
				os << "0";
		}

		if(add_line_return)
			os << "\n";
	}


	const std::string toString()
	{
		stringstream result;
		print(result, false);
		return result.str();
	}

	/*
	Bitwise OR.
	*/
	friend BC operator|(const BC & bs1, const BC & bs2)
	{
		BC result(bs1);

		for(size_t i = 0; i < bs2.mData.size(); ++i)
			result.mData[i] = result.mData[i] | bs2.mData[i];
		//result.

		return result;
	}

	/*
	XOR (1 if bits are different)
	*/
	friend BC operator^(const BC & bs1, const BC & bs2)
	{
		BC result(bs1);

		for(size_t i = 0; i < bs2.mData.size(); ++i)
			result.mData[i] = result.mData[i] ^ bs2.mData[i];
		//result.

		return result;
	}

	/*
	Bitwise AND.
	*/
	friend BC operator&(const BC & bs1, const BC & bs2)
	{
		BC result(bs1);

		for(size_t i = bs2.mData.size(); i > 0; --i)
			result.mData[i-1] = result.mData[i-1] & bs2.mData[i-1];

		return result;
	}

	friend bool operator==(const BC & bs1, const BC & bs2)
	{
		return (memcmp(bs1.mData.data(), bs2.mData.data(), bs1.mData.size()*sizeof(value_type)) == 0);
	}

	friend BC operator~(const BC & bc)
	{
		BC result(bc);

		for(size_t i = 0; i < bc.mData.size(); ++i)
			result.mData[i] = ~ bc.mData[i];

		return result;
	}

	BitReference operator[](const size_t bit_index);
	bool operator[](const size_t bit_index) const;

	/*
	Returns vector of indices of positive bits.
	*/
	vector<size_t> toIndices() const;

	/*
	Returns number of bits set to 1.
	*/
	int trueCount() const;

	//Main operations
	void setMasked(const BC & value, const BC & mask);
	bool equalMasked(const BC & value, const BC & mask) const;
	size_t equalCountMasked(const BC & _value, const BC & _mask) const;

	size_t hash() const
	{
		//const size_t _FNV_offset_basis = 14695981039346656037ULL;
		//const size_t _FNV_prime = 1099511628211ULL;

		/*return _Hash_seq((const unsigned char *)mData.data(), mData.size() * sizeof(T) / sizeof(const unsigned char *));*/
		auto * ptr = mData.data();

		size_t r = (*ptr);
		for(size_t i = mData.size() - 1; i > 0; --i)
		{
			r = r ^ (*++ptr);
			//r *= _FNV_prime;
		}

		
		//return r;
		return std::hash<value_type>()(r);
	}

	static void checkSizes(const BC & bc1, const BC & bc2, const BC & bc3);

	size_t serialize(void * dest) const;
	size_t deserialize(char * dest);

	void serialize(std::ofstream & os) const;
	int deserialize(std::ifstream & is);


	std::vector<value_type> mData;
	size_t mBitCount;
};

typedef UBitContainer UBitset;
typedef UBitContainer bit_vector;

#endif
