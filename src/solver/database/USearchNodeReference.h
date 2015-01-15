
#ifndef UltraSolver_USearchNodeReference_h
#define UltraSolver_USearchNodeReference_h

#include "../config.h"
#include "../UNodeEstimator.h"
#include "../USearchNode.h"
#include <utility>
#include <functional>

class USearchNodeReference
{
	friend class USearchNodeFactory;

public:
	USearchNodeReference(size_t state_index_ = std::numeric_limits<size_t>::max(), NodeAddressT node_index_ = NodeAddressT::invalidAddress(), NodeAddressT parent_address_ = NodeAddressT::invalidAddress(), bool expanded_ = false);
	~USearchNodeReference();
	size_t stateIndex() const;
	NodeAddressT address() const;
	const NodeAddressT & parentAddress() const;
	//USearchNodeReference parent() const;
	bool hasParent() const;
	void setExpanded();
	bool expanded() const;
	USearchNodeReference & operator=(const USearchNodeReference & other);
	bool operator==(const USearchNodeReference & other) const;
	bool operator!=(const USearchNodeReference & other) const;
	//friend USearchNodeReference & operator++(USearchNodeReference & old);
private:
	//USearchNodeFactory & m_factory;

	size_t m_stateIndex; //Index of the state in state factory
		
	NodeAddressT	m_address,			//Index in the node database
					m_parentAddress;	//Index of the parent. If the parent index is similar to its own index - item has no parent.
	bool m_expanded;
};
/*
class NodeRefIntervalType : public std::pair<USearchNodeReference, USearchNodeReference>
{
	typedef std::pair<USearchNodeReference, USearchNodeReference> _Base;
public:

	class iterator
	{
		friend class NodeRefIntervalType;
	private:
		iterator(const NodeRefIntervalType & interval_, size_t local_index)
			:interval(interval_), m_localIndex(local_index)
		{
		}

		const NodeRefIntervalType & interval;
		size_t m_localIndex;

	public:
		bool operator==(const iterator & other) const
		{
			return other.m_localIndex == m_localIndex;
		}

		iterator & operator++()
		{
			++m_localIndex;
			return *this;
		}

		friend bool operator!=(const iterator & i1, const iterator & i2)
		{
			return i1.m_localIndex != i2.m_localIndex;
		}

		USearchNodeReference operator*() const;
	};

	NodeRefIntervalType(const USearchNodeReference & left, const USearchNodeReference & right);
	NodeRefIntervalType();
	size_t count() const;
	bool empty() const;
	iterator begin() const;
	iterator end() const;

private:
	bool m_empty;
};
*/


/*
Used for processing in search queue.
*/
struct UNodeEstimatedRef
{
	UNodeEstimatedRef(StateAddress addr = 0, NodeEstimationT est = NodeEstimationT());

	NodeEstimationT estimation;
	StateAddress stateAddress;
};

/*
Used for storing in DB.
*/
struct UNodeReference
{
	UNodeReference(StateAddress addr = 0, size_t hash_val = 0);

	size_t hash;
	StateAddress stateAddress;
};

struct NodeRefHasher
{
	size_t operator()(const UNodeReference & node_ref)
	{
		return node_ref.hash;
	}
};

#endif
