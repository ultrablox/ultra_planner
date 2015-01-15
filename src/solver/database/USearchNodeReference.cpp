
#include "USearchNodeReference.h"
#include "USearchNodeFactory.h"

USearchNodeReference::USearchNodeReference(size_t state_index_, NodeAddressT node_index_ , NodeAddressT parent_address_, bool expanded_)
	: m_stateIndex(state_index_), m_address(node_index_), m_parentAddress(parent_address_), m_expanded(expanded_)
{
}

USearchNodeReference::~USearchNodeReference()
{
}

size_t USearchNodeReference::stateIndex() const
{
	return m_stateIndex;
}

NodeAddressT USearchNodeReference::address() const
{
	return m_address;
}

const NodeAddressT & USearchNodeReference::parentAddress() const
{
	return m_parentAddress;
}
/*
USearchNodeReference USearchNodeReference::parent() const
{
	return USearchNodeFactory::m_pSingleton->nodeRef(parentAddress());
}
*/
bool USearchNodeReference::hasParent() const
{
	return m_parentAddress.isValid();
}

void USearchNodeReference::setExpanded()
{
	auto & container_ref = USearchNodeFactory::m_pSingleton->m_data_[m_address.groupIndex];
	container_ref[m_address.localIndex].expanded = true;

	m_expanded = true;
}

bool USearchNodeReference::expanded() const
{
	return m_expanded;
}

USearchNodeReference & USearchNodeReference::operator=(const USearchNodeReference & other)
{
	m_stateIndex = other.m_stateIndex;
	m_address = other.m_address;
	m_parentAddress = other.m_parentAddress;
	m_expanded = other.m_expanded;
	return *this;
}

bool USearchNodeReference::operator==(const USearchNodeReference & other) const
{
	return (m_stateIndex == other.m_stateIndex) && (m_address == other.m_address) && (m_parentAddress == other.m_parentAddress);
}

bool USearchNodeReference::operator!=(const USearchNodeReference & other) const
{
	return !(operator==(other));
}
/*
USearchNodeReference & operator++(USearchNodeReference & old)
{
	auto new_ref = USearchNodeFactory::m_pSingleton->nodeRef(NodeAddressT(old.m_address.groupIndex, old.m_address.localIndex + 1ULL));
	std::swap(old, new_ref);
	return old;
}


size_t SearchNodeDeferredHash::operator()(const size_t node_index) const
{
	return operator()(USearchNodeFactory::m_pSingleton->node(node_index).state);
}

size_t SearchNodeDeferredHash::operator()(const UState & state) const
{
	return state.hash<4, 1>();
}

USearchNodeReference USearchNodeFactory::createNode(UStateReference & state_ref, USearchNodeReference & parent_node_ref)
{
	USearchNodeData new_data;
	//new_data.
	//m_data.push_back(

	USearchNodeReference new_ref(*this, 0, 0);
	return new_ref;
}
*/
/*
//===================Interval============================
NodeRefIntervalType::NodeRefIntervalType(const USearchNodeReference & left, const USearchNodeReference & right)
	:_Base(left, right), m_empty(false)
{
	if(left.address().groupIndex != right.address().groupIndex)
		throw core_exception("Invalid node interval - not same group index.");
}

NodeRefIntervalType::NodeRefIntervalType()
	:_Base(USearchNodeReference(0, NodeAddressT(), NodeAddressT::invalidAddress(), false), USearchNodeReference(0, NodeAddressT(), NodeAddressT::invalidAddress(), false)), m_empty(true)
{
}

size_t NodeRefIntervalType::count() const
{
	return second.address().localIndex - first.address().localIndex + 1;
}

bool NodeRefIntervalType::empty() const
{
	return m_empty;
}

USearchNodeReference NodeRefIntervalType::iterator::operator*() const
{
	return USearchNodeFactory::m_pSingleton->nodeRef(NodeAddressT(interval.first.address().groupIndex, m_localIndex));
}

NodeRefIntervalType::iterator NodeRefIntervalType::begin() const
{
	return iterator(*this, first.address().localIndex);
}

NodeRefIntervalType::iterator NodeRefIntervalType::end() const
{
	return iterator(*this, second.address().localIndex + 1);
}

*/

UNodeEstimatedRef::UNodeEstimatedRef(StateAddress addr, NodeEstimationT est)
	:stateAddress(addr), estimation(est)
{}

UNodeReference::UNodeReference(StateAddress addr, size_t hash_val)
	:stateAddress(addr), hash(hash_val)
{}
