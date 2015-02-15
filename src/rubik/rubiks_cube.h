
#ifndef UltraTest_rubiks_cube_h
#define UltraTest_rubiks_cube_h

#include <vector>
#include <array>
#include <core/utils/helpers.h>
#include <core/streamer.h>
#include <core/hash.h>
#include <map>

/*
0 - L (left)
1 - F (front)
2 - R (right)
3 - B (back)
4 - U (up)
5 - D (down)

U
|
L -> F <- R
|
D
*/

enum class face_id { face_L = 0, face_F, face_R, face_B, face_U, face_D, face_count = 6 };

struct face_t
{
	using element_t = unsigned char;

	face_t(int size = 0)
		:data(size * size, std::numeric_limits<element_t>::max())
	{}

	template<typename TableT>
	void transform(bool inversed, const TableT & transform_table)
	{
		std::vector<element_t> transformed_data = data;

		if (inversed)
		{
			for (auto & tr : transform_table)
				transformed_data[tr.first] = data[tr.second];
		}
		else
		{
			for (auto & tr : transform_table)
				transformed_data[tr.second] = data[tr.first];
		}

		data = std::move(transformed_data);
	}

	friend bool operator==(const face_t & lhs, const face_t & rhs)
	{
		return lhs.data == rhs.data;
	}

	std::vector<element_t> data;
};

struct rubik_state
{
	using element_t = unsigned char;

	rubik_state(int size = 0)
		:m_size(size), m_cubes(size * size * size, std::numeric_limits<element_t>::max())
	{
		faces.fill(face_t(size));
	}

	face_t & face(face_id id)
	{
		return faces[static_cast<int>(id)];
	}

	const face_t & face(face_id id) const
	{
		return faces[static_cast<int>(id)];
	}

	element_t & element(face_id f_id, int x, int y)
	{
		return face(f_id).data[y * m_size + x];
	}

	const element_t & element(face_id f_id, int x, int y) const
	{
		return face(f_id).data[y * m_size + x];
	}

	element_t & cube(int x, int y, int z)
	{
		return m_cubes[z * m_size * m_size + y * m_size + x];
	}

	const element_t & cube(int x, int y, int z) const
	{
		return m_cubes[z * m_size * m_size + y * m_size + x];
	}

	friend bool operator==(const rubik_state & lhs, const rubik_state & rhs)
	{
		//return lhs.faces == rhs.faces;
		return lhs.m_cubes == rhs.m_cubes;
	}

	std::array<face_t, 6> faces;

	std::vector<element_t> m_cubes;

	int m_size;
};

struct rubik_state_snapshot
{
	using element_t = unsigned char;

	rubik_state_snapshot(int size = 0)
		:m_size(size)
	{
		faces.fill(face_t(size));
	}

	element_t & element(face_id f_id, int x, int y)
	{
		return face(f_id).data[y * m_size + x];
	}

	face_t & face(face_id id)
	{
		return faces[static_cast<int>(id)];
	}

	const face_t & face(face_id id) const
	{
		return faces[static_cast<int>(id)];
	}

	std::array<face_t, 6> faces;
	int m_size;
};

class rubiks_cube
{
public:
	typedef rubik_state state_t;
	typedef int size_description_t;
	using element_t = rubik_state::element_t;

	enum class transition_t { F = 0, B, U, D, L, R, F_, B_, U_, D_, L_, R_, count };

	class state_streamer_t : public streamer_base
	{
	public:
		state_streamer_t(const rubiks_cube & _rubik)
			:streamer_base(_rubik.m_size * _rubik.m_size * 6 * sizeof(element_t)), m_size(_rubik.m_size)
		{}

		void serialize(void * dst, const state_t & state) const
		{
			const int step_size = m_size * m_size * sizeof(element_t);

			char * dest_ptr = (char*)dst;
			for (int i = 0; i < 6; ++i, dest_ptr += step_size)
				memcpy(dest_ptr, state.faces[i].data.data(), step_size);
		}

		void deserialize(const void * src, state_t & state) const
		{
			const int step_size = m_size * m_size * sizeof(element_t);

			char * src_ptr = (char*)src;

			for (int i = 0; i < 6; ++i, src_ptr += step_size)
			{
				state.faces[i].data.resize(m_size * m_size);
				memcpy(state.faces[i].data.data(), src_ptr, step_size);
			}
		}

	private:
		size_description_t m_size;
	};


	rubiks_cube(int problem_size)
		:m_size(problem_size)
	{
		init_rotation_table();
	}

