

#ifndef UltraCore_transition_system_h
#define UltraCore_transition_system_h

#include <functional>
#include <list>
#include <utility>

template<typename T>
class transition_system : public T
{
public:
	typedef T _Base;
	typedef typename _Base::state_t state_t;
	typedef typename _Base::transition_t transition_t;

	/*transition_system(const state_t & _state)
		:_Base(m_state), m_state(_state)
	{
	}*/

	template<typename Args>
	transition_system(Args descr)
		:_Base(m_state, descr)
	{
		m_state = _Base::default_state(descr);
	}

	transition_system(const transition_system & rhs)
		:_Base(m_state, rhs.m_size), m_state(rhs.m_state)
	{
	}

	transition_system & operator=(const transition_system & rhs)
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
	}

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

	template<typename It>
	std::list<transition_t> build_transition_path(It begin_state, It end_state)
	{
		std::list<transition_t> res;

		if(begin_state != end_state)
		{
			for(; (begin_state+1) != end_state; ++begin_state)
				res.push_back(difference(*begin_state, *(begin_state+1)));
		}

		return res;
	}

	bool is_solved() const
	{
		return _Base::is_solved(m_state);
	}

	void apply(const transition_t & transition)
	{
		_Base::apply(m_state, transition);
	}

	void set_state(const state_t & new_state)
	{
		m_state = new_state;
	}

	template<typename F>
	void forall_generated_states(const state_t & base_state, F fun) const
	{
		forall_available_transitions(base_state, [&](transition_t transition){
			state_t new_state(base_state);
			_Base::apply(new_state, transition);
			fun(new_state);
		});
	}

protected:
	state_t m_state;
	//std::function<transition_t(const state_t &, const state_t &)> m_diffFun;
};

#endif
