
#ifndef HannoiTower_h
#define HannoiTower_h

#include <utility>
#include <vector>
#include <iostream>
#include <core/streamer.h>

struct hannoi_state
{
	using number_t = unsigned char;
	using tower_t = std::vector<number_t>;

	std::vector<tower_t> towers;

	friend bool operator==(const hannoi_state & lhs, const hannoi_state & rhs)
	{
		return lhs.towers == rhs.towers;
	}
};

class hannoi_tower
{
public:
	using number_t = hannoi_state::number_t;
	using state_t = hannoi_state;

	//Number of disks + Number of towers
	using size_description_t = std::pair<number_t, number_t>;

	//Source Tower -> Destination Tower
	using transition_t = std::pair<number_t, number_t>;

	class state_streamer_t : public streamer_base
	{
	public:
		state_streamer_t(const hannoi_tower & h_tower)
			:m_size(h_tower.m_size), streamer_base((m_size.first + m_size.second) * sizeof(number_t))
		{}

		void serialize(void * dst, const state_t & state) const
		{
			number_t * sizes_ptr = (number_t*)dst;
			number_t * datas_ptr = sizes_ptr + m_size.first;

			for (number_t i = 0; i < m_size.second; ++i)
			{
				number_t tower_size = state.towers[i].size();
				*sizes_ptr++ = tower_size;

				memcpy(datas_ptr, &state.towers[i][0], tower_size * sizeof(number_t));
				datas_ptr += tower_size;
			}
		}

		void deserialize(const void * src, state_t & state) const
		{
			state.towers.resize(m_size.second);

			number_t * sizes_ptr = (number_t*)src;
			number_t * datas_ptr = sizes_ptr + m_size.first;

			for (number_t i = 0; i < m_size.second; ++i)
			{
				number_t tower_size = *sizes_ptr++;
				state.towers[i].resize(tower_size);
				memcpy(&state.towers[i][0], datas_ptr, tower_size * sizeof(number_t));
				datas_ptr += tower_size;
			}
		}

	private:
		size_description_t m_size;
	};

	hannoi_tower(const size_description_t & description)
		:m_size(description)
	{
	}

	static size_description_t deserialize_problem_size(std::istream & is)
	{
		int width, height;
		is >> width >> height;
		return size_description_t(width, height);
	}

	void deserialize_state(std::istream & is, state_t & state) const
	{
		state.towers.resize(m_size.second);
	
		number_t el;
		for (number_t l = 0; l < m_size.first; ++l)
		{
			for (number_t t = 0; t < m_size.second; ++t)
			{
				is >> el;
				if (el != '*')
					state.towers[t].push_back((int)el);
			}
		}

		for (auto & tower : state.towers)
			std::reverse(tower.begin(), tower.end());
	}

	template<typename F>
	void forall_available_transitions(const state_t & base_state, F fun) const
	{
		for (number_t src_t = 0; src_t < m_size.second; ++src_t)
		{
			for (number_t dst_t = src_t + 1; dst_t < m_size.second; ++dst_t)
			{			
				if (base_state.towers[src_t].empty() && base_state.towers[dst_t].empty())
					continue;

				if ((!base_state.towers[src_t].empty()) && (base_state.towers[dst_t].empty()))
					fun(transition_t(src_t, dst_t));
				else if ((!base_state.towers[dst_t].empty()) && (base_state.towers[src_t].empty()))
					fun(transition_t(dst_t, src_t));
				else if (*base_state.towers[src_t].rbegin() < *base_state.towers[dst_t].rbegin())
					fun(transition_t(src_t, dst_t));
				else
					fun(transition_t(dst_t, src_t));
			}
		}
	}

	void apply(state_t & state, transition_t transition) const
	{
		number_t block_id = *state.towers[transition.first].rbegin();
		state.towers[transition.first].pop_back();
		state.towers[transition.second].push_back(block_id);
	}

private:
	size_description_t m_size;
};

#endif
