#ifndef UltraCore_masked_bit_vector_h
#define UltraCore_masked_bit_vector_h

#include "config.h"
#include "bit_container.h"
#include <initializer_list>
#include <sstream>


struct ULTRA_CORE_API masked_bit_vector
{
	masked_bit_vector(size_t bit_count = 0);
	masked_bit_vector(const bit_vector & _value, const bit_vector & _mask);
	~masked_bit_vector();

	void clear();
	void resize(const size_t bit_count);
	void set(const size_t bit_index, const bool value);
	
	/*
	Returns count of bits in given bitset that are masked
	equal to this vector.
	*/
	//size_t equalCount(const bit_vector & bitset) const;

	//Prints vector in 2 lines
	void print(std::ostream & stream = std::cout) const;

	/*
	1. byte count in each element (int)
	2. plain state representation
	3. plain mask representation
	*/
	/*const size_t plainDataSize() const;

	size_t serialize(void * dest) const;
	
	void serialize(std::ofstream & os) const;
	int deserialize(std::ifstream & is);*/

	template<typename It>
	void remove_indices(It first, It last)
	{
		value.remove_indices(first, last);
		mask.remove_indices(first, last);
	}

	friend bool operator==(const masked_bit_vector & lhs, const masked_bit_vector & rhs);

	//0 - negative, 1 - true
	bit_vector value;

	//0 - doesn't matter, 1 - matter
	bit_vector mask;
};

//typedef UBitContainer bit_vector;

#endif
