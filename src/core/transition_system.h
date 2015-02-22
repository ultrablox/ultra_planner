

#ifndef UltraCore_transition_system_h
#define UltraCore_transition_system_h

#include <functional>
#include <list>
#include <utility>
#include <iostream>
#include <type_traits>

template<bool B, typename T = void>
using Enable_if = typename std::enable_if<B, T>::type;

template<typename T> bool Is_class()
{
	return std::is_class<T>::value;
}

template<typename T>
class transition_system : public T
{
//	static_assert(std::is_class<T::state_streamer_t>::value, "Transition system base must have its state_streamer_t");
public:
	typedef T _Base;
	typedef typename _Base::state_t state_t;
	typedef typename _Base::transition_t transition_t;
	//typedef typename _Base::size_description_t size_description_t;

	template<typename Args>
	transition_system(Args descr)
		:_Base(descr)
	{
	}

	template<typename ...Args>
	transition_system(Args... descr)
		: _Base(descr...)
	{
	}

	/*transition_system(const transition_system & rhs)
		:_Base(rhs.m_size)
	{
	}*/

	/*transition_system & operator=(const transition_system & rhs)
	{
		m_state = rhs.m_state;
	}

	const state_t & state() const
	{
		return m_state;
	}

	state_t & state()
	{
		return m_state;
	}*/

	/*
	Applies transition and generating a new state.
	*/
	/*template<typename TrFun>
	void apply(TrFun tr_fun, transition_t transition)
	{
		tr_fun(m_state, transition);
	}

	transition_t difference(const state_t & lhs, const state_t & rhs)
	{
		return m_diffFun(lhs, rhs);
	}*/

	//
	/*typename std::enable_if<std::is_member_function_pointer<decltype(&_Base::difference)>::value, transition_t>::type difference(const state_t & lhs, const state_t & rhs) const
	{
		return _Base::difference(lhs, rhs);
	}*/

//	using has_fast_difference_implementation = Is_class<typename std::is_member_function_pointer<transition_t(&T::difference)>::value>();

	/*Enable_if<Is_class<has_fast_difference_implementation>(), has_fast_difference_implementation> int test()
	{
		return 0;
	}*/

	//using non_uniform_transitions = typename std::is_member_function_pointer<decltype(&T::transition_cost)>::value;

	/*template<typename U>
	using non_u = std::is_member_function_pointer<decltype(&U::transition_cost)>;


	template<typename U = T>
	Enable_if<non_u<U>::value, float> transition_cost(const typename U::state_t & s, const typename U::transition_t & t) const
	{
		return U::transition_cost(s, t);
	}

	template<typename U = T>
	Enable_if<!std::is_member_function_pointer<decltype(&U::transition_cost)>::value, float> transition_cost(const typename U::state_t &, const typename U::transition_t &) const
	{
		return 1.0f;
	}

	template<bool NonUniform>
	float get_transition_cost(const state_t & s, const transition_t & t) const
	{
		return 1.0f;
	}

	template<typename U = T>
	float transition_cost(const state_t & s, const transition_t & t) const
	{
		return get_transition_cost<non_u<U>::value>(s, t);
	}*/

	transition_t difference(const state_t & lhs, const state_t & rhs) const
	{
		bool found = false;
		transition_t res_transition;
		this->forall_available_transitions(lhs, [&](const transition_t & transition){
			state_t sample_state(lhs);
			this->apply(sample_state, transition);
			if (sample_state == rhs)
			{
				res_transition = transition;
				found = true;
			}
		});

		if (!found)
		{
#if _DEBUG
			std::cout << "Unable to find transition between states:" << std::endl;
			std::cout << "State #1:" << std::endl;
			_Base::interpet_state(cout, lhs);
			std::cout << "State #2:" << std::endl;
			_Base::interpet_state(cout, rhs);
#endif
			throw std::runtime_error("Transition not found");
		}

		return std::move(res_transition);
	}

	template<typename It>
	std::list<transition_t> build_transition_path(It begin_state, It end_state)
	{
		std::list<transition_t> res;

		if(begin_state != end_state)
		{
			for(; (begin_state+1) != end_state; ++begin_state)
				res.push_back(this->difference(*begin_state, *(begin_state+1)));
		}

		return res;
	}

	/*void apply(const transition_t & transition)
	{
		_Base::apply(m_state, transition);
	}

	void set_state(const state_t & new_state)
	{
		m_state = new_state;
	}*/

	template<typename F>
	void forall_generated_states(const state_t & base_state, F fun) const
	{
		this->forall_available_transitions(base_state, [&](const transition_t & transition){
			state_t new_state(base_state);
			_Base::apply(new_state, transition);
			fun(new_state, this->transition_cost(base_state, transition));
		});
	}

	template<typename PlanContainerT>
	bool verify_solution(state_t initial_state, const PlanContainerT & path) const
	{
		for (auto tr : path)
			_Base::apply(initial_state, tr);

		//return initial_state == _Base::solved_state();
		return _Base::is_solved(initial_state);
	}

	template<typename StateIt, typename TransIt>
	void trace_solution(std::ostream & os, TransIt trans_first, TransIt trans_last, StateIt state_first)
	{
		int i = 1;
		for (; trans_first != trans_last; ++trans_first, ++state_first, ++i)
		{
			os << "Step #" << i << ':';
			this->interpret_transition(os, *state_first, *trans_first) << std::endl;
			this->interpet_state(os, *state_first) << std::endl;
		}

		os << "Final:" << std::endl;
		this->interpet_state(os, *state_first) << std::endl;
	}

protected:
	//state_t m_state;
	//std::function<transition_t(const state_t &, const state_t &)> m_diffFun;
	//size_description_t m_size;
};

#endif
