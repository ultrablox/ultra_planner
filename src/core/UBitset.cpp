
#include "UBitset.h"
#include "UError.h"
//#include <vld.h>
//#include <intrin.h>
#include <type_traits>

//========================Bit reference===================
UBitContainer::BitReference::BitReference(value_type & data_reference, const int bit_index)
	:mDataReference(data_reference), mBitIndex(bit_index)
{
}

UBitContainer::BitReference & UBitContainer::BitReference::operator=(const bool value)
{
	value_type val = 1ULL << mBitIndex;

	if(value)
		mDataReference = mDataReference | val;
	else
		mDataReference = mDataReference & ~val;

	return *this;
}

UBitContainer::BitReference::operator bool() const
{
	return value();
}

bool UBitContainer::BitReference::value() const
{
	value_type val = 1 << mBitIndex;
	value_type result = mDataReference & val;
	if(result)
		return true;
	else
		return false;
}

//========================Bitset==========================

UBitContainer::positive_iterator & UBitContainer::positive_iterator::operator++()
{
	/*const int bits_per_element = sizeof(UBitContainer::value_type) * 8;
	size_t cont_elem_index = m_bitIndex >> 64;
	auto mask = m_container.mData[cont_elem_index];

	mask = mask >> m_bitIndex;*/

	do
	{
		++m_bitIndex;
	}while((*this != m_container.pend()) && (!m_container[m_bitIndex]));

	return *this;
}

UBitContainer::positive_iterator UBitContainer::pbegin() const
{
	//Find first set bit
	/*unsigned long index;
	
	auto isNonzero = _BitScanForward64(&index, mData[0]);
	*/
	positive_iterator res(*this, 0);
	while((!operator[](res.m_bitIndex)) && (res != pend()))
		++res.m_bitIndex;
	return res;
}

void UBitContainer::set(const size_t bit_index, const bool value)
{
#if USE_INTRINSIC
	if(value)
		_bittestandcomplement64((long long*)mData.data(), bit_index);
	else
		_bittestandreset64((long long*)mData.data(), bit_index);
#else
	operator[](bit_index) = value;
#endif
}

void UBitContainer::setValues(const bool value)
{
	//T val = 0;
	if(value)
		memset(mData.data(), 0xff, sizeof(value_type) * mData.size());
	else
		memset(mData.data(), 0x0, sizeof(value_type) * mData.size());

	clearTail();
}

UBitContainer::BitReference UBitContainer::operator[](const size_t bit_index)
{
	//Calculate address
	auto addr = getBitAddress(bit_index);

	BitReference ref(mData[addr.first], addr.second);
	return ref;
}

#if USE_INTRINSIC

template<int N>
bool check_bit(void * data, const size_t bit_index)
{
	return _bittest64(data, bit_index);
}

template<>
bool check_bit<4>(void * data, const size_t bit_index)
{
	return _bittest((const long*)data, bit_index);
}
#endif

bool UBitContainer::operator[](const size_t bit_index) const
{
#if USE_INTRINSIC
	return check_bit<sizeof(value_type)>((void*)mData.data(), bit_index);
#else
	auto addr = getBitAddress(bit_index);
	value_type val = 1 << addr.second;
	value_type result = mData[addr.first] & val;
	//const BitReference ref(mData[addr.first], addr.second);
	return bool(result);
#endif
}

vector<size_t> UBitContainer::toIndices() const
{
	vector<size_t> result;

	for(size_t i = 0; i < mBitCount; ++i)
	{
		if((*this)[i])
			result.push_back(i);
	}

	return result;
}

int UBitContainer::trueCount() const
{
#if USE_INTRINSIC
	int res(0);

	for(auto el : mData)
		res += __popcnt(el);

	return res;
#else
    return toIndices().size();
#endif
}

void UBitContainer::setMasked(const BC & value, const BC & mask)
{
	checkSizes(*this, value, mask);

	value_type * cur_ptr = mData.data();
	const value_type * val_ptr = value.mData.data(), *mask_ptr = mask.mData.data();

	for(size_t bit_index = 0, max_bit = mData.size(); bit_index < max_bit; ++bit_index)
	{
		*cur_ptr = ((*cur_ptr) & (~(*mask_ptr))) | ((*val_ptr) & (*mask_ptr));
		++cur_ptr;
		++val_ptr;
		++mask_ptr;
	}
}

