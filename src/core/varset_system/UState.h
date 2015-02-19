
#ifndef UltraCore_UState_h
#define UltraCore_UState_h

#include "../config.h"
#include "../bit_container.h"
//#include "UTransition.h"
#include <vector>

class UTransition;
struct CUStateData;
class UStateReference;

class UFloatVector: public std::vector<float>
{
public:
	size_t serialize(char * dest) const;
	size_t deserialize(char * dest);
};

ULTRA_CORE_API std::ostream & operator<<(std::ostream & os, const UFloatVector & vec);

/*
Simple state. Consists of boolean and
numeric atoms.
*/
class ULTRA_CORE_API UState
{
public:
	UState(int predicate_instances_count = 0, int function_instances_count = 0);
	UState(const UState & other);
	//UState(const UState && other);
	UState(const UStateReference & state_ref);
	~UState();
	void clear();
	void setPredicateState(size_t predicate_index, bool value);
	//std::hash<size_t> hash() const;
	
	//Apply standart
	void apply(const UTransition & transition);

	//Apply with no negation
	void applyRelaxed(const UTransition & transition);

	/*
	Creates plane representation of state and returns it.
	*/
	void * toPlainData() const;
	size_t serialize(char * dest) const;
	void deserialize(char * src);
	//Returns size in bytes for full state representation
	size_t plainDataSize() const;

	void print(ostream & os = std::cout) const;
	//template<int ByteCount, int LevelCount>
	size_t hash() const
	{
		return flags.hash();
	}

	friend bool operator==(const UState & s1, const UState & s2);

	UBitset flags;
	UFloatVector functionValues;
};

ULTRA_CORE_API bool operator<(const UState & s1, const UState & s2);

ULTRA_CORE_API std::string to_string(const UState & state);


/*
Restrictions, that is state sub-space. Can be converted
to array of exact states.
*/
class ULTRA_CORE_API UPartialState
{
public:
	UPartialState(int bool_count = 0);
	bool matches(const UState & state) const;

	UMaskedBitVector flags;
};

#endif
