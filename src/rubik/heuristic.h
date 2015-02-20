
#ifndef SlidingPuzzle_heuristic_h
#define SlidingPuzzle_heuristic_h

#include "rubiks_cube.h"
#include <core/algorithm/math.h>
#include <core/algorithm/graph.h>
#include <core/streamer.h>
#include <core/search_algorithms/bfs_engine.h>
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <random>
#include <unordered_map>

using namespace std;

/*
Manhattan Distance heuristic
http://heuristicswiki.wikispaces.com/3D+Manhattan+distance
*/

template<typename P>
class manhattan_3D_distance_heuristic
{
	typedef P rubik_t;
	using state_t = typename rubik_t::state_t;
	using element_t = typename state_t::element_t;

	struct point3d
	{
		point3d(int _x = 0, int _y = 0, int _z = 0)
		:x(_x), y(_y), z(_z)
		{}

		/*friend int operator-(const point3d & p1, const point3d & p2)
		{
		//return abs(p1.x - p2.x) + abs(p1.y - p2.y) + abs(p1.z - p2.z);

		}*/

		friend int corner_distance(const point3d & p1, const point3d & p2)
		{
			int res = 0;
			if (p1.x != p2.x)
				++res;
			if (p1.y != p2.y)
				++res;
			if (p1.z != p2.z)
				++res;
			return res;
		}

		friend int edge_distance(const point3d & p1, const point3d & p2)
		{
			return 0;
		}

		int x, y, z;
	};

	struct state_mask_t
	{
		state_mask_t()
		:corners(8, false), edges(12, false)
		{}

		std::vector<bool> corners, edges;
	};

	struct state_mask_hasher
	{
		size_t operator()(const state_mask_t & mask) const
		{
			size_t res = 0;
			for (auto c : mask.corners)
				res = (res << 1) | (size_t)c;

			for (auto c : mask.edges)
				res = (res << 1) | (size_t)c;

			return res;
		}
	};

	struct cubie_descr_t
	{
		friend bool operator==(const cubie_descr_t & lhs, const cubie_descr_t & rhs)
		{
			return (lhs.type == rhs.type) && (lhs.cubie_id == rhs.cubie_id) && (lhs.pos == rhs.pos) && (lhs.orientation == rhs.orientation);
		}

		int type; //0-corner, 1 - edge
		int cubie_id, pos, orientation;
	};

	struct cubie_descr_hasher
	{
		size_t operator()(const cubie_descr_t & descr) const
		{
			size_t res = descr.type;
			res = (res << 2) | descr.orientation;
			res = (res << 5) | descr.cubie_id;
			res = (res << 5) | descr.pos;
			return res;
		}
	};

public:
	manhattan_3D_distance_heuristic(const rubik_t & _rubik)
		:m_rubik(_rubik)/*, m_destPositions(_rubik.size() * _rubik.size() * _rubik.size() - 6 - 1), m_currentPositions(m_destPositions.size()), m_cornerIds({ 0, 2, 5, 7, 12, 14, 17, 19 }), m_edgeIds({1, 3, 4, 6, 8, 9, 10, 11, 13, 15, 16, 18})*/
	{
		/*state_t solved_state = _rubik.solved_state();

		for (int x = 0; x < _rubik.size(); ++x)
		{
			for (int y = 0; y < _rubik.size(); ++y)
			{
				for (int z = 0; z < _rubik.size(); ++z)
				{
					element_t cube_id = solved_state.cube(x, y, z);
					if (cube_id != std::numeric_limits<element_t>::max())
						m_destPositions[cube_id] = point3d(x, y, z);
				}
			}
		}*/
	}

	manhattan_3D_distance_heuristic(const manhattan_3D_distance_heuristic & rhs)
		:m_rubik(rhs.m_rubik)
	{}

	int operator()(const state_t & state) const
	{
		//Calculate corners sum
		float corners_sum = 0;

		for (int i = 0; i < 8; ++i)
		{
			cubie_descr_t descr;
			descr.type = 0;
			descr.cubie_id = state.m_cornerCubies[i];
			descr.pos = i;
			descr.orientation = state.m_cornerMarkers[descr.cubie_id];

			corners_sum += cubie_estimation(state, descr);
		}

		//Calculate edges sum
		float edges_sum = 0;
		for (int i = 0; i < state.m_edgeCubies.size(); ++i)
		{
			cubie_descr_t descr;
			descr.type = 1;
			descr.cubie_id = state.m_edgeCubies[i];
			descr.pos = i;
			descr.orientation = state.m_edgeMarkers[descr.cubie_id];

			edges_sum += cubie_estimation(state, descr);
		}

		return max((float)corners_sum / 4, (float)edges_sum / 4);
	}

	float cubie_estimation(const state_t & est_state, const cubie_descr_t & descr) const
	{
		auto it = m_cubieEstimations.find(descr);
		if (it != m_cubieEstimations.end())
		{
			//cout << "Pattern found!" << std::endl;
			return it->second;
		}
		else
		{
			using graph_t = transition_system_graph<rubik_t>;
			typename rubik_t::state_streamer_t streamer(m_rubik);
			bfs_engine<graph_t, false, hashset_t::Internal> eng(streamer);

			graph_t graph(m_rubik);

			std::vector<state_t> path;
			bool found = eng(graph, est_state, [&](const state_t & state){

				if (descr.type == 0)
					return (state.m_cornerCubies[descr.cubie_id] == descr.cubie_id) && (state.m_cornerMarkers[descr.cubie_id] == 0);
				else
					return (state.m_edgeCubies[descr.cubie_id] == descr.cubie_id) && (state.m_edgeMarkers[descr.cubie_id] == 0);
			}, path);


			float res = path.size() - 1;			
			m_cubieEstimations.insert(make_pair(descr, res));

			//cout << "Added pattern, total " << m_cubieEstimations.size() << std::endl;
			return res;
		}
	}

	/*float masked_estimation(const state_t & state, const state_mask_t & mask)
	{
		using graph_t = transition_system_graph<rubik_t>;
		rubik_t::state_streamer_t streamer(m_rubik);
		bfs_engine<graph_t, false, hashset_t::Internal> eng(streamer);

		graph_t graph(m_rubik);

		std::vector<rubik_t::state_t> path;
		bool found = eng(graph, state, [&](const rubik_t::state_t & state){


			for (int c = 0; c < 8; ++c)
			{
				if (mask.corners[c])
				{
					if ((state.m_cornerCubies[c] != c) || (state.m_cornerMarkers[c] != 0))
						return false;
				}
			}

			for (int e = 0; e < 12; ++e)
			{
				if (mask.edges[e])
				{
					if ((state.m_edgeCubies[e] != e) || (state.m_edgeMarkers[e] != 0))
						return false;
				}
			}

			return true;
		}, path);

		return path.size() - 1;
	}*/
private:
	const rubik_t & m_rubik;
	/*std::vector<point3d> m_destPositions;
	mutable std::vector<point3d> m_currentPositions;

	const std::array<element_t, 8> m_cornerIds;
	const std::array<element_t, 12> m_edgeIds;

	std::unordered_map<state_mask_t, float, state_mask_t> m_estimations;*/

	mutable std::unordered_map<cubie_descr_t, float, cubie_descr_hasher> m_cubieEstimations;
};

#endif
