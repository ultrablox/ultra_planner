
#ifndef UltraTest_edfd_cover_h
#define UltraTest_edfd_cover_h

#include <core/algorithm/math.h>
#include <core/io/streamer.h>
#include <core/utils/helpers.h>
#include <core/hash.h>
#include <core/compressed_stream.h>
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <random>
#include "core/algorithm/graph.h"

#define STATE_COMPRESSION 1

using namespace std;

struct edfd_element
{
	enum class type_t
	{
		entity,
		process,
		nf_process //special type
	};

	string name;
	type_t type;
};

struct edfd_connection
{
	string name; //name is required for every connection in edfd
};

typedef explicit_graph<edfd_element, edfd_connection> edfd_graph;

struct edfd_cover_state
{
	edfd_graph src_graph;
	//как-то надо хранить данные о покрытых участках графа
};

class edfd_cover
{
public:
	edfd_cover()
	{

	}
protected:
	vector<edfd_graph> sdps; //available set of SDPs to cover source graph
};

#endif