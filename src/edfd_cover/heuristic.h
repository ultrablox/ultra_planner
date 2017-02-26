
#ifndef EDFDCover_heuristic_h
#define EDFDCover_heuristic_h

#include "edfd_cover.h"
#include <core/transition_system.h>
#include <core/algorithm/math.h>
#include <core/io/streamer.h>
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <random>

using namespace std;

//heuristic must be a template
template<typename P>
class simple_heuristic;

//but this class is suitable for edfd_cover only
template<>
class simple_heuristic<transition_system<edfd_cover>>
{
	typedef edfd_cover::state_t state_t;

public:
	simple_heuristic(const edfd_cover & _edfd) //do nothing, really don't need this parameter
	{}

	float operator()(const state_t & state) const
	{
		return 1.0f / state.cover.size();
	}
};

#endif
