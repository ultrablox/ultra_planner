
#include "USearchNode.h"

std::string to_string(const NodeAddressT & addr)
{
	stringstream ss;
	ss << addr.groupIndex << ":" << addr.localIndex;
	
	string res;
	ss >> res;
	return res;
}

USearchNode::USearchNode()
{}

USearchNode::USearchNode(const UState & _state, const StateAddress parent_address)
	:state(_state), parentAddress(parent_address), m_hasParent(true)
{}

/*
const size_t USearchNode::parentIndex() const
{
	return mParentIndex;
}

const NodeAddressT & USearchNode::parentAddress() const
{
	return m_parentAddress;
}*/

size_t USearchNode::plainDataSize() const
{
	return /*sizeof(mParentIndex) +*/ state.plainDataSize();
}

/*
bool operator==(const USearchNode & n1, const USearchNode & n2)
{
	return (n1.state.flags == n2.state.flags) && (n1.transitions == n2.transitions);
}

bool operator<(const USearchNode & n1, const USearchNode & n2)
{
	return n1.state < n2.state;
}
*/