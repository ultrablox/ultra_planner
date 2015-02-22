
#ifndef UltraTest_sliding_puzzle_h
#define UltraTest_sliding_puzzle_h

#include <core/algorithm/math.h>
#include <core/streamer.h>
#include <core/utils/helpers.h>
#include <core/hash.h>
#include <core/compressed_stream.h>
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <random>

#define STATE_COMPRESSION 1

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
	using size_description_t = std::pair<int, int>;
	typedef unsigned char plate_t;

	puzzle_state(const size_description_t & size = size_description_t())
		:data(size.first * size.second)
	{}

	friend bool operator==(const puzzle_state & lhs, const puzzle_state & rhs)
	{
		return (lhs.empty_pos == rhs.empty_pos) && (lhs.data == rhs.data);
	}

	std::vector<plate_t> data;
	plate_t empty_pos;
};

class sliding_puzzle
{
public:
	typedef puzzle_state::plate_t plate_t;
	typedef puzzle_state state_t;
	typedef int transition_t;
	using size_description_t = state_t::size_description_t;
	
	class state_streamer_t : public streamer_base
	{
/*#if STATE_COMPRESSION
		struct conversion_element
		{
			int byte_offset;
			int bit_offset;
			int mask;
		};
#endif*/
	public:
		state_streamer_t(const sliding_puzzle & _puzzle)
#if STATE_COMPRESSION
			:streamer_base(integer_ceil(_puzzle.m_size.first * _puzzle.m_size.second * bits_for_representing(_puzzle.m_size.first * _puzzle.m_size.second - 1), 8)), /*m_conversionTable(_puzzle.m_size.first * _puzzle.m_size.second),*/ m_bitsPerSlide(bits_for_representing(_puzzle.m_size.first * _puzzle.m_size.second - 1)),
#else
			:streamer_base(_puzzle.m_size.first * _puzzle.m_size.second * sizeof(state_t::plate_t)),
#endif
				m_size(_puzzle.m_size)
		{

#if STATE_COMPRESSION
			m_bitsPerSlide = bits_for_representing(_puzzle.m_size.first * _puzzle.m_size.second - 1);

			//Generate conversion table
/*			const int max_bits = bits_for_representing(_puzzle.m_size.first * _puzzle.m_size.second - 1);
			int cur_bit = 0, cur_byte = 0;
			for (int i = 0; i < _puzzle.m_size.first * _puzzle.m_size.second; ++i, cur_bit += max_bits)
			{
				while (cur_bit >= 8)
				{
					cur_bit -= 8;
					++cur_byte;
				}

				m_conversionTable[i].byte_offset = cur_byte;
				m_conversionTable[i].bit_offset = cur_bit;
				m_conversionTable[i].mask = ((1 << max_bits) - 1) << cur_bit;
			}*/
#endif
		}


		void serialize(void * dst, const state_t & state) const
		{
#if STATE_COMPRESSION
			compressed_stream wstrem(dst);
			wstrem.write(state.data.begin(), state.data.end(), m_bitsPerSlide);

			/*char * dst_ptr = (char*)dst;
			for (int i = 0; i < state.data.size(); ++i)
			{
				auto & el = m_conversionTable[i];
				int * el_ptr = (int*)(dst_ptr + el.byte_offset);
				int val = state.data[i];
				val = val << el.bit_offset;
				*el_ptr = (el.mask & val) | ((~el.mask) & *el_ptr);
			}*/
#else
			memcpy(dst, &state.data[0], serialized_size());
#endif
		}

		void deserialize(const void * src, state_t & state) const
		{
#if STATE_COMPRESSION
			state.data.resize(m_size.first * m_size.second);

			compressed_stream rstrem(src);
			rstrem.read(state.data.begin(), state.data.end(), m_bitsPerSlide);

			/*const char * src_ptr = (char*)src;
			for (int i = 0; i < state.data.size(); ++i)
			{
				auto & el = m_conversionTable[i];
				const int * el_ptr = (const int*)(src_ptr + el.byte_offset);
				
				state.data[i] = ((*el_ptr) & el.mask) >> el.bit_offset;
			}*/
#else
			state.data.resize(serialized_size());
			memcpy(&state.data[0], src, serialized_size());
#endif
			//Obtain empty pos
			auto it = std::find(state.data.begin(), state.data.end(), 0);
			state.empty_pos = std::distance(state.data.begin(), it);
		}

		/*puzzle_state allocated_value() const
		{
			return puzzle_state(m_size);
		}*/

	private:
		size_description_t m_size;
#if STATE_COMPRESSION
//		std::vector<conversion_element> m_conversionTable;
		int m_bitsPerSlide;
#endif
	};

	
	sliding_puzzle(const size_description_t & description)
		:m_size(description)
	{
	}

	static size_description_t deserialize_problem_size(std::istream & is)
	{
		int width, height;
		is >> width >> height;
		return size_description_t(width, height);
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

	state_t solved_state() const
	{
		state_t res(m_size);
		res.empty_pos = 0;
		for (int i = 0; i < m_size.first * m_size.second; ++i)
		{
			//res[i] = pos_t(i % descr.first, i / descr.first);
			res.data[i] = i;
		}
		return std::move(res);
	}

	void serialize_state(std::ostream & os, const state_t & state) const
	{
		os << width() << ' ' << height() << std::endl;
		interpet_state(os, state);
	}

	/*void serialize_state(void * dst, const state_t & state) const
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
	}*/

	void deserialize_state(std::istream & is, state_t & state) const
	{
		state.data.resize(m_size.first * m_size.second);
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

	std::ostream & interpret_transition(std::ostream & os, const state_t & state, const transition_t & transition) const
	{
		os << " move " << (int)state.data[transition];
		return os;
	}

	std::ostream & interpet_state(std::ostream & os, const state_t & state) const
	{
		for (int y = 0; y < height(); ++y)
		{
			os << (int)state.data[y*width()];
			for (int x = 1; x < width(); ++x)
				os << ' ' << (int)state.data[x + width()*y];

			os << std::endl;
		}
		return os;
	}

	const size_description_t & size() const
	{
		return m_size;
	}

	float transition_cost(const state_t&, transition_t) const
	{
		return 1.0f;
	}
protected:
	//state_t & m_state;
	size_description_t m_size;
};

namespace std {
	template<>
	class hash<puzzle_state>
	{
		typedef unsigned char element_t;
	public:
		/*hash()
			:m_hasher(0)
		{
			cache.m_size = 0;
		}*/

		size_t operator()(const puzzle_state & state) const
		{
			const int size = state.data.size();
			if (m_hasher.m_size != size)
				m_hasher = mr_hasher<element_t>(size);

			/*if (cache.m_size != size)
			{
				cache.m_size = size;
				cache.Pi.resize(size);
				cache.PiInv.resize(size);
				cache.cache.resize(size - 1);
			}

			cache.Pi = state.data;
			for (int i = 0; i < size; ++i)
				cache.PiInv[cache.Pi[i]] = i;
			return mr_hash(size, cache.Pi, cache.PiInv, cache.cache);*/

			m_hasher.cache.Pi = state.data;
			return m_hasher();
		}
	private:
		/*mutable struct
		{
			int m_size;
			vector<element_t> Pi, PiInv;
			vector<size_t> cache;
		} cache;*/

		mutable mr_hasher<element_t> m_hasher;
	};
}

#endif
