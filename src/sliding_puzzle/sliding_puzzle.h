
#ifndef UltraTest_sliding_puzzle_h
#define UltraTest_sliding_puzzle_h

#include <core/algorithm/math.h>
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <random>

using namespace std;

struct pos_t
{
	typedef unsigned char coordinate_t;

	pos_t()
	{}

	pos_t(coordinate_t _x, coordinate_t _y)
		:x(_x), y(_y)
	{}

	bool inside_rect(coordinate_t _x, coordinate_t _y, coordinate_t w, coordinate_t h) const
	{
		return (x >= _x) && (y >= _y) && (x < _x + w) && (y < _y + h);
	}

	friend bool operator==(const pos_t & lhs, const pos_t & rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == rhs.y);
	}

	friend bool operator!=(const pos_t & lhs, const pos_t & rhs)
	{
		return (lhs.x != rhs.x) || (lhs.y != rhs.y);
	}

	coordinate_t x, y;
};

struct puzzle_state
{
	typedef unsigned char plate_t;

	puzzle_state(int element_count = 0)
		:data(element_count)
	{}

	friend bool operator==(const puzzle_state & lhs, const puzzle_state & rhs)
	{
		return (lhs.empty_pos == rhs.empty_pos) && (lhs.data == rhs.data);
	}

	std::vector<plate_t> data;
	plate_t empty_pos;
};



/*
Myrvold & Ruskey Hash Function
http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=D01128EA46AB84979443C3FD5A092A22?doi=10.1.1.43.4521&rep=rep1&type=pdf
*/
template<typename T>
size_t mr_hash(int n, T & Pi, T & PiInv)
{
	if(n == 1)
		return 0;

	int s = Pi[n-1];
	std::swap(Pi[n-1], Pi[PiInv[n-1]]);
	std::swap(PiInv[s], PiInv[n-1]);
	return s + n * mr_hash(n-1, Pi, PiInv);
}

namespace std {
template<>
class hash<puzzle_state>
{
	typedef unsigned char element_t;
public:
    size_t operator()(const puzzle_state & state) const
    {
		const int size = state.data.size();

		vector<element_t> Pi = state.data, PiInv(size);
		for(int i = 0; i < size; ++i)
			PiInv[Pi[i]] = i;
		return mr_hash(size, Pi, PiInv);
		//return mr_hash(size, Pi, PiInv) % 2;
    }
};
}


class sliding_puzzle
{
public:
	typedef puzzle_state::plate_t plate_t;
	typedef puzzle_state state_t;

	typedef int transition_t;
	typedef std::pair<int, int> size_description_t;
	
	sliding_puzzle(size_description_t description)
		:/*m_state(_state), */m_size(description)
	{
	}

	bool is_solved(const state_t & cur_state) const
	{
		for(int i = 0; i < cur_state.data.size(); ++i)
			if(cur_state.data[i] != i)
				return false;
		
		return true;
	}

	transition_t difference(const state_t & lhs, const state_t & rhs)
	{
		return rhs.empty_pos;
	}

	/*
	Must check, that puzzle is solvable, but it does
	not work.
	http://e-maxx.ru/algo/15_puzzle
	*/
	bool has_solution() const
	{
		return true;
		/*std::vector<int> a(width() * height());

		for(int i = 0; i < m_state.size(); ++i)
			a[m_state[i].y * width() + m_state[i].x] = i;

		int inv = 0;
		for (int i=0; i < width()*height(); ++i)
		{
			if (a[i])
			{
				for (int j=0; j<i; ++j)
				{
					if (a[j] > a[i])
						++inv;
				}
			}
		}

		for (int i=0; i < width() * height(); ++i)
		{
			if (a[i] == 0)
				inv += 1 + i / width(); //!!!! ??
		}

		return ((inv % 2) == 0);*/
	}
	
