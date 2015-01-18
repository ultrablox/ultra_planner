
#ifndef UltraTest_rubiks_cube_h
#define UltraTest_rubiks_cube_h

struct rubik_state
{

};

class rubiks_cube
{
public:
	typedef rubik_state state_t;
	typedef int transition_t;
	typedef int size_description_t;

	rubiks_cube(int problem_size)
		:m_size(problem_size)
	{

	}

private:
	int m_size;
};

#endif
