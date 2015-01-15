
#include "UStateReference.h"
#include "UStateFactory.h"
#include <core/transition_system/UState.h>

UStateReference::UStateReference(UStateFactory * const factory, StateAddress adress_)
	:m_address(adress_), m_pFactory(factory)
{
}

UStateReference::~UStateReference()
{

}

UStateReference & UStateReference::operator=(const UState & state)
{
	//Copy flags
	const size_t flag_data_begin = m_pFactory->startAddress(m_address, UStateFactory::Storage::Flags);

	for(int i = 0; i < m_pFactory->flagElementCount(); ++i)
		m_pFactory->flagsData()[flag_data_begin + i] = state.flags.mData[i];

	//Copy floats
	const size_t floats_data_begin = m_pFactory->startAddress(m_address, UStateFactory::Storage::Floats);
	for(int i = 0; i < m_pFactory->floatCount(); ++i)
		m_pFactory->floatsData()[floats_data_begin + i] = state.functionValues[i];

	m_pFactory->hashData()[m_address] = state.hash();

	return *this;
}
/*
UStateFactory & UStateReference::factory() const
{
	return m_factory;
}
*/
StateAddress UStateReference::address() const
{
	return m_address;
}

StateAddress UStateReference::parentAddress() const
{
	return m_pFactory->m_parentData[m_address];
}

size_t UStateReference::hash() const
{
	return m_pFactory->hashData()[m_address];
}


void UStateReference::setEstimation(NodeEstimationT val) const
{
	m_pFactory->m_estimationData[m_address] = val;
}

NodeEstimationT UStateReference::estimation() const
{
	return m_pFactory->m_estimationData[m_address];
}

void UStateReference::setExpanded(bool expanded) const
{
	m_pFactory->m_expandedData[m_address] = expanded;
}

//==============!!!!!!!!!!===============
UState::UState(const UStateReference & state_ref)
{
	//Copy flags to current bitset
	const size_t flag_data_begin = state_ref.m_pFactory->startAddress(state_ref.address(), UStateFactory::Storage::Flags);

	flags.mBitCount = state_ref.m_pFactory->flagCount();
	flags.resizeContainer(state_ref.m_pFactory->flagElementCount());
	for(int i = 0; i < state_ref.m_pFactory->flagElementCount(); ++i)
		flags.mData[i] = state_ref.m_pFactory->flagsData()[flag_data_begin + i];

	//Copy floats
	const size_t floats_data_begin = state_ref.m_pFactory->startAddress(state_ref.address(), UStateFactory::Storage::Floats);
	functionValues.resize(state_ref.m_pFactory->floatCount());
	for(int i = 0; i < state_ref.m_pFactory->floatCount(); ++i)
		functionValues[i] = state_ref.m_pFactory->floatsData()[floats_data_begin + i];
}
