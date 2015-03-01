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
#include <stdint.h>

#if USE_INTRINSIC
#include <intrin.h>
#endif

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
#if USE_INTRINSIC
	typedef union{
		__m128i m;
		unsigned __int64 ui64[2];
	} value_type;
#else
	typedef uint64_t value_type;
#endif
	
	using base_value_t = value_type;
	friend class UMaskedBitVector;

	struct BitReference
	{
		BitReference(value_type & data_reference, const int bit_index)
			:m_dataRef(data_reference), mBitIndex(bit_index)
		{}

		BitReference & operator=(bool value)
		{
#if USE_INTRINSIC
			if (value)
				_bittestandset64((long long *)&m_dataRef, mBitIndex);
			else
				_bittestandreset64((long long *)&m_dataRef, mBitIndex);
#else
			base_value_t val = (base_value_t)1 << mBitIndex;
			m_dataRef = value ? (m_dataRef | val) : (m_dataRef & ~val);
#endif
			return *this;
		}

		operator bool() const
		{
#if USE_INTRINSIC
			return _bittest64((long long *)&m_dataRef, mBitIndex);
#else
			value_type val = 1 << mBitIndex;
			value_type result = m_dataRef & val;
			if(result)
				return true;
			else
				return false;
#endif
		}

		base_value_t & m_dataRef;
		const int mBitIndex;
	};

	bit_vector(const size_t bit_count = 0, bool default_value = false)
		:mBitCount(bit_count), mData(integer_ceil(bit_count, sizeof(base_value_t)* 8)
#if !USE_INTRINSIC
		, default_value ? std::numeric_limits<base_value_t>::max() : 0
#endif
		)
	{
#if USE_INTRINSIC
		memset(mData.data(), default_value ? 0xff : 0x00, mData.size() * sizeof(base_value_t));
#endif
	}

	bit_vector(const std::initializer_list<bool> & _value);

	void clear()
	{
		setValues(false);
	}

	//Sets all last bytes to zeros for preventig errors
	/*void clearTail()
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
	}*/

	void resize(int bit_count)
	{
#if USE_INTRINSIC
		base_value_t zero_val;
		zero_val.ui64[0] = 0;
		zero_val.ui64[1] = 0;
		mData.resize(integer_ceil(bit_count, sizeof(base_value_t)* 8), zero_val);
#else
		mData.resize(integer_ceil(bit_count, sizeof(base_value_t)* 8), 0);
#endif
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
	/*size_t byteCount() const
	{
		return sizeof(value_type) * mData.size();
	}*/

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
	/*friend bit_vector operator|(const bit_vector & bs1, const bit_vector & bs2)
	{
		bit_vector result(bs1);

		for(size_t i = 0; i < bs2.mData.size(); ++i)
			result.mData[i] = result.mData[i] | bs2.mData[i];
		//result.

		return std::move(result);
	}*/

	/*
	XOR (1 if bits are different)
	*/
	friend bit_vector operator^(const bit_vector & bs1, const bit_vector & bs2)
	{
		bit_vector result(bs1);

#if USE_INTRINSIC
		for (size_t i = 0, sz = bs2.mData.size(); i < sz; ++i)
			result.mData[i].m = _mm_xor_si128(result.mData[i].m, bs2.mData[i].m);
#else
		for(size_t i = 0; i < bs2.mData.size(); ++i)
			result.mData[i] = result.mData[i] ^ bs2.mData[i];
#endif
		return std::move(result);
	}

	/*
	Bitwise AND.
	*/
	friend bit_vector operator&(const bit_vector & bs1, const bit_vector & bs2)
	{
		bit_vector result(bs1);
#if USE_INTRINSIC
		for(size_t i = 0, sz = bs2.mData.size(); i < sz; ++i)
			result.mData[i].m = _mm_and_si128(result.mData[i].m, bs2.mData[i].m);
#else
		for(size_t i = bs2.mData.size(); i > 0; --i)
			result.mData[i-1] = result.mData[i-1] & bs2.mData[i-1];
#endif

		return std::move(result);
	}

	friend bit_vector operator~(const bit_vector & bc)
	{
		bit_vector result(bc);
#if USE_INTRINSIC
		for (size_t i = 0, sz = bc.mData.size(); i < sz; ++i)
		{
			result.mData[i].ui64[0] = ~ bc.mData[i].ui64[0];
			result.mData[i].ui64[1] = ~ bc.mData[i].ui64[1];
		}
#else
		for(size_t i = 0; i < bc.mData.size(); ++i)
			result.mData[i] = ~ bc.mData[i];
#endif
		return std::move(result);
	}

	BitReference bit_vector::operator[](size_t bit_index)
	{
#if _DEBUG
		assert(bit_index < mBitCount);
#endif
		return BitReference(mData[bit_index / 8 / sizeof(base_value_t)], bit_index % (8 * sizeof(base_value_t)));
	}

	bool operator[](size_t bit_index) const
	{
#if _DEBUG
		assert(bit_index < mBitCount);
#endif
		
#if USE_INTRINSIC
		return _bittest64((const long long *)mData.data(), bit_index);
#else
		auto val_it = mData.begin() + (bit_index / 8 / sizeof(base_value_t));
		return (((base_value_t)1 << (bit_index % (8 * sizeof(base_value_t)))) & *val_it) != 0;
#endif
	}

	void set(size_t bit_index, bool value)
	{
		
#if USE_INTRINSIC
		if (value)
			_bittestandset64((long long*)mData.data(), bit_index);
		else
			_bittestandreset64((long long*)mData.data(), bit_index);
#else
		auto val_it = mData.begin() + (bit_index / 8 / sizeof(base_value_t));
		base_value_t bit_idx = (bit_index % (8 * sizeof(base_value_t)));

		base_value_t mask = (base_value_t)1 << bit_idx;
		*val_it = (*val_it & (~mask)) | (((base_value_t)value << bit_idx) & mask);
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
#if USE_INTRINSIC
		int val_index = 0, local_bit;
		int sz = mData.size() * 2;
		for (const unsigned __int64 * p_val = mData.begin()->ui64; val_index < sz; ++val_index, ++p_val)
		{
			auto cur_val = *p_val;
			for (local_bit = 0; cur_val != 0; ++local_bit)
			{
				if (cur_val & 1)
				{
					int res_idx = val_index * sizeof(unsigned __int64)* 8 + local_bit;
#if _DEBUG
					assert(res_idx < mBitCount);
#endif
					fun(res_idx);
				}
				cur_val = cur_val >> 1;
			}
		}
#else
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
#endif
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
		auto cur_first = this->mData.begin();
		auto val_first = value.mData.cbegin(), mask_first = mask.mData.cbegin();
#if USE_INTRINSIC
		for (int i = 0, sz = this->mData.size(); i < sz; ++i, ++cur_first, ++val_first, ++mask_first)
			cur_first->m = _mm_or_si128(_mm_andnot_si128(mask_first->m, cur_first->m), _mm_and_si128(mask_first->m, val_first->m));
#else
		for (int i = this->mData.size(); i != 0; --i, ++cur_first, ++val_first, ++mask_first)
			*cur_first = (*cur_first & (~*mask_first)) | (*val_first & *mask_first);
#endif
	}

	//bool equalMasked(const bit_vector & value, const bit_vector & mask) const;

	bool equal_masked(const bit_vector & value, const bit_vector & mask) const
	{
#if _DEBUG
		assert((this->mBitCount == value.mBitCount) && (this->mBitCount == mask.mBitCount), "Parameters sizes mismatch");
#endif

		auto cur_first = this->mData.cbegin(), val_first = value.mData.cbegin(), mask_first = mask.mData.cbegin();
#if USE_INTRINSIC
		
		int res = 1;
		for (int i = 0, sz = this->mData.size(); i < sz; ++i, ++cur_first, ++val_first, ++mask_first)
		{
			__m128i neq = _mm_xor_si128(_mm_and_si128(cur_first->m, mask_first->m), _mm_and_si128(val_first->m, mask_first->m));
			res = res & _mm_test_all_zeros(neq, neq);
			/*tmp = _mm_cmpeq_epi64(_mm_and_si128(cur_first->m, mask_first->m), _mm_and_si128(val_first->m, mask_first->m));
			res &= tmp.m128i_u8[0] & tmp.m128i_u8[8];*/
		}

		return (res != 0);
#else
		bool r = true;
		for (int i = this->mData.size(); i != 0; --i, ++cur_first, ++val_first, ++mask_first)
			r &= ((*cur_first & *mask_first) == (*val_first & *mask_first));	
		return r;
#endif
		
	}

	//size_t equalCountMasked(const bit_vector & _value, const bit_vector & _mask) const;

	static void checkSizes(const bit_vector & bc1, const bit_vector & bc2, const bit_vector & bc3);

	/*size_t serialize(void * dest) const;
	size_t deserialize(char * dest);

	void serialize(std::ofstream & os) const;
	int deserialize(std::ifstream & is);*/

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

#if USE_INTRINSIC
		memset(mData.data(), 0x00, mData.size() * sizeof(base_value_t));
#else
		std::fill(mData.begin(), mData.end(), 0);
#endif
		
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
