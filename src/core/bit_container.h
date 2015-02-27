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
#include <cassert>
#include <bitset>

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

template<typename It, typename CIt, typename Fun>
void for_each_tri(It first_first, It first_last, CIt second_first, CIt third_first, Fun fun)
{
	for (; (first_first != first_last) && fun(*first_first, *second_first, *third_first); ++first_first, ++second_first, ++third_first);
}

struct ULTRA_CORE_API bit_vector
{
	typedef unsigned int value_type;
	using base_value_t = value_type;
	friend class UMaskedBitVector;

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
		friend class bit_vector;
	private:
		positive_iterator(const bit_vector & container_, size_t bit_index)
			:m_container(container_), m_bitIndex(bit_index)
		{
		}

		const bit_vector & m_container;
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

	bit_vector(const size_t bit_count = 0, bool default_value = false)
		:mData(integer_ceil(bit_count, sizeof(base_value_t) * 8), default_value ? std::numeric_limits<base_value_t>::max() : 0), mBitCount(bit_count)
	{
	}

	bit_vector(const std::initializer_list<bool> & _value);

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

	void resize(int bit_count)
	{
		mData.resize(integer_ceil(bit_count, sizeof(base_value_t)* 8), 0);
		mBitCount = bit_count;

		/*const size_t byte_count = ceil(static_cast<double>(bit_count) / elementBitCount());
		mData.resize(byte_count);

		setValues(value);*/
	}

	void resizeContainer(int element_count)
	{
		mData.resize(element_count);
	}

	/*
	Returns bit count in array.
	*/
	size_t size() const
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

	void set(size_t bit_index, bool value);

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
	friend bit_vector operator|(const bit_vector & bs1, const bit_vector & bs2)
	{
		bit_vector result(bs1);

		for(size_t i = 0; i < bs2.mData.size(); ++i)
			result.mData[i] = result.mData[i] | bs2.mData[i];
		//result.

		return std::move(result);
	}

	/*
	XOR (1 if bits are different)
	*/
	friend bit_vector operator^(const bit_vector & bs1, const bit_vector & bs2)
	{
		bit_vector result(bs1);

		for(size_t i = 0; i < bs2.mData.size(); ++i)
			result.mData[i] = result.mData[i] ^ bs2.mData[i];
		//result.

		return std::move(result);
	}

	/*
	Bitwise AND.
	*/
	friend bit_vector operator&(const bit_vector & bs1, const bit_vector & bs2)
	{
		bit_vector result(bs1);

		for(size_t i = bs2.mData.size(); i > 0; --i)
			result.mData[i-1] = result.mData[i-1] & bs2.mData[i-1];

		return std::move(result);
	}

	friend bit_vector operator~(const bit_vector & bc)
	{
		bit_vector result(bc);

		for(size_t i = 0; i < bc.mData.size(); ++i)
			result.mData[i] = ~ bc.mData[i];

		return std::move(result);
	}

	BitReference operator[](size_t bit_index);
	bool operator[](size_t bit_index) const
	{
#if _DEBUG
		assert(bit_index < mBitCount);
#endif

#if USE_INTRINSIC
		return check_bit<sizeof(value_type)>((void*)mData.data(), bit_index);
#else
		auto val_it = mData.begin() + (bit_index / 8 / sizeof(base_value_t));
		return ((1 << (bit_index % (8 * sizeof(base_value_t)))) & *val_it) != 0;
#endif
	}

	bool at(size_t bit_index) const
	{
		return operator[](bit_index);
	}
	/*
	Returns vector of indices of positive bits.
	*/
	vector<size_t> toIndices() const;
	
	template<typename F>
	void for_each_true(F fun) const
	{
		int val_index = 0, local_bit;
		for (base_value_t cur_val : mData)
		{
			for (local_bit = 0; cur_val != 0; ++local_bit)
			{
				if (cur_val & 1)
					fun(val_index * sizeof(base_value_t) * 8 + local_bit);
				cur_val = cur_val >> 1;
			}
			++val_index;
		}
	}

	/*
	Returns number of bits set to 1.
	*/
	int trueCount() const;

