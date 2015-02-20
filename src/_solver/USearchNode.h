
#ifndef UltraPlanner_USearchNode_h
#define UltraPlanner_USearchNode_h

#include "config.h"
#include "UNodeEstimator.h"
#include <core/transition_system/UTransitionSystem.h>
#include <core/transition_system/UState.h>
#include <list>
#include <forward_list>
#include <tuple>
#include <array>
#include <cassert>

//typedef std::vector<size_t> TransitionPath;

struct SolverResult
{
	SolverResult(const UState & _state)
		:finalState(_state)
	{}

	TransitionPath path;
	UState finalState;
};

template<template<typename T, typename = std::allocator<T>> class transition_container_type>
struct USearchNodeBase
{
	typedef transition_container_type<int> TransitionPathType;

	USearchNodeBase(const TransitionPathType & path = TransitionPathType())
		:transitions(path)
	{

	}

	void addTransition(int transition_index)
	{
		_addTransition(transitions, transition_index);
	}

	TransitionPathType transitions;

private:
	void _addTransition(std::forward_list<int> & container, int transition_index)
	{
		container.push_front(transition_index);
	}

	void _addTransition(std::vector<int> & container, int transition_index)
	{
		container.push_back(transition_index);
	}
};

struct NodeAddressT
{
	typedef NodeEstimationT NodeGroupIndexT; 
	typedef size_t NodeLocalIndexT;

	NodeGroupIndexT groupIndex;
	NodeLocalIndexT localIndex;

	NodeAddressT(const NodeGroupIndexT & groupIndex_ = NodeGroupIndexT(), const NodeLocalIndexT & localIndex_ = std::numeric_limits<NodeLocalIndexT>::max())
		:groupIndex(groupIndex_), localIndex(localIndex_)
	{
	}

	bool isValid() const
	{
		//return !((*this) == invalidAddress());
		return localIndex != std::numeric_limits<NodeLocalIndexT>::max();
	}

	bool operator==(const NodeAddressT & other) const
	{
		const bool v1 = isValid(), v2 = other.isValid();

		if(v1 && v2)
			return (groupIndex == other.groupIndex) && (localIndex == other.localIndex);
		else
			return (v1 == v1);
	}

	static NodeAddressT invalidAddress()
	{
		return NodeAddressT(NodeGroupIndexT(), std::numeric_limits<NodeLocalIndexT>::max());
	}
};

ULTRA_SOLVER_API std::string to_string(const NodeAddressT & addr);

struct ULTRA_SOLVER_API USearchNode// : public USearchNodeBase<std::vector>
{
	//typedef USearchNodeBase<std::vector> _Base;
	USearchNode();
	//explicit USearchNode(const UState & _state, const size_t parent_index = -1); //, const TransitionPath & _transitions = TransitionPath()
	//explicit USearchNode(const UState & _state, const NodeAddressT parent_address);
	explicit USearchNode(const UState & _state, const StateAddress parent_address);

	UState state;
	StateAddress parentAddress;
	//const size_t parentIndex() const;
	//const NodeAddressT & parentAddress() const;
	size_t plainDataSize() const;
private:
	//const USearchNode * m_pParent;
	//size_t mParentIndex;
	bool m_hasParent;
	//NodeAddressT m_parentAddress;

	
};
/*
inline size_t _Hash_seq(const unsigned char *_First, size_t _Count)
{
 #ifdef _M_X64
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	const size_t _FNV_offset_basis = 14695981039346656037ULL;
	const size_t _FNV_prime = 1099511628211ULL;

 #else
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
	const size_t _FNV_offset_basis = 2166136261U;
	const size_t _FNV_prime = 16777619U;
 #endif

	size_t _Val = _FNV_offset_basis;
	for (size_t _Next = 0; _Next < _Count; ++_Next)
		{	// fold in another byte
		_Val ^= (size_t)_First[_Next];
		_Val *= _FNV_prime;
		}

 #ifdef _M_X64
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	_Val ^= _Val >> 32;

 #else
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
 #endif 

	return (_Val);
	}*/

template<size_t LevelCount, int ItemByteCount = 4>
struct SearchNodeMultilevelHash
{
	typedef size_t HashResult;
	std::array<HashResult, LevelCount> operator()(const USearchNode * snode) const
	{
		static_assert(ItemByteCount <= 8, "Item size is too big.");
		static_assert(LevelCount >= 1, "Levels count must be a positive integer.");

		const UBitset * flags_bitset = &snode->state.flags;

		std::array<HashResult, LevelCount> arr;

		const size_t bytes_per_level = flags_bitset->byteCount() / LevelCount;
		
		const unsigned char * cdata = (const unsigned char *)flags_bitset->mData.data();

		/*for(int i = 0; i < LevelCount; ++i)
			arr[i] = _Hash_seq(cdata + i * bytes_per_level, bytes_per_level);
		

		for(int i = 0; i < LevelCount; ++i)
			arr[i] = calcHash<ItemByteCount, LevelCount*ItemByteCount>((const char *)flags_bitset->mData.data(), flags_bitset->byteCount());
			*/

		//return snode->state.hash<2, LevelCount>();
		
		return arr;
	}

	template<int Size, int Stride>
	HashResult calcHash(const char * data_start, int byte_count) const
	{
		HashResult res;

		//hash()
		return 0;
	}

};

struct StateHash
{
	size_t operator()(const UState & state) const
	{
		return state.hash();
	}
};

struct SearchNodeHash
{
	size_t operator()(const USearchNode * snode) const
	{
		return snode->state.hash();
	}
};

struct SearchNodeEqual
{
	bool operator()(const USearchNode * n1, const USearchNode * n2) const
	{
		return n1->state.flags == n2->state.flags;
	}
};


#endif
