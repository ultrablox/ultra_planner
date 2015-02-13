
#ifndef SlidingPuzzle_heuristic_h
#define SlidingPuzzle_heuristic_h

#include "sliding_puzzle.h"
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
http://heuristicswiki.wikispaces.com/Manhattan+Distance
*/

template<typename P>
class manhattan_heuristic
{
	typedef P puzzle_t;
	typedef typename puzzle_t::state_t state_t;

public:
	manhattan_heuristic(const puzzle_t & _puzzle)
		:m_puzzle(_puzzle)
	{}

	int operator()(const state_t & state) const
	{
		int sum(0);
		/*for(int i = 1, len = lhs.size(); i < len; ++i)
			sum += abs(lhs[i].x - rhs[i].x) + abs(lhs[i].y - rhs[i].y);*/

		for(int i = 0, len = state.data.size(); i < len; ++i)
		{
			if(state.data[i] != 0)
			{
				pos_t real_pos(state.data[i] % m_puzzle.width(), state.data[i] / m_puzzle.height()),
						correct_pos(i % m_puzzle.width(), i / m_puzzle.height());
				sum += abs(real_pos.x - correct_pos.x) + abs(real_pos.y - correct_pos.y);
				
			}
		}

		//return sum;
		return sum;
	}
private:
	const puzzle_t & m_puzzle;
};

/*
PatternDatabase (Fringe Database) heuristic
http://heuristicswiki.wikispaces.com/pattern+database
*/
template<typename P>
class fringe_db_heuristic
{
	typedef P puzzle_t;
	typedef typename puzzle_t::state_t state_t;
	
public:
	fringe_db_heuristic(const puzzle_t & _puzzle)
		:m_puzzle(_puzzle)
	{}

	int operator()(const state_t & state) const
	{
		//return calculate_distance(lhs, rhs, m_puzzle.width(), m_puzzle.height());

		/*int blue_sum(0), red_sum(0);
		for(int i = 0; i < lhs.size(); ++i)
		{
			int delta = abs((lhs[i].y * m_puzzle.width() + lhs[i].x) - (rhs[i].y * m_puzzle.width() + rhs[i].x));
			if(inside_pattern(rhs[i].x, rhs[i].y))
				red_sum += delta;
			else
				blue_sum += delta;
		}

		return max(red_sum, blue_sum);*/
		return 0;
	}

	bool inside_pattern(int x, int y) const
	{
		return (x == m_puzzle.width() - 1) || (y == m_puzzle.height() - 1) || ((x == 0) && (y == 0));
	}

private:

	int calculate_distance(const state_t & lhs, const state_t & rhs, int W, int H) const
	{
		if((W == 0) || (H == 0))
			return 0;

		std::vector<int> angle_indices(W + H - 1);
		int j(0);
		for(int i = 0; i < H-1; ++i)
			angle_indices[j++] = (W-1) + i*W;

		for(int i = 0; i < W; ++i)
			angle_indices[j++] = (W-1) * H + i;

		int sum(0);
		for(auto ind : angle_indices)
			sum += distance(lhs[ind], rhs[ind]);
		
		return sum + calculate_distance(lhs, rhs, W-1, H-1);
	}

	int distance(const pos_t & lhs, const pos_t & rhs) const
	{
		return abs(lhs.x - rhs.x) + abs(lhs.y - rhs.y);
	}

private:
	const puzzle_t & m_puzzle;
};

#endif
