
#ifndef SlidingPuzzle_heuristic_h
#define SlidingPuzzle_heuristic_h

#include "rubiks_cube.h"
#include <core/algorithm/math.h>
#include <core/streamer.h>
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <random>

using namespace std;

/*
Manhattan Distance heuristic
http://heuristicswiki.wikispaces.com/3D+Manhattan+distance
*/

template<typename P>
class manhattan_3D_distance_heuristic
{
	typedef P rubik_t;
	typedef typename rubik_t::state_t state_t;
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
public:
	manhattan_3D_distance_heuristic(const rubik_t & _rubik)
		:m_rubik(_rubik), m_destPositions(_rubik.size() * _rubik.size() * _rubik.size() - 6 - 1), m_currentPositions(m_destPositions.size()), m_cornerIds({ 0, 2, 5, 7, 12, 14, 17, 19 }), m_edgeIds({1, 3, 4, 6, 8, 9, 10, 11, 13, 15, 16, 18})
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

	int operator()(const state_t & state) const
	{
		/*for (int x = 0; x < m_rubik.size(); ++x)
		{
			for (int y = 0; y < m_rubik.size(); ++y)
			{
				for (int z = 0; z < m_rubik.size(); ++z)
				{
					element_t cube_id = state.cube(x, y, z);
					if (cube_id != std::numeric_limits<element_t>::max())
						m_currentPositions[cube_id] = point3d(x, y, z);
				}
			}
		}

		//Calculate corners sum
		int corners_sum = 0;
		for (auto corner_id : m_cornerIds)
			corners_sum += corner_distance(m_destPositions[corner_id], m_currentPositions[corner_id]);

		//Calculate edges sum
		int edges_sum = 0;
		for (auto edge_id : m_edgeIds)
			edges_sum += edge_distance(m_destPositions[edge_id], m_currentPositions[edge_id]);

		return max((float)corners_sum / 4, (float)edges_sum / 4);*/

		return 0;
	}
private:
	const rubik_t & m_rubik;
	std::vector<point3d> m_destPositions;
	mutable std::vector<point3d> m_currentPositions;

	const std::array<element_t, 8> m_cornerIds;
	const std::array<element_t, 12> m_edgeIds;
};

#endif