	/*
	Swaps plate with given index and 0-plate.
	*/
	void apply(state_t & state, transition_t plate_index) const
	{
		//if(state.data[state.empty_pos] != 0)
		//	throw runtime_error("Invalid state");

		std::swap(state.data[state.empty_pos], state.data[plate_index]);
		state.empty_pos = plate_index;
	}

	/*friend void print_puzzle(const sliding_puzzle & puzzle)
	{
		cout << "Puzzle " << puzzle.width() << "x" << puzzle.height() << std::endl;
		
		puzzle.serialize_state(cout, puzzle.state());
	}*/

	friend state_t random_init(sliding_puzzle & puzzle, int permutation_count = 10)
	{
		//Random shuffle
		//std::random_shuffle(puzzle.m_state.begin(), puzzle.m_state.end());

		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0, 3);

		
		transition_t last_transition(-1);
		state_t state = puzzle.default_state();

		for(int i = 0; i < permutation_count; ++i)
		{
			vector<transition_t> possible_trans;
			puzzle.forall_available_transitions(state, [&](const transition_t & transition){
				if(last_transition != transition)
					possible_trans.push_back(transition);
			});

			int random_index = distribution(generator);
			transition_t selected_trans = possible_trans[random_index % possible_trans.size()];

			puzzle.apply(state, selected_trans);
		}

		return state;
	}

	state_t default_state() const
	{
		return default_state(m_size);	
	}

	static state_t default_state(const size_description_t & descr)
	{
		state_t res(descr.first * descr.second);
		res.empty_pos = 0;
		for(int i = 0; i < descr.first * descr.second; ++i)
		{
			//res[i] = pos_t(i % descr.first, i / descr.first);
			res.data[i] = i;
		}
		return res;
	}

	void serialize_state(std::ostream & os, const state_t & state) const
	{
		os << width() << ' ' << height() << std::endl;

		for(int y = 0; y < height(); ++y)
		{
			//os << print_data[0 + y*width()];
			os << (int)state.data[y*width()];
			for(int x = 1; x < width(); ++x)
				os << ' ' << (int)state.data[x + width()*y]; //os << ' ' << print_data[x + width()*y];

			os << std::endl;
		}
	}

	void serialize_state(void * dst, const state_t & state) const
	{
		memcpy(dst, &state.data[0], serialized_state_size());
	}

	void deserialize_state(const void * src, state_t & state) const
	{
		state.data.resize(width() * height());
		memcpy(&state.data[0], src, serialized_state_size());
		
		//Obtain empty pos
		auto it = std::find(state.data.begin(), state.data.end(), 0);
		state.empty_pos = std::distance(state.data.begin(), it);
	}

	void deserialize_state(std::istream & is, state_t & state) const
	{
		int el;
		for(int i = 0; i < height() * width(); ++i)
		{
			is >> el;
			state.data[i] = el;

			if(el == 0)
				state.empty_pos = i;
		}
	}

	template<typename F>
	void forall_available_transitions(const state_t & base_state, F fun) const
	{
		static const int deltas[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

		pos_t empty_pos(base_state.empty_pos % width(), base_state.empty_pos / width());

		int max_element = width() * height();
		for(int i = 0; i < 4; ++i)
		{
			pos_t candidate_pos(empty_pos.x + deltas[i][0], empty_pos.y + deltas[i][1]);

			if((candidate_pos.x >= 0) && (candidate_pos.y >= 0) && (candidate_pos.x < width()) && (candidate_pos.y < height()))
			{
				fun(candidate_pos.y * width() + candidate_pos.x);
			}
		}
	}

	/*const state_t & state() const
	{
		return m_state;
	}*/

	int width() const
	{
		return m_size.first;
	}

	int height() const
	{
		return m_size.second;
	}

	int serialized_state_size() const
	{
		return width() * height() * sizeof(state_t::plate_t);
	}

	//returns 10^x = (max state count)
	size_t max_state_count_10() const
	{
		return lg_factor(width() * height());
	}
protected:
	//state_t & m_state;
	size_description_t m_size;
};

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
