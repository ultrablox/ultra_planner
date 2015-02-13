
#ifndef HannoiTower_heuristic_h
#define HannoiTower_heuristic_h

/*
Sum of [2 x (the number of disks that are not in place)-1]
over each non-goal peg, and add 2 x (the number of disks
not in place) on the goal peg
*/

template<typename H>
class hannoi_heuristic
{
	typedef H hannoi_problem_t;
	typedef typename hannoi_problem_t::state_t state_t;

public:
	hannoi_heuristic(const hannoi_problem_t & _hannoi)
		:m_hannoi(_hannoi)
	{}

	int operator()(const state_t & state) const
	{
		/*int number_of_disks_not_in_place = 0;
		for(int t = 0; t < m_hannoi.size().second - 1; ++t)
			number_of_disks_not_in_place += state.towers[t].size();*/

		int sum = 0;
		int y = 0;
		auto & last_tower = *state.towers.rbegin();
		for(int expected_block_id = m_hannoi.m_size.first; expected_block_id > 0; --expected_block_id, ++y)
		{
			if(y > last_tower.size())
				break;
			if(last_tower[y] != expected_block_id)
				break;
		}
		//for(int t = 0; t < m_hannoi.size().second - 1; ++t)


		return m_hannoi.m_size.first - sum;
	}
private:
	const hannoi_problem_t & m_hannoi;
};


#endif
