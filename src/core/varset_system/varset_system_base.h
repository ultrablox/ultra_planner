
#ifndef UltraCore_varset_system_base_h
#define UltraCore_varset_system_base_h

#include <string>
#include <vector>

template<typename T>
class varset_transition_base : public T
{
	using _Base = T;
public:
	template<typename A, typename B, typename C, typename D>
	varset_transition_base(const A & a, const B & b, const C & c, const D & d)
		:_Base(a, b, c, d)
	{}

	template<typename A, typename B>
	varset_transition_base(const A & a, const B & b)
		: _Base(a, b)
	{}

	template<typename A>
	varset_transition_base(const A & a)
		: _Base(a)
	{}

	varset_transition_base()
	{}

	varset_transition_base(const varset_transition_base & rhs)
		: _Base(rhs), name(rhs.name)
	{
	}

	friend bool operator==(const varset_transition_base & lhs, const varset_transition_base & rhs)
	{
		return (lhs.name == rhs.name) && (static_cast<const _Base&>(lhs) == static_cast<const _Base&>(rhs));
	}

	friend bool operator<(const varset_transition_base & lhs, const varset_transition_base & rhs)
	{
		return lhs.name < rhs.name;
	}

	friend std::ostream & operator<<(std::ostream & os, const varset_transition_base & transition)
	{
		os << transition.name;
		return os;
	}

	std::string name;
};

template<typename T>
class varset_system_base : public T
{
	using _Base = T;
public:
	using transition_t = typename _Base::transition_t;
	using state_t = typename _Base::state_t;
	using masked_state_t = typename _Base::masked_state_t;

	template<typename... Params>
	varset_system_base(Params... args)
		:_Base(args...)
	{}

	/*template<typename Param>
	varset_system_base(Param arg)
		: _Base(arg)
	{}*/

	template<typename... Params>
	void add_transition(Params... args)
	{
		add_transition(transition_t(args...));
	}

	void add_transition(const transition_t & trans)
	{
		m_transitions.push_back(trans);
	}

	template<typename F>
	void forall_available_transitions(const state_t & base_state, F fun) const
	{
		for (auto & transition : m_transitions)
		{
			if (_Base::transition_available(base_state, transition))
				fun(transition);
		}
	}

	std::ostream & interpret_transition(std::ostream & os, const state_t & state, const transition_t & transition) const
	{
		os << transition.name;
		return os;
	}

	const std::vector<transition_t> & transitions() const
	{
		return m_transitions;
	}

	std::vector<transition_t> & transitions()
	{
		return m_transitions;
	}

	void to_relaxed()
	{
		for (auto & tr : m_transitions)
			tr.to_relaxed();
	}

	/*template<typename Params>
	transition_t create_transition(const std::initializer_list<Params> & args)
	{
		transition_t new_trans = _Base::create_transition(args);
		m_transitions.push_back(new_trans);
		return new_trans;
	}*/
protected:
	std::vector<transition_t> m_transitions;
	
};

#endif