	//Main operations
	//void setMasked(const bit_vector & value, const bit_vector & mask);
	void set_masked(const bit_vector & value, const bit_vector & mask)
	{
#if _DEBUG
		assert((this->mBitCount == value.mBitCount) && (this->mBitCount == mask.mBitCount), "Parameters sizes mismatch");
#endif
	/*	for_each_tri(this->mData.begin(), this->mData.end(), value.mData.cbegin(), mask.mData.cbegin(), [&](base_value_t & this_el, const base_value_t & val_el, const base_value_t & mask_el){
			this_el = (this_el & (~mask_el)) | (val_el & mask_el);
			return true;
		});*/

		auto cur_first = this->mData.begin();
		auto val_first = value.mData.cbegin(), mask_first = mask.mData.cbegin();
		for (int i = this->mData.size(); i != 0; --i, ++cur_first, ++val_first, ++mask_first)
			*cur_first = (*cur_first & (~*mask_first)) | (*val_first & *mask_first);
	}

	//bool equalMasked(const bit_vector & value, const bit_vector & mask) const;

	bool equal_masked(const bit_vector & value, const bit_vector & mask) const
	{
#if _DEBUG
		assert((this->mBitCount == value.mBitCount) && (this->mBitCount == mask.mBitCount), "Parameters sizes mismatch");
#endif
		/*bool res = true;
		for_each_tri(this->mData.cbegin(), this->mData.cend(), value.mData.cbegin(), mask.mData.cbegin(), [&](const base_value_t & this_el, const base_value_t & val_el, const base_value_t & mask_el){
			res = (val_el & mask_el) == (this_el & mask_el);
			return res;
		});

		return res;
*/
		auto cur_first = this->mData.cbegin(), val_first = value.mData.cbegin(), mask_first = mask.mData.cbegin();
		bool r = true;
		for (int i = this->mData.size(); i != 0; --i, ++cur_first, ++val_first, ++mask_first)
			r &= ((*cur_first & *mask_first) == (*val_first & *mask_first));

		return r;
	}

	size_t equalCountMasked(const bit_vector & _value, const bit_vector & _mask) const;

	static void checkSizes(const bit_vector & bc1, const bit_vector & bc2, const bit_vector & bc3);

	size_t serialize(void * dest) const;
	size_t deserialize(char * dest);

	void serialize(std::ofstream & os) const;
	int deserialize(std::ifstream & is);

	template<typename It>
	void remove_indices(It first, It last)
	{
		std::vector<int> indices(first, last);
		std::sort(indices.begin(), indices.end(), std::greater<int>());

		std::vector<bool> data(mBitCount, false);
		for_each_true([&](int idx){
			data[idx] = true;
		});

		for (int idx : indices)
			data.erase(data.begin() + idx);

		resize(data.size());
		std::fill(mData.begin(), mData.end(), 0);
		
		for (int i = 0; i < data.size(); ++i)
			set(i, data[i]);
	}

	friend bool operator==(const bit_vector & bs1, const bit_vector & bs2)
	{
		return (memcmp(bs1.mData.data(), bs2.mData.data(), bs1.mData.size() * sizeof(base_value_t)) == 0);
		//return (memcmp(bs1.mData.data(), bs2.mData.data(), integerbs1.mBitCount) == 0);
	}

	friend bool operator==(const bit_vector & lhs, const std::vector<bool> & rhs)
	{
		if (lhs.bitCount() != rhs.size())
			return false;

		bool r = true;
		for (int i = 0; (i < rhs.size()) && r; ++i)
			r = r && (lhs[i] == rhs[i]);
		return r;
	}

	/*template<typename Container>
	friend bool operator==(const Container & rhs, const bit_vector & lhs)
	{
		return (lhs == rhs);
	}*/

	std::vector<base_value_t> mData;
	size_t mBitCount;
};

//typedef bit_vector UBitContainer;
//typedef UBitContainer UBitset;


namespace std {
template<>
class hash<bit_vector>
{
public:

	size_t operator()(const bit_vector & bv) const
	{
		/*auto * ptr = bv.mData.data();

		size_t r = (*ptr);
		for (size_t i = bv.mData.size() - 1; i > 0; --i)
		{
			r = r ^ (*++ptr);
			//r *= _FNV_prime;
		}


		//return r;
		return std::hash<bit_vector::value_type>()(r);*/

		return _Hash_seq((const unsigned char *)bv.mData.data(), bv.mData.size() * sizeof(bit_vector::base_value_t));
	}
};
}

#endif
