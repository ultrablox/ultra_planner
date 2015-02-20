
#ifndef UltraCore_UStateFactory_h
#define UltraCore_UStateFactory_h

#include "../config.h"
#include "UStateReference.h"
#include "../UNodeEstimator.h"
#include <core/UBitset.h>
#include <stxxl.h>

/*
Contains database for bitvectors, floatvectors. Can create
new states.
*/

class UStateFactory
{
	friend class UStateReference;

	typedef UBitContainer::value_type flag_compressed_type;

#if USE_HDD_STORAGE
	typedef stxxl::VECTOR_GENERATOR<flag_compressed_type, 8U, 64U>::result flags_vector_type;
	typedef stxxl::VECTOR_GENERATOR<float, 8U, 64U>::result float_vector_type;
	typedef stxxl::VECTOR_GENERATOR<size_t>::result uint_vector_type;
	typedef stxxl::VECTOR_GENERATOR<NodeEstimationT>::result estimation_vector_type;
	typedef stxxl::VECTOR_GENERATOR<bool>::result bool_vector_type;
#else
	typedef std::vector<flag_compressed_type> flags_vector_type;
	typedef std::vector<float> float_vector_type;
	typedef std::vector<size_t> hash_vector_type;
#endif

public:
	UStateFactory(int flag_count = 0, int float_count = 0);
	void setDimensions(int flag_count, int float_count);
	UStateReference createState(StateAddress parent_addr = StateAddress());
	int flagElementCount() const;
	int flagCount() const;
	int floatCount() const;
	flags_vector_type & flagsData();
	float_vector_type & floatsData();
	uint_vector_type & hashData();
	UStateReference operator[](size_t state_index);
	void clear();

	//static UStateFactory * m_pSingleton;

	void serialize(ofstream & fout) const;
	int deserialize(ifstream & fin);

	//Addressing helpers
	enum class Storage {Flags, Floats, Hash, Parent};
	size_t startAddress(size_t index, Storage container) const;
private:
	int m_flagElementCount, m_flagCount, m_floatCount;
	
	flags_vector_type m_flagsData;
	float_vector_type m_floatsData;
	uint_vector_type m_hashData, m_parentData;
	estimation_vector_type m_estimationData;
	bool_vector_type m_expandedData;

	size_t m_stateCounter;
};

#endif
