
#ifndef HannoiTower_h
#define HannoiTower_h

#include <core/streamer.h>
#include <core/hash.h>
#include <utility>
#include <vector>
#include <iostream>
#include <string>

struct hannoi_state
{
	using number_t = unsigned char;
	using tower_t = std::vector<number_t>;

	//Number of disks + Number of towers
	struct size_description_t
	{
		size_description_t(number_t disk_count, number_t peg_count)
			:diskCount(disk_count), pegCount(peg_count)
		{}

		number_t diskCount, pegCount;
	};

	hannoi_state()
	{
	}

	hannoi_state(const size_description_t & size)
		:towers(size.pegCount, tower_t(size.diskCount))
	{
		for (auto & tower : towers)
			tower.resize(0);
	}

	friend bool operator==(const hannoi_state & lhs, const hannoi_state & rhs)
	{
		return lhs.towers == rhs.towers;
	}

	std::vector<tower_t> towers;
};

namespace std {
template<>
class hash<hannoi_state>
{
	using element_t = hannoi_state::number_t;
public:
	hash()
	{
		cache.m_size = 0;
	}

	size_t operator()(const hannoi_state & state) const
	{
		cache.Pi.resize(0);

		for(auto & tower : state.towers)
		{
			cache.Pi.insert(cache.Pi.end(), tower.begin(), tower.end());
		}


		const int size = cache.Pi.size();
		for (int i = 0; i < size; ++i)
			--cache.Pi[i];

		if (cache.m_size != size)
		{
			cache.m_size = size;
			cache.Pi.resize(size);
			cache.PiInv.resize(size);
			cache.cache.resize(size-1);
		}

		for (int i = 0; i < size; ++i)
			cache.PiInv[cache.Pi[i]] = i;
		
		return mr_hash(size, cache.Pi, cache.PiInv, cache.cache);
	}
private:
	mutable struct
	{
		int m_size;
		vector<element_t> Pi, PiInv;/**/
		vector<size_t> cache;
	} cache;
};
}



class hannoi_tower
{
public:
	using number_t = hannoi_state::number_t;
	using state_t = hannoi_state;
	using size_description_t = hannoi_state::size_description_t;
	

	//Source Tower -> Destination Tower
	using transition_t = std::pair<number_t, number_t>;

	class state_streamer_t : public streamer_base
	{
	public:
		state_streamer_t(const hannoi_tower & h_tower)
			:m_size(h_tower.m_size), streamer_base((h_tower.m_size.diskCount + h_tower.m_size.pegCount) * sizeof(number_t))
		{}

		void serialize(void * dst, const state_t & state) const
		{
			number_t * sizes_ptr = (number_t*)dst;
			number_t * datas_ptr = sizes_ptr + m_size.pegCount;

			for (number_t i = 0; i < m_size.pegCount; ++i)
			{
				number_t tower_size = state.towers[i].size();
				*sizes_ptr++ = tower_size;

				if (!state.towers[i].empty())
				{
					memcpy(datas_ptr, &state.towers[i][0], tower_size * sizeof(number_t));
					datas_ptr += tower_size;
				}
			}
		}

		void deserialize(const void * src, state_t & state) const
		{
			state.towers.resize(m_size.pegCount);

			number_t * sizes_ptr = (number_t*)src;
			number_t * datas_ptr = sizes_ptr + m_size.pegCount;

			for (number_t i = 0; i < m_size.pegCount; ++i)
			{
				number_t tower_size = *sizes_ptr++;
				state.towers[i].resize(tower_size);
				if (tower_size > 0)
				{
					memcpy(&state.towers[i][0], datas_ptr, tower_size * sizeof(number_t));
					datas_ptr += tower_size;
				}
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
		int disc_count, peg_count;
		is >> disc_count >> peg_count;
		return size_description_t(disc_count, peg_count);
	}

	void deserialize_state(std::istream & is, state_t & state) const
	{
		state.towers.resize(m_size.pegCount);
	
		std::string el;
		for (number_t l = 0; l < m_size.diskCount; ++l)
		{
			for (number_t t = 0; t < m_size.pegCount; ++t)
			{
				is >> el;
				if (el != "*")
				{
					state.towers[t].push_back(stoi(el));
				}
			}
		}

		for (auto & tower : state.towers)
			std::reverse(tower.begin(), tower.end());
	}

	template<typename F>
	void forall_available_transitions(const state_t & base_state, F fun) const
	{
		for (number_t src_t = 0; src_t < m_size.pegCount; ++src_t)
		{
			for (number_t dst_t = src_t + 1; dst_t < m_size.pegCount; ++dst_t)
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

	state_t default_state() const
	{
		state_t res(m_size);
		for (number_t d = m_size.diskCount; d > 0; --d)
			res.towers[m_size.pegCount - 1].push_back(d);
		return std::move(res);
	}

	bool is_solved(const state_t & cur_state) const
	{
		return cur_state.towers[m_size.pegCount - 1].size() == m_size.diskCount;
	}

	/*transition_t difference(const state_t & lhs, const state_t & rhs)
	{
		transition_t res;
		for (number_t i = 0; i < m_size.pegCount; ++i)
		{
			int delta = lhs.towers[i].size() - rhs.towers[i].size();
			if(delta < 0)
				res.second = i;
			else if(delta > 0)
				res.first = i;
		}

		return std::move(res);
	}*/

	std::ostream & interpret_transition(std::ostream & os, const state_t & state, const transition_t & transition) const
	{
		os << " Move block " << (int)(*state.towers[transition.first].rbegin()) << " " << (int)transition.first << "->" << (int)transition.second;
		return os;
	}

	std::ostream & interpet_state(std::ostream & os, const state_t & state) const
	{
		for (int l = m_size.diskCount - 1; l >= 0; --l)
		{
			for (int t = 0; t < m_size.pegCount; ++t)
			{
				if(l >= state.towers[t].size())
					os << '*';
				else
					os << (int)state.towers[t][l];
				os << ' ';
			}
			os << std::endl;
		}

		return os;
	}

	const size_description_t & size() const
	{
		return m_size;
	}

	float transition_cost(const state_t&, const transition_t&) const
	{
		return 1.0f;
	}

	/*float transition_cost(transition_t) const
	{
		return 1.0f;
	}*/
private:
	size_description_t m_size;
};

#endif
