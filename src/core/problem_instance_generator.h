
#ifndef UltraSolver_problem_instance_generator_h
#define UltraSolver_problem_instance_generator_h

#include <ostream>
#include <random>
#include <vector>
#include <chrono>

template<typename T>
class problem_instance_generator
{
	using problem_t = T;
	using problem_size_description_t = typename problem_t::size_description_t;
	using state_t = typename problem_t::state_t;
	using transition_t = typename problem_t::transition_t;
public:
	void generate(const problem_size_description_t & size, int permutation_count, std::ostream & os)
	{
		problem_t problem(size);
		
		std::vector<transition_t> path;

		state_t state = generate_state(problem, permutation_count, &path);

		problem.serialize_state(os, state);

		std::reverse(path.begin(), path.end());
		os << "Solution (" << path.size() << " length): ";
	}

	state_t generate_state(const problem_t & problem, int permutation_count, std::vector<transition_t> * p_transitions = nullptr)
	{
#if 0
		std::default_random_engine generator;
#else
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 generator(seed);
#endif
		std::uniform_int_distribution<int> distribution(0, 999);

		state_t state = problem.solved_state();

		for (int i = 0; i < permutation_count; ++i)
		{
			std::vector<transition_t> possible_trans;
			problem.forall_available_transitions(state, [&](const transition_t & transition){
				possible_trans.push_back(transition);
			});

			int random_index = distribution(generator);
			transition_t selected_trans = possible_trans[random_index % possible_trans.size()];

			problem.apply(state, selected_trans);

			if (p_transitions)
				p_transitions->push_back(selected_trans);
		}

		return std::move(state);
	}
};

#endif
