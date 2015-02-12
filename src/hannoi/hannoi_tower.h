
#ifndef HannoiTower_h
#define HannoiTower_h

#include <utility>
#include <vector>
#include <iostream>

struct hannoi_state
{
	using number_t = unsigned char;
	using tower_t = std::vector<number_t>;

	std::vector<tower_t> towers;
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

	class state_streamer_t
	{
	public:
		state_streamer_t(const hannoi_tower & h_tower)
			:m_size(h_tower.m_size), m_serializedStateSize((m_size.first + m_size.second) * sizeof(number_t))
		{}

		size_t serialized_size() const
		{
			return m_serializedStateSize;
		}

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
		size_t m_serializedStateSize;
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
		//Read first delimeter
		number_t el;
		is >> el;
		/*for (int i = 0; i < height() * width(); ++i)
		{
		is >> el;
		state.data[i] = el;

		if (el == 0)
		state.empty_pos = i;
		}*/
	}

private:
	size_description_t m_size;
};

#endif
