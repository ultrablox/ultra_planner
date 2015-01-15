
#ifndef UltraCore_UStateReference_h
#define UltraCore_UStateReference_h

//#include "UStateFactory.h"
#include "../config.h"
#include "../UNodeEstimator.h"
#include <stddef.h>

class UState;
class UStateFactory;

class UStateReference
{
	friend class UState;
public:
	UStateReference(UStateFactory * const factory, StateAddress adress_);
	~UStateReference();
	UStateReference & operator=(const UState & state);
	StateAddress address() const;
	StateAddress parentAddress() const;
	size_t hash() const;

	//Estimation accessor
	void setEstimation(NodeEstimationT val) const;
	NodeEstimationT estimation() const;

	//Expanded accessor
	void setExpanded(bool expanded) const;
private:
	const StateAddress m_address;
	UStateFactory * const m_pFactory;
};

#endif