	static size_description_t deserialize_problem_size(std::istream & is)
	{
		int sz;
		is >> sz;
		return size_description_t(sz);
	}

	template<typename F>
	void forall_available_transitions(const state_t & base_state, F fun) const
	{
		for (int i = 0; i < static_cast<int>(transition_t::count); ++i)
			fun(static_cast<transition_t>(i));
	}

	void deserialize_state(std::istream & is, state_t & state) const
	{
		rubik_state_snapshot state_snapshot(m_size);

		int el;
		for (int i = 0; i < 6; ++i)
		{
			for (int j = 0; j < m_size * m_size; ++j)
			{
				is >> el;
				//state.faces[i].data[j] = (element_t)el;
				state_snapshot.faces[i].data[j] = (element_t)el;
			}
		}

		deserialize_state(state_snapshot, state);
	}

	void deserialize_state(rubik_state_snapshot & state_snapshot, state_t & state) const
	{
		//Identify corner cubes
		state.cube(0, 0, 0) = identify_corner(state_snapshot.element(face_id::face_L, 2, 0), state_snapshot.element(face_id::face_F, 0, 0), state_snapshot.element(face_id::face_U, 0, 2));
		state.cube(2, 0, 0) = identify_corner(state_snapshot.element(face_id::face_F, 2, 0), state_snapshot.element(face_id::face_U, 2, 2), state_snapshot.element(face_id::face_R, 0, 0));
		state.cube(0, 2, 0) = identify_corner(state_snapshot.element(face_id::face_L, 2, 2), state_snapshot.element(face_id::face_F, 0, 2), state_snapshot.element(face_id::face_D, 0, 0));
		state.cube(2, 2, 0) = identify_corner(state_snapshot.element(face_id::face_F, 2, 2), state_snapshot.element(face_id::face_R, 0, 2), state_snapshot.element(face_id::face_D, 2, 0));
		state.cube(0, 0, 2) = identify_corner(state_snapshot.element(face_id::face_U, 0, 0), state_snapshot.element(face_id::face_L, 0, 0), state_snapshot.element(face_id::face_B, 2, 0));
		state.cube(2, 0, 2) = identify_corner(state_snapshot.element(face_id::face_U, 2, 0), state_snapshot.element(face_id::face_B, 0, 0), state_snapshot.element(face_id::face_R, 2, 0));
		state.cube(0, 2, 2) = identify_corner(state_snapshot.element(face_id::face_L, 0, 2), state_snapshot.element(face_id::face_B, 2, 2), state_snapshot.element(face_id::face_D, 0, 2));
		state.cube(2, 2, 2) = identify_corner(state_snapshot.element(face_id::face_R, 2, 2), state_snapshot.element(face_id::face_B, 0, 2), state_snapshot.element(face_id::face_D, 2, 2));

		//Identify edge cubes
		state.cube(1, 0, 0) = identify_edge(state_snapshot.element(face_id::face_F, 1, 0), state_snapshot.element(face_id::face_U, 1, 2));
		state.cube(0, 1, 0) = identify_edge(state_snapshot.element(face_id::face_L, 2, 1), state_snapshot.element(face_id::face_F, 0, 1));
		state.cube(2, 1, 0) = identify_edge(state_snapshot.element(face_id::face_F, 2, 1), state_snapshot.element(face_id::face_R, 0, 1));
		state.cube(1, 2, 0) = identify_edge(state_snapshot.element(face_id::face_F, 1, 2), state_snapshot.element(face_id::face_D, 1, 0));
		state.cube(0, 0, 1) = identify_edge(state_snapshot.element(face_id::face_L, 1, 0), state_snapshot.element(face_id::face_U, 0, 1));
		state.cube(2, 0, 1) = identify_edge(state_snapshot.element(face_id::face_U, 2, 1), state_snapshot.element(face_id::face_R, 1, 0));
		state.cube(0, 2, 1) = identify_edge(state_snapshot.element(face_id::face_L, 1, 2), state_snapshot.element(face_id::face_D, 0, 1));
		state.cube(2, 2, 1) = identify_edge(state_snapshot.element(face_id::face_D, 2, 1), state_snapshot.element(face_id::face_R, 1, 2));
		state.cube(1, 0, 2) = identify_edge(state_snapshot.element(face_id::face_U, 1, 0), state_snapshot.element(face_id::face_B, 1, 0));
		state.cube(0, 1, 2) = identify_edge(state_snapshot.element(face_id::face_L, 0, 1), state_snapshot.element(face_id::face_B, 2, 1));
		state.cube(2, 1, 2) = identify_edge(state_snapshot.element(face_id::face_R, 2, 1), state_snapshot.element(face_id::face_B, 0, 1));
		state.cube(1, 2, 2) = identify_edge(state_snapshot.element(face_id::face_B, 1, 2), state_snapshot.element(face_id::face_D, 1, 2));
	}

	void serialize_state(std::ostream & os, const state_t & state) const
	{
		os << m_size << std::endl << std::endl;
		
		rubik_state_snapshot snapshot(m_size);

		//==========================
		//Calculate snapshot
		//==========================

		//Central elements are the same as a face identifier
		for (int i = 0; i < 6; ++i)
			snapshot.element(static_cast<face_id>(i), 1, 1) = (element_t)i;

		//Identify edges

		//==========================
		//Print
		//==========================
		for (int i = 0; i < 6; ++i)
		{
			for (int y = 0; y < m_size; ++y)
			{
				for (int x = 0; x < m_size; ++x)
				{
					os << (int)snapshot.element(static_cast<face_id>(i), x, y) << " ";
				}

				os << std::endl;
			}

			os << std::endl;
		}
	}

	void apply(state_t & state, transition_t transition) const
	{
		bool inversed = false;
		switch (transition)
		{
		case rubiks_cube::transition_t::F_:
			inversed = true;
		case rubiks_cube::transition_t::F:
			/*state.face(face_id::face_F).transform(inversed, m_rotationTable);
			for (int i = 0; i < m_size; ++i)
				loop_offset(inversed, state.element(face_id::face_L, 2, i),
				state.element(face_id::face_U, 2 - i, 2),
				state.element(face_id::face_R, 0, 2 - i),
				state.element(face_id::face_D, i, 0));*/

			loop_offset(inversed, state.cube(1, 0, 0), state.cube(2, 1, 0), state.cube(1, 2, 0), state.cube(0, 1, 0));
			loop_offset(inversed, state.cube(0, 0, 0), state.cube(2, 0, 0), state.cube(2, 2, 0), state.cube(0, 2, 0));
			break;
		case rubiks_cube::transition_t::B_:
			inversed = true;
		case rubiks_cube::transition_t::B:
			/*state.face(face_id::face_B).transform(inversed, m_rotationTable);
			for (int i = 0; i < m_size; ++i)
				loop_offset(inversed, state.element(face_id::face_L, 0, i),
				state.element(face_id::face_D, i, 2),
				state.element(face_id::face_R, 2, 2 - i),
				state.element(face_id::face_U, 2 - i, 0));*/

			loop_offset(inversed, state.cube(0, 1, 2), state.cube(1, 2, 2), state.cube(2, 1, 2), state.cube(1, 0, 2));
			loop_offset(inversed, state.cube(0, 2, 2), state.cube(2, 2, 2), state.cube(2, 0, 2), state.cube(0, 0, 2));
			break;
		case rubiks_cube::transition_t::U_:
			inversed = true;
		case rubiks_cube::transition_t::U:
			/*state.face(face_id::face_U).transform(inversed, m_rotationTable);
			for (int i = 0; i < m_size; ++i)
				loop_offset(inversed, state.element(face_id::face_B, i, 0),
				state.element(face_id::face_R, i, 0),
				state.element(face_id::face_F, i, 0),
				state.element(face_id::face_L, i, 0));*/
			loop_offset(inversed, state.cube(1, 0, 2), state.cube(2, 0, 1), state.cube(1, 0, 0), state.cube(0, 0, 1));
			loop_offset(inversed, state.cube(0, 0, 2), state.cube(2, 0, 2), state.cube(2, 0, 0), state.cube(0, 0, 0));
			break;
		case rubiks_cube::transition_t::D_:
			inversed = true;
		case rubiks_cube::transition_t::D:
			/*state.face(face_id::face_D).transform(inversed, m_rotationTable);
			for (int i = 0; i < m_size; ++i)
				loop_offset(inversed, state.element(face_id::face_L, i, 2),
				state.element(face_id::face_F, i, 2),
				state.element(face_id::face_R, i, 2),
				state.element(face_id::face_B, i, 2));*/
			loop_offset(inversed, state.cube(1, 2, 0), state.cube(2, 2, 1), state.cube(1, 2, 2), state.cube(0, 2, 1));
			loop_offset(inversed, state.cube(0, 2, 0), state.cube(2, 2, 0), state.cube(2, 2, 2), state.cube(0, 2, 2));
			break;
		case rubiks_cube::transition_t::L_:
			inversed = true;
		case rubiks_cube::transition_t::L:
			/*state.face(face_id::face_L).transform(inversed, m_rotationTable);
			for (int i = 0; i < m_size; ++i)
				loop_offset(inversed, state.element(face_id::face_U, 0, i),
				state.element(face_id::face_F, 0, i),
				state.element(face_id::face_D, 0, i),
				state.element(face_id::face_B, 2, 2 - i));*/
			loop_offset(inversed, state.cube(0, 0, 1), state.cube(0, 1, 0), state.cube(0, 2, 1), state.cube(0, 1, 2));
			loop_offset(inversed, state.cube(0, 0, 2), state.cube(0, 0, 0), state.cube(0, 2, 0), state.cube(0, 2, 2));
			break;
		case rubiks_cube::transition_t::R_:
			inversed = true;
		case rubiks_cube::transition_t::R:
			/*state.face(face_id::face_R).transform(inversed, m_rotationTable);
			for (int i = 0; i < m_size; ++i)
				loop_offset(inversed, state.element(face_id::face_D, 2, i),
				state.element(face_id::face_F, 2, i),
				state.element(face_id::face_U, 2, i),
				state.element(face_id::face_B, 0, 2 - i));*/
			loop_offset(inversed, state.cube(2, 0, 1), state.cube(2, 1, 2), state.cube(2, 2, 1), state.cube(2, 1, 0));
			loop_offset(inversed, state.cube(2, 0, 0), state.cube(2, 0, 2), state.cube(2, 2, 2), state.cube(2, 2, 0));
			break;
		default:
			throw std::runtime_error("Unknown transition");
		}
	}

	const size_description_t & size() const
	{
		return m_size;
	}

	/*
	Each face has elements with the same number.
	*/
	state_t solved_state() const
	{
		rubik_state_snapshot state_snapshot(m_size);

		for (int i = 0; i < 6; ++i)
		{
			for (int y = 0; y < m_size; ++y)
			{
				for (int x = 0; x < m_size; ++x)
				{
					state_snapshot.element(static_cast<face_id>(i), x, y) = static_cast<state_t::element_t>(i);
				}
			}
		}

		state_t res(m_size);
		deserialize_state(state_snapshot, res);
		return std::move(res);
	}

	bool is_solved(const state_t & cur_state) const
	{
		for (int i = 0; i < 6; ++i)
		{
			int expected_color = i;
			for (int y = 0; y < m_size; ++y)
			{
				for (int x = 0; x < m_size; ++x)
				{
					if (cur_state.element(static_cast<face_id>(i), x, y) != expected_color)
						return false;
				}
			}
		}

		return true;
	}

	std::ostream & interpret_transition(std::ostream & os, const state_t & state, const transition_t & transition) const
	{
		os << transition;
		return os;
	}

	std::ostream & interpet_state(std::ostream & os, const state_t & state) const
	{
		for (int i = 0; i < 6; ++i)
		{
			for (int y = 0; y < m_size; ++y)
			{
				for (int x = 0; x < m_size; ++x)
				{
					os << (int)state.element(static_cast<face_id>(i), x, y) << " ";
				}

				os << std::endl;
			}

			os << std::endl;
		}

		return os;
	}

	friend std::ostream & operator<<(std::ostream & os, const transition_t & transition)
	{
		switch (transition)
		{
		case rubiks_cube::transition_t::F_:
			os << "F'";
			break;
		case rubiks_cube::transition_t::F:
			os << "F";
			break;
		case rubiks_cube::transition_t::B_:
			os << "B'";
			break;
		case rubiks_cube::transition_t::B:
			os << "B";
			break;
		case rubiks_cube::transition_t::U_:
			os << "U'";
			break;
		case rubiks_cube::transition_t::U:
			os << "U";
			break;
		case rubiks_cube::transition_t::D_:
			os << "D'";
			break;
		case rubiks_cube::transition_t::D:
			os << "D";
			break;
		case rubiks_cube::transition_t::L_:
			os << "L'";
			break;
		case rubiks_cube::transition_t::L:
			os << "L";
			break;
		case rubiks_cube::transition_t::R_:
			os << "R'";
			break;
		case rubiks_cube::transition_t::R:
			os << "R";
			break;
		default:
			throw std::runtime_error("Unknown transition");
		}

		return os;
	}

	transition_t difference(const state_t & lhs, const state_t & rhs)
	{
		for (int i = 0; i < static_cast<int>(transition_t::count); ++i)
		{
			state_t sample_state(lhs);
			apply(sample_state, static_cast<transition_t>(i));
			if (lhs == rhs)
				return static_cast<transition_t>(i);
		}

		throw std::runtime_error("Transition not found");
		return transition_t::count;
	}
private:
	void init_rotation_table()
	{
		//Top -> Right
		for (int i = 0; i < m_size - 1; ++i)
		{
			int x0 = i, y0 = 0, x1 = m_size - 1, y1 = i;
			m_rotationTable.push_back(std::make_pair(y0 * m_size + x0, y1 * m_size + x1));
		}

		//Right -> Bottom
		for (int i = 0; i < m_size - 1; ++i)
		{
			int x0 = m_size - 1, y0 = i, x1 = m_size - 1 - i, y1 = m_size - 1;
			m_rotationTable.push_back(std::make_pair(y0 * m_size + x0, y1 * m_size + x1));
		}

		//Bottom -> Left
		for (int i = 0; i < m_size - 1; ++i)
		{
			int x0 = m_size - 1 - i, y0 = m_size - 1, x1 = 0, y1 = m_size - 1 - i;
			m_rotationTable.push_back(std::make_pair(y0 * m_size + x0, y1 * m_size + x1));
		}

		//Left -> Top
		for (int i = 0; i < m_size - 1; ++i)
		{
			int x0 = 0, y0 = m_size - 1 - i, x1 = i, y1 = 0;
			m_rotationTable.push_back(std::make_pair(y0 * m_size + x0, y1 * m_size + x1));
		}

		std::sort(m_rotationTable.begin(), m_rotationTable.end());
	}

	element_t identify_corner(element_t first_color, element_t second_color, element_t third_color) const
	{
		std::map<std::tuple<element_t, element_t, element_t>, element_t> corner_mapping = {
			{ make_tuple(0, 1, 4), 0},
			{ make_tuple(1, 4, 2), 2},
			{ make_tuple(0, 1, 5), 5},
			{ make_tuple(1, 5, 2), 7},
			{ make_tuple(0, 4, 3), 12},
			{ make_tuple(2, 4, 3), 14},
			{ make_tuple(0, 3, 5), 17},
			{ make_tuple(2, 3, 5), 19}
		};

		std::array<element_t, 3> input = { first_color, second_color, third_color};
		std::sort(input.begin(), input.end());

		for (auto & rec : corner_mapping)
		{
			std::array<element_t, 3> rec_data = {get<0>(rec.first), get<1>(rec.first), get<2>(rec.first)};
			std::sort(rec_data.begin(), rec_data.end());
			if (input == rec_data)
				return rec.second;
		}

#if _DEBUG
		throw runtime_error("Unable to identify corner");
#endif
		return std::numeric_limits<element_t>::max();
	}

	element_t identify_edge(element_t first_color, element_t second_color) const
	{
		std::map<std::pair<element_t, element_t>, element_t> edge_mapping = {
			{ make_pair(4, 1), 1 },
			{ make_pair(0, 1), 3 },
			{ make_pair(1, 2), 4 },
			{ make_pair(1, 5), 6 },
			{ make_pair(0, 4), 8 },
			{ make_pair(4, 2), 9 },
			{ make_pair(0, 5), 10 },
			{ make_pair(2, 5), 11 },
			{ make_pair(4, 3), 13 },
			{ make_pair(0, 3), 15 },
			{ make_pair(2, 3), 16 },
			{ make_pair(5, 3), 18 }
		};

		std::array<element_t, 2> input = { first_color, second_color};
		std::sort(input.begin(), input.end());

		for (auto & rec : edge_mapping)
		{
			std::array<element_t, 2> rec_data = { rec.first.first, rec.first.second };
			std::sort(rec_data.begin(), rec_data.end());
			if (input == rec_data)
				return rec.second;
		}

#if _DEBUG
		throw runtime_error("Unable to identify edge cube");
#endif
		return std::numeric_limits<element_t>::max();
	}

private:
	size_description_t m_size;
	std::vector<std::pair<int, int>> m_rotationTable;
};

namespace std {
	template<>
	class hash<rubik_state>
	{
		using element_t = rubik_state::element_t;
	public:

		size_t operator()(const rubik_state & state) const
		{
			const int size = state.m_size * state.m_size * state.m_size - 6 - 1;
			if (m_hasher.m_size != size)
				m_hasher = mr_hasher<element_t>(size);

			for (int i = 0, j = 0; i < state.m_cubes.size(); ++i)
			{
				if (state.m_cubes[i] != std::numeric_limits<element_t>::max())
					m_hasher.cache.Pi[j++] = state.m_cubes[i];
			}

			return m_hasher();
		}
	private:
		mutable mr_hasher<element_t> m_hasher;
	};
}

#endif