bool UBitContainer::equalMasked(const BC & value, const BC & mask) const
{
	checkSizes(*this, value, mask);

	const value_type * cur_ptr = mData.data(), * val_ptr = value.mData.data(), *mask_ptr = mask.mData.data();

	bool r = true;
	for(size_t i = mData.size(); i > 0 ; --i)
	{
		if(!((*val_ptr) == ((*cur_ptr) & (*mask_ptr))))
		{
			return false;
		}

		++cur_ptr;
		++val_ptr;
		++mask_ptr;
	}

	return r;
}

size_t UBitContainer::equalCountMasked(const BC & _value, const BC & _mask) const
{
	const value_type * cur_ptr = mData.data(), * val_ptr = _value.mData.data(), *mask_ptr = _mask.mData.data();

	size_t sum(0);
	for(size_t i = mData.size(); i > 0 ; --i)
	{
		value_type cur_value = *cur_ptr, value = *val_ptr, mask = *mask_ptr;

		//T cv_nm = cur_value & mask, v_nm = value & mask;
		//T x = (cv_nm) ^ (v_nm);
		value_type res = mask & (~((cur_value & mask) ^ (value & mask)));

		for(int i = sizeof(value_type) * 8; i > 0; --i)
		{
			sum += res & 1;
			res >>= 1;
		}
		++cur_ptr;
		++val_ptr;
		++mask_ptr;
	}

	return sum;
}

size_t UBitContainer::serialize(void * dest) const
{
	char * cdest = (char*)dest;
	size_t res = 0;
		
	//Byte Count
	res += serialize_int(cdest, mData.size());

	//Byte Data
	memcpy(&cdest[res], mData.data(), byteCount());
	res += byteCount();

	return res;
}

size_t UBitContainer::deserialize(char * dest)
{
	size_t r(0);

	int element_count;
	r += deserialize_int(dest, element_count);
	mData.resize(element_count);

	const size_t data_size = element_count * sizeof(value_type);
	memcpy(mData.data(), dest + r, data_size);
	r += data_size;

	return r;
}

void UBitContainer::serialize(std::ofstream & os) const
{
	serialize_int(os, mBitCount);

	serialize_vector(os, mData);
}

int UBitContainer::deserialize(std::ifstream & is)
{
	mBitCount = deserialize_int(is);
	
	mData = deserialize_vector<vector<value_type>>(is);

	return 0;
}

void UBitContainer::checkSizes(const BC & bc1, const BC & bc2, const BC & bc3)
{
	const size_t s1 = bc1.mData.size(), s2 = bc2.mData.size(), s3 = bc3.mData.size();
	if(!((s1 == s2) && (s2 == s3)))
		throw core_exception("UBitContainer::checkSizes error: Can't process bitsets with different size.");
}


//=====================Masked Vector======================
UMaskedBitVector::UMaskedBitVector(size_t bit_count)
	:state(bit_count), mask(bit_count)
{
}

UMaskedBitVector::~UMaskedBitVector()
{
}

void UMaskedBitVector::clear()
{
	state.clear();
	mask.clear();
}

void UMaskedBitVector::resize(const size_t bit_count)
{
	state.resize(bit_count);
	mask.resize(bit_count);
}

void UMaskedBitVector::set(const size_t bit_index, const bool value)
{
	state[bit_index] = value;
	mask[bit_index] = true;
}

size_t UMaskedBitVector::equalCount(const UBitset & bitset) const
{
	return bitset.equalCountMasked(state, mask);
}

void UMaskedBitVector::print(std::ostream & stream) const
{
	cout << "S:";
	state.print(stream, true);

	cout << "M:";
	mask.print(stream);
}

const size_t UMaskedBitVector::plainDataSize() const
{
	return sizeof(int32_t) + state.byteCount() + sizeof(int32_t) + mask.byteCount();
}

size_t UMaskedBitVector::serialize(void * dest) const
{
	char * cdest = (char*)dest;

	size_t res = 0;

	//State
	res += state.serialize(cdest);

	//Mask
	res += mask.serialize(cdest + res);
	
	return res;
}

void UMaskedBitVector::serialize(std::ofstream & os) const
{
	state.serialize(os);
	mask.serialize(os);
}

int UMaskedBitVector::deserialize(std::ifstream & is)
{
	int res = state.deserialize(is);
	if(res)
		return res;

	res = mask.deserialize(is);
	return res;
}
