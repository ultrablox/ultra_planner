
#include "masked_bit_vector.h"

//=====================Masked Vector======================
masked_bit_vector::masked_bit_vector(size_t bit_count)
:value(bit_count), mask(bit_count)
{
}

masked_bit_vector::masked_bit_vector(const bit_vector & _value, const bit_vector & _mask)
: value(_value), mask(_mask)
{
}

masked_bit_vector::~masked_bit_vector()
{
}

void masked_bit_vector::clear()
{
	value.clear();
	mask.clear();
}

void masked_bit_vector::resize(const size_t bit_count)
{
	value.resize(bit_count);
	mask.resize(bit_count);
}

void masked_bit_vector::set(const size_t bit_index, const bool val)
{
	value[bit_index] = val;
	mask[bit_index] = true;
}
/*
size_t masked_bit_vector::equalCount(const bit_vector & bitset) const
{
	return bitset.equalCountMasked(value, mask);
}*/

void masked_bit_vector::print(std::ostream & stream) const
{
	cout << "S:";
	value.print(stream, true);

	cout << "M:";
	mask.print(stream);
}
/*
const size_t masked_bit_vector::plainDataSize() const
{
	return sizeof(int32_t)+value.byteCount() + sizeof(int32_t)+mask.byteCount();
}

size_t masked_bit_vector::serialize(void * dest) const
{
	char * cdest = (char*)dest;

	size_t res = 0;

	//State
	res += value.serialize(cdest);

	//Mask
	res += mask.serialize(cdest + res);

	return res;
}

void masked_bit_vector::serialize(std::ofstream & os) const
{
	value.serialize(os);
	mask.serialize(os);
}

int masked_bit_vector::deserialize(std::ifstream & is)
{
	int res = value.deserialize(is);
	if (res)
		return res;

	res = mask.deserialize(is);
	return res;
}*/

bool operator==(const masked_bit_vector & lhs, const masked_bit_vector & rhs)
{
	return (lhs.value == rhs.value) && (lhs.mask == rhs.mask);
}