
#ifndef UltraTest_rubiks_cube_h
#define UltraTest_rubiks_cube_h

#include <vector>
#include <array>
#include <core/utils/helpers.h>
#include <core/streamer.h>
#include <core/hash.h>
#include <numeric>
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

struct point2
{
	point2(int _x, int _y)
		:x(_x), y(_y)
	{}

	friend bool operator==(const point2 & lhs, const point2 & rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == rhs.y);
	}

	int x, y;
};

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
		:m_size(size), /*m_cubes(size * size * size, std::numeric_limits<element_t>::max()),*/ m_edgeCubies(12), m_edgeMarkers(12, 0)
	{
		//faces.fill(face_t(size));
		m_cornerMarkers.fill(0);
	}

	/*face_t & face(face_id id)
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
*/
	friend bool operator==(const rubik_state & lhs, const rubik_state & rhs)
	{
		//return lhs.faces == rhs.faces;
		//return lhs.m_cubes == rhs.m_cubes;
		return (lhs.m_cornerCubies == rhs.m_cornerCubies) &&
			(lhs.m_edgeCubies == rhs.m_edgeCubies) &&
			(lhs.m_cornerMarkers == rhs.m_cornerMarkers) &&
			(lhs.m_edgeMarkers == rhs.m_edgeMarkers);
	}

	std::array<element_t, 8> m_cornerCubies;
	std::vector<element_t> m_edgeCubies;

	std::array<element_t, 8> m_cornerMarkers;
	std::vector<element_t> m_edgeMarkers;

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

	const element_t & element(face_id f_id, int x, int y) const
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

	template<typename Fun>
	void for_each_facet(Fun fun) const
	{
		for (int i = 0; i < 6; ++i)
		{
			for (int x = 0; x < m_size; ++x)
			{
				for (int y = 0; y < m_size; ++y)
				{
					fun(static_cast<face_id>(i), point2(x, y), element(static_cast<face_id>(i), x, y));
				}
			}
		}
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

	enum class transition_t { R = 0, L, U, D, F, B, count, F_, B_, U_, D_, L_, R_ };

	struct transition_data_t
	{
		using permutation_loop = std::vector<element_t>;
		using marker_deltas = std::vector<element_t>;

		transition_data_t(const std::initializer_list<element_t> & corners_list, const std::initializer_list<element_t> & edges_list, const std::initializer_list<element_t> & _corners_deltas, const std::initializer_list<element_t> & _edges_deltas)
			:corners_loop(corners_list), edges_loop(edges_list), corner_markers_deltas(_corners_deltas), edge_markers_deltas(_edges_deltas)
		{
		}

		permutation_loop corners_loop, edges_loop;
		marker_deltas corner_markers_deltas, edge_markers_deltas;
	};

	class state_streamer_t : public streamer_base
	{
	public:
		state_streamer_t(const rubiks_cube & _rubik)
			:streamer_base(_rubik.m_size * _rubik.m_size * 6 * sizeof(element_t)), m_size(_rubik.m_size)
		{}

		void serialize(void * dst, const state_t & state) const
		{
			/*const int step_size = m_size * m_size * sizeof(element_t);

			char * dest_ptr = (char*)dst;
			for (int i = 0; i < 6; ++i, dest_ptr += step_size)
				memcpy(dest_ptr, state.faces[i].data.data(), step_size);*/
		}

		void deserialize(const void * src, state_t & state) const
		{
			/*const int step_size = m_size * m_size * sizeof(element_t);

			char * src_ptr = (char*)src;

			for (int i = 0; i < 6; ++i, src_ptr += step_size)
			{
				state.faces[i].data.resize(m_size * m_size);
				memcpy(state.faces[i].data.data(), src_ptr, step_size);
			}*/
		}

	private:
		size_description_t m_size;
	};

	using corner_colors_t = std::array<element_t, 3>;
	using edge_colors_t = std::array<element_t, 2>;

	rubiks_cube(int problem_size)
		:m_size(problem_size), m_markerMap(problem_size), m_facetToCubieMap(problem_size), m_edgeColorsMap(12)
	{

		m_corners2DCoords = { point2(0, 0), point2(m_size - 1, 0), point2(m_size - 1, m_size - 1), point2(0, m_size - 1) };
		m_edges2DCoords = { point2(1, 0), point2(m_size - 1, 1), point2(1, m_size - 1), point2(0, 1) };

		//Init color mapping (face -> color)
		m_colorMapper = { { 'W', 'R', 'Y', 'O', 'B', 'G' } };


		//Init markers map
		m_markerMap.element(face_id::face_L, 0, 0) = 1;
		m_markerMap.element(face_id::face_L, 1, 0) = 1;
		m_markerMap.element(face_id::face_L, 2, 0) = 2;
		m_markerMap.element(face_id::face_L, 0, 1) = 0;
		m_markerMap.element(face_id::face_L, 2, 1) = 0;
		m_markerMap.element(face_id::face_L, 0, 2) = 2;
		m_markerMap.element(face_id::face_L, 1, 2) = 1;
		m_markerMap.element(face_id::face_L, 2, 2) = 1;

		m_markerMap.element(face_id::face_U, 0, 0) = 0;
		m_markerMap.element(face_id::face_U, 1, 0) = 0;
		m_markerMap.element(face_id::face_U, 2, 0) = 0;
		m_markerMap.element(face_id::face_U, 0, 1) = 0;
		m_markerMap.element(face_id::face_U, 2, 1) = 0;
		m_markerMap.element(face_id::face_U, 0, 2) = 0;
		m_markerMap.element(face_id::face_U, 1, 2) = 0;
		m_markerMap.element(face_id::face_U, 2, 2) = 0;


		m_markerMap.element(face_id::face_F, 0, 0) = 1;
		m_markerMap.element(face_id::face_F, 1, 0) = 1;
		m_markerMap.element(face_id::face_F, 2, 0) = 2;
		m_markerMap.element(face_id::face_F, 0, 1) = 1;
		m_markerMap.element(face_id::face_F, 2, 1) = 1;
		m_markerMap.element(face_id::face_F, 0, 2) = 2;
		m_markerMap.element(face_id::face_F, 1, 2) = 1;
		m_markerMap.element(face_id::face_F, 2, 2) = 1;

		m_markerMap.element(face_id::face_R, 0, 0) = 1;
		m_markerMap.element(face_id::face_R, 1, 0) = 1;
		m_markerMap.element(face_id::face_R, 2, 0) = 2;
		m_markerMap.element(face_id::face_R, 0, 1) = 0;
		m_markerMap.element(face_id::face_R, 2, 1) = 0;
		m_markerMap.element(face_id::face_R, 0, 2) = 2;
		m_markerMap.element(face_id::face_R, 1, 2) = 1;
		m_markerMap.element(face_id::face_R, 2, 2) = 1;

		m_markerMap.element(face_id::face_B, 0, 0) = 1;
		m_markerMap.element(face_id::face_B, 1, 0) = 1;
		m_markerMap.element(face_id::face_B, 2, 0) = 2;
		m_markerMap.element(face_id::face_B, 0, 1) = 1;
		m_markerMap.element(face_id::face_B, 2, 1) = 1;
		m_markerMap.element(face_id::face_B, 0, 2) = 2;
		m_markerMap.element(face_id::face_B, 1, 2) = 1;
		m_markerMap.element(face_id::face_B, 2, 2) = 1;

		m_markerMap.element(face_id::face_D, 0, 0) = 0;
		m_markerMap.element(face_id::face_D, 1, 0) = 0;
		m_markerMap.element(face_id::face_D, 2, 0) = 0;
		m_markerMap.element(face_id::face_D, 0, 1) = 0;
		m_markerMap.element(face_id::face_D, 2, 1) = 0;
		m_markerMap.element(face_id::face_D, 0, 2) = 0;
		m_markerMap.element(face_id::face_D, 1, 2) = 0;
		m_markerMap.element(face_id::face_D, 2, 2) = 0;

		//Init Facet->Cibie map
		//(corners)
		m_facetToCubieMap.element(face_id::face_U, 0, 0) = 0;
		m_facetToCubieMap.element(face_id::face_U, 2, 0) = 1;
		m_facetToCubieMap.element(face_id::face_U, 2, 2) = 2;
		m_facetToCubieMap.element(face_id::face_U, 0, 2) = 3;

		m_facetToCubieMap.element(face_id::face_L, 0, 0) = 0;
		m_facetToCubieMap.element(face_id::face_L, 2, 0) = 3;
		m_facetToCubieMap.element(face_id::face_L, 2, 2) = 7;
		m_facetToCubieMap.element(face_id::face_L, 0, 2) = 4;

		m_facetToCubieMap.element(face_id::face_F, 0, 0) = 3;
		m_facetToCubieMap.element(face_id::face_F, 2, 0) = 2;
		m_facetToCubieMap.element(face_id::face_F, 2, 2) = 6;
		m_facetToCubieMap.element(face_id::face_F, 0, 2) = 7;

		m_facetToCubieMap.element(face_id::face_R, 0, 0) = 2;
		m_facetToCubieMap.element(face_id::face_R, 2, 0) = 1;
		m_facetToCubieMap.element(face_id::face_R, 2, 2) = 5;
		m_facetToCubieMap.element(face_id::face_R, 0, 2) = 6;

		m_facetToCubieMap.element(face_id::face_B, 0, 0) = 1;
		m_facetToCubieMap.element(face_id::face_B, 2, 0) = 0;
		m_facetToCubieMap.element(face_id::face_B, 2, 2) = 4;
		m_facetToCubieMap.element(face_id::face_B, 0, 2) = 5;

		m_facetToCubieMap.element(face_id::face_D, 0, 0) = 7;
		m_facetToCubieMap.element(face_id::face_D, 2, 0) = 6;
		m_facetToCubieMap.element(face_id::face_D, 2, 2) = 5;
		m_facetToCubieMap.element(face_id::face_D, 0, 2) = 4;

		//(edges)
		m_facetToCubieMap.element(face_id::face_U, 1, 0) = 0;
		m_facetToCubieMap.element(face_id::face_U, 2, 1) = 1;
		m_facetToCubieMap.element(face_id::face_U, 1, 2) = 2;
		m_facetToCubieMap.element(face_id::face_U, 0, 1) = 3;

		m_facetToCubieMap.element(face_id::face_L, 1, 0) = 3;
		m_facetToCubieMap.element(face_id::face_L, 2, 1) = 7;
		m_facetToCubieMap.element(face_id::face_L, 1, 2) = 11;
		m_facetToCubieMap.element(face_id::face_L, 0, 1) = 4;

		m_facetToCubieMap.element(face_id::face_F, 1, 0) = 2;
		m_facetToCubieMap.element(face_id::face_F, 2, 1) = 6;
		m_facetToCubieMap.element(face_id::face_F, 1, 2) = 10;
		m_facetToCubieMap.element(face_id::face_F, 0, 1) = 7;

		m_facetToCubieMap.element(face_id::face_R, 1, 0) = 1;
		m_facetToCubieMap.element(face_id::face_R, 2, 1) = 5;
		m_facetToCubieMap.element(face_id::face_R, 1, 2) = 9;
		m_facetToCubieMap.element(face_id::face_R, 0, 1) = 6;

		m_facetToCubieMap.element(face_id::face_B, 1, 0) = 0;
		m_facetToCubieMap.element(face_id::face_B, 2, 1) = 4;
		m_facetToCubieMap.element(face_id::face_B, 1, 2) = 8;
		m_facetToCubieMap.element(face_id::face_B, 0, 1) = 5;

		m_facetToCubieMap.element(face_id::face_D, 1, 0) = 10;
		m_facetToCubieMap.element(face_id::face_D, 2, 1) = 9;
		m_facetToCubieMap.element(face_id::face_D, 1, 2) = 8;
		m_facetToCubieMap.element(face_id::face_D, 0, 1) = 11;

		//Initialize default cubies colors
		
		for (int i = 0; i < 6; ++i)
		{
			//(corners)
			for (auto & p : m_corners2DCoords)
			{
				element_t cubie_id = m_facetToCubieMap.element(static_cast<face_id>(i), p.x, p.y);
				element_t marker_id = m_markerMap.element(static_cast<face_id>(i), p.x, p.y);
					
				m_cornerColorsMap[cubie_id][marker_id] = i;
			}

			//(edges)

			for (auto & p : m_edges2DCoords)
			{
				element_t cubie_id = m_facetToCubieMap.element(static_cast<face_id>(i), p.x, p.y);
				element_t marker_id = m_markerMap.element(static_cast<face_id>(i), p.x, p.y);

				m_edgeColorsMap[cubie_id][marker_id] = i;
			}
		}

		//Initialize transitions
		m_transitions = { transition_data_t({ 1, 5, 6, 2 }, { 1, 5, 9, 6 }, { 0, 1, 2, 0, 0, 2, 1, 0 }, {0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0}),	//R
			transition_data_t({ 0, 3, 7, 4 }, { 3, 7, 11, 4 }, { 2, 0, 0, 1, 1, 0, 0, 2 }, {0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1}), //L
			transition_data_t({ 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), //U
			transition_data_t({ 4, 7, 6, 5 }, { 8, 11, 10, 9 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }), //D
			transition_data_t({ 2, 6, 7, 3 }, { 2, 6, 10, 7 }, { 0, 0, 1, 2, 0, 0, 2, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }), //F
			transition_data_t({ 0, 4, 5, 1 }, { 0, 4, 8, 5 }, { 1, 2, 0, 0, 2, 1, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }) //B
		};
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

		char el;

		auto single_face_reader = [&](face_id face){
			for (int y = 0; y < m_size; ++y)
			{
				for (int x = 0; x < m_size; ++x)
				{
					is >> el;
					state_snapshot.element(face, x, y) = color_id(el);
				}
			}
		};

		auto multi_face_reader = [&](face_id face_first, face_id face_last){
			for (int y = 0; y < m_size; ++y)
			{
				for (int i = static_cast<int>(face_first); i < static_cast<int>(face_last); ++i)
				{
					for (int x = 0; x < m_size; ++x)
					{
						is >> el;
						state_snapshot.element(static_cast<face_id>(i), x, y) = color_id(el);
					}
				}
			}
		};

		single_face_reader(face_id::face_U);

		multi_face_reader(face_id::face_L, face_id::face_U);

		single_face_reader(face_id::face_D);

		deserialize_state(state_snapshot, state);
	}

	void serialize_state(std::ostream & os, const state_t & state) const
	{
		os << m_size << std::endl << std::endl;
		
		rubik_state_snapshot snapshot(m_size);

		//==========================
		//Calculate snapshot
		//==========================
		
		for (int i = 0; i < 6; ++i)
		{
			//Central elements are the same as a face identifier
			snapshot.element(static_cast<face_id>(i), 1, 1) = (element_t)i;

			//Identify corners
			for (auto & p : m_corners2DCoords)
			{
				element_t cubie_id = state.m_cornerCubies[m_facetToCubieMap.element(static_cast<face_id>(i), p.x, p.y)];
				element_t marker_id = (m_markerMap.element(static_cast<face_id>(i), p.x, p.y) + state.m_cornerMarkers[cubie_id] ) % 3;
				snapshot.element(static_cast<face_id>(i), p.x, p.y) = m_cornerColorsMap[cubie_id][marker_id];
			}

			//Identify edges
			for (auto & p : m_edges2DCoords)
			{
				element_t cubie_id = state.m_edgeCubies[m_facetToCubieMap.element(static_cast<face_id>(i), p.x, p.y)];
				element_t marker_id = (m_markerMap.element(static_cast<face_id>(i), p.x, p.y) + state.m_edgeMarkers[cubie_id] ) % 2;
				snapshot.element(static_cast<face_id>(i), p.x, p.y) = m_edgeColorsMap[cubie_id][marker_id];
			}
		}

		//==========================
		//Print
		//==========================
		auto single_face_printer = [&](face_id face, int offset_left){
			
			for (int y = 0; y < m_size; ++y)
			{
				for (int i = 0; i < offset_left; ++i)
					for (int j = 0; j < m_size; ++j)
						os << ' ' << ' ';
				os << ' ';

				for (int x = 0; x < m_size; ++x)
					os << m_colorMapper[snapshot.element(face, x, y)] << " ";
				os << std::endl;
			}
		};

		auto multi_face_printer = [&](face_id face_first, face_id face_last){
			for (int y = 0; y < m_size; ++y)
			{
				for (int i = static_cast<int>(face_first); i < static_cast<int>(face_last); ++i)
				{
					for (int x = 0; x < m_size; ++x)
					{
						os << m_colorMapper[snapshot.element(static_cast<face_id>(i), x, y)] << " ";
					}
					os << ' ';
				}
				os << std::endl;
			}
		};


		//U
		single_face_printer(face_id::face_U, 1);

		//L-B
		multi_face_printer(face_id::face_L, face_id::face_U);

		//D
		single_face_printer(face_id::face_D, 1);
	}

	void apply(state_t & state, transition_t transition) const
	{
		const transition_data_t & tr_data = m_transitions[static_cast<int>(transition)];
		
		//Increment corner markers
		for (int i = 0; i < 8; ++i)
		{
			int cubie_id = state.m_cornerCubies[i];
			state.m_cornerMarkers[cubie_id] = (state.m_cornerMarkers[cubie_id] + tr_data.corner_markers_deltas[i]) % 3;
		}

		//Increment edge cubies markers
		for (int i = 0; i < 12; ++i)
		{
			int cubie_id = state.m_edgeCubies[i];
			state.m_edgeMarkers[cubie_id] = (state.m_edgeMarkers[cubie_id] + tr_data.edge_markers_deltas[i]) % 2;
		}

		//Permutate corners
		loop_offset(state.m_cornerCubies[tr_data.corners_loop[0]], state.m_cornerCubies[tr_data.corners_loop[1]], state.m_cornerCubies[tr_data.corners_loop[2]], state.m_cornerCubies[tr_data.corners_loop[3]]);

		//Permutate edges
		loop_offset(state.m_edgeCubies[tr_data.edges_loop[0]], state.m_edgeCubies[tr_data.edges_loop[1]], state.m_edgeCubies[tr_data.edges_loop[2]], state.m_edgeCubies[tr_data.edges_loop[3]]);
	}

	const size_description_t & size() const
	{
		return m_size;
	}

	state_t solved_state() const
	{
		state_t res(m_size);

		//Corner cubies
		for (element_t i = 0; i < 8; ++i)
			res.m_cornerCubies[i] = i;

		//Edge cubies
		for (element_t i = 0; i < 12; ++i)
			res.m_edgeCubies[i] = i;

		//Markers are zero by default
		return std::move(res);
	}

	bool is_solved(const state_t & state) const
	{
		return std::is_sorted(state.m_cornerCubies.begin(), state.m_cornerCubies.end()) && 
				std::is_sorted(state.m_edgeCubies.begin(), state.m_edgeCubies.end()) && 
				(std::accumulate(state.m_cornerMarkers.begin(), state.m_cornerMarkers.end(), 0) == 0) && 
				(std::accumulate(state.m_edgeMarkers.begin(), state.m_edgeMarkers.end(), 0) == 0);
	}

	std::ostream & interpret_transition(std::ostream & os, const state_t & state, const transition_t & transition) const
	{
		os << transition;
		return os;
	}

	std::ostream & interpet_state(std::ostream & os, const state_t & state) const
	{
		/*for (int i = 0; i < 6; ++i)
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
		}*/
		serialize_state(os, state);

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
	void deserialize_state(rubik_state_snapshot & ss, state_t & state) const
	{
		//Identify corner cubes;

		identify_corner(state, ss.element(face_id::face_U, 0, 0), ss.element(face_id::face_L, 0, 0), ss.element(face_id::face_B, 2, 0), state.m_cornerCubies[0]);
		identify_corner(state, ss.element(face_id::face_U, 2, 0), ss.element(face_id::face_B, 0, 0), ss.element(face_id::face_R, 2, 0), state.m_cornerCubies[1]);
		identify_corner(state, ss.element(face_id::face_U, 2, 2), ss.element(face_id::face_R, 0, 0), ss.element(face_id::face_F, 2, 0), state.m_cornerCubies[2]);
		identify_corner(state, ss.element(face_id::face_U, 0, 2), ss.element(face_id::face_F, 0, 0), ss.element(face_id::face_L, 2, 0), state.m_cornerCubies[3]);
		identify_corner(state, ss.element(face_id::face_D, 0, 2), ss.element(face_id::face_B, 2, 2), ss.element(face_id::face_L, 0, 2), state.m_cornerCubies[4]);
		identify_corner(state, ss.element(face_id::face_D, 2, 2), ss.element(face_id::face_R, 2, 2), ss.element(face_id::face_B, 0, 2), state.m_cornerCubies[5]);
		identify_corner(state, ss.element(face_id::face_D, 2, 0), ss.element(face_id::face_F, 2, 2), ss.element(face_id::face_R, 0, 2), state.m_cornerCubies[6]);
		identify_corner(state, ss.element(face_id::face_D, 0, 0), ss.element(face_id::face_L, 2, 2), ss.element(face_id::face_F, 0, 2), state.m_cornerCubies[7]);

		//Identify edge cubes
		identify_edge(state, ss.element(face_id::face_U, 1, 0), ss.element(face_id::face_B, 1, 0), state.m_edgeCubies[0]);
		identify_edge(state, ss.element(face_id::face_U, 2, 1), ss.element(face_id::face_R, 1, 0), state.m_edgeCubies[1]);
		identify_edge(state, ss.element(face_id::face_U, 1, 2), ss.element(face_id::face_F, 1, 0), state.m_edgeCubies[2]);
		identify_edge(state, ss.element(face_id::face_U, 0, 1), ss.element(face_id::face_L, 1, 0), state.m_edgeCubies[3]);
		identify_edge(state, ss.element(face_id::face_L, 0, 1), ss.element(face_id::face_B, 2, 1), state.m_edgeCubies[4]);
		identify_edge(state, ss.element(face_id::face_R, 2, 1), ss.element(face_id::face_B, 0, 1), state.m_edgeCubies[5]);
		identify_edge(state, ss.element(face_id::face_R, 0, 1), ss.element(face_id::face_F, 2, 1), state.m_edgeCubies[6]);
		identify_edge(state, ss.element(face_id::face_L, 2, 1), ss.element(face_id::face_F, 0, 1), state.m_edgeCubies[7]);
		identify_edge(state, ss.element(face_id::face_D, 1, 2), ss.element(face_id::face_B, 1, 2), state.m_edgeCubies[8]);
		identify_edge(state, ss.element(face_id::face_D, 2, 1), ss.element(face_id::face_R, 1, 2), state.m_edgeCubies[9]);
		identify_edge(state, ss.element(face_id::face_D, 1, 0), ss.element(face_id::face_F, 1, 2), state.m_edgeCubies[10]);
		identify_edge(state, ss.element(face_id::face_D, 0, 1), ss.element(face_id::face_L, 1, 2), state.m_edgeCubies[11]);
	}

	void identify_corner(state_t & state, element_t first_color, element_t second_color, element_t third_color, element_t & cubie_id) const
	{
		corner_colors_t input = { first_color, second_color, third_color };
		std::sort(input.begin(), input.end());

		auto it = std::find_if(m_cornerColorsMap.begin(), m_cornerColorsMap.end(), [&](corner_colors_t map_colors){
			std::sort(map_colors.begin(), map_colors.end());
			return map_colors == input;
		});

		cubie_id = std::distance(m_cornerColorsMap.begin(), it);
		input = { { first_color, second_color, third_color } };

		element_t & cubie_marker = state.m_cornerMarkers[cubie_id];
		cubie_marker = 0;

		for (; input != *it; loop_offset(input), ++cubie_marker);
	}

	void identify_edge(state_t & state, element_t first_color, element_t second_color, element_t & cubie_id) const
	{
		std::array<element_t, 2> input = { first_color, second_color };
		std::sort(input.begin(), input.end());

		auto it = std::find_if(m_edgeColorsMap.begin(), m_edgeColorsMap.end(), [&](edge_colors_t map_colors){
			std::sort(map_colors.begin(), map_colors.end());
			return map_colors == input;
		});

		cubie_id = std::distance(m_edgeColorsMap.begin(), it);

		element_t & cubie_marker = state.m_edgeMarkers[cubie_id];

		if (first_color == (*it)[0])
			cubie_marker = 0;
		else
			cubie_marker = 1;
	}

	element_t color_id(char color_book) const
	{
		auto it = std::find(m_colorMapper.begin(), m_colorMapper.end(), color_book);
		return std::distance(m_colorMapper.begin(), it);
	}

private:
	size_description_t m_size;
	rubik_state_snapshot m_markerMap, m_facetToCubieMap;
	std::vector<transition_data_t> m_transitions;
	std::array<char, 8> m_colorMapper;

	std::array<corner_colors_t, 8> m_cornerColorsMap;
	std::vector<edge_colors_t> m_edgeColorsMap;
	std::vector<point2> m_corners2DCoords, m_edges2DCoords;
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

			int j = 0;
			for (int i = 0; i < 8; ++i)
				m_hasher.cache.Pi[j++] = state.m_cornerCubies[i];

			for (int i = 0; i < state.m_edgeCubies.size(); ++i)
				m_hasher.cache.Pi[j++] = state.m_edgeCubies[i] + 8;

			return m_hasher();
		}
	private:
		mutable mr_hasher<element_t> m_hasher;
	};
}

#endif
