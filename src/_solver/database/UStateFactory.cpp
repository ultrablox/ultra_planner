
#include "UStateFactory.h"
#include <core/UError.h>
#include <core/transition_system/UState.h>
#include <core/utils/helpers.h>

//UStateFactory * UStateFactory::m_pSingleton;
//==============================UStateFactory=====================
UStateFactory::UStateFactory(int flag_count, int float_count)
	:m_stateCounter(0)
{
	//m_pSingleton = this;

	cout << "Creating state factory...\n";
	cout << "flagsData memory usage is " << fixed << internal_memory_usage(m_flagsData) << " MBs\n";
	cout << "floatsData memory usage is " << fixed << internal_memory_usage(m_floatsData) << " MBs\n";

	setDimensions(flag_count, float_count);
}

void UStateFactory::setDimensions(int flag_count, int float_count)
{
	if(m_stateCounter != 0)
		throw core_exception("Unable to resize not-empty state factory");

	m_flagCount = flag_count;
	m_flagElementCount = ceil(static_cast<double>(flag_count) / (8 * sizeof(flag_compressed_type)));
	m_floatCount = float_count;
}

UStateReference UStateFactory::createState(StateAddress parent_addr)
{
	UStateReference new_ref(this, m_stateCounter++);

	m_flagsData.resize(m_flagsData.size() + m_flagElementCount);
	m_floatsData.resize(m_floatsData.size() + m_floatCount);
	m_hashData.push_back(0);
	m_parentData.push_back(parent_addr);
	m_estimationData.push_back(NodeEstimationT());
	m_expandedData.push_back(false);

	return new_ref;
}

int UStateFactory::flagElementCount() const
{
	return m_flagElementCount;
}

int UStateFactory::flagCount() const
{
	return m_flagCount;
}

int UStateFactory::floatCount() const
{
	return m_floatCount;
}

UStateFactory::flags_vector_type & UStateFactory::flagsData()
{
	return m_flagsData;
}

UStateFactory::float_vector_type & UStateFactory::floatsData()
{
	return m_floatsData;
}

UStateFactory::uint_vector_type & UStateFactory::hashData()
{
	return m_hashData;
}

UStateReference UStateFactory::operator[](size_t state_index)
{
	if(state_index >= m_stateCounter)
	{
		cout << "State " << state_index << " requested, but factory has only " << m_stateCounter << " states.\n";
		throw std::out_of_range("State index out of range");
	}

	return UStateReference(this, state_index);
}

void UStateFactory::clear()
{
	cout << "Node factory contains " << m_flagsData.size() << " flags and " << m_floatsData.size() << " floats.\n";

	m_flagsData.clear();
	m_floatsData.clear();
}

void UStateFactory::serialize(ofstream & fout) const
{
	//=====================Common information=============================
	//Sizeof bit-archive type
	serialize_int(fout, sizeof(flag_compressed_type));
	
	//Flag count
	serialize_int(fout, m_flagCount);
	serialize_int(fout, m_flagElementCount);

	//Float count
	serialize_int(fout, m_floatCount);

	//State count
	serialize_value<size_t>(fout, m_stateCounter);

	cout << "Dumping " << m_stateCounter << " states...\n";

	//===================Data=================================
	serialize_vector(fout, m_flagsData);
	serialize_vector(fout, m_floatsData);
	serialize_vector(fout, m_hashData);
}

int UStateFactory::deserialize(ifstream & fin)
{
	//Sizeof bit-archive type
	int flag_type_size = deserialize_int(fin);
	if(sizeof(flag_compressed_type) != flag_type_size)
	{
		cout << "Different flag container element size.\n";
		return 1;
	}

	//Flag count
	m_flagCount = deserialize_int(fin);
	m_flagElementCount = deserialize_int(fin);

	//Float count
	m_floatCount = deserialize_int(fin);

	//State count
	m_stateCounter = deserialize_value<size_t>(fin);

	//===================Data=================================
	m_flagsData = deserialize_vector<flags_vector_type>(fin);
	m_floatsData = deserialize_vector<float_vector_type>(fin);
	m_hashData = deserialize_vector<uint_vector_type>(fin);

	return 0;
}

size_t UStateFactory::startAddress(size_t index, Storage container) const
{
	switch(container)
	{
	case Storage::Flags:
		return m_flagElementCount * index;
	case Storage::Floats:
		return m_floatCount * index;
	default:
		return index;
	}
}
