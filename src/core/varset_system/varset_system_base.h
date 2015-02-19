
#ifndef UltraCore_varset_system_base_h
#define UltraCore_varset_system_base_h

template<typename T>
class varset_transition_base : public T
{
	using _Base = T;
public:
	template<typename... Params>
	varset_transition_base(const std::string & _name, Params... args)
		:_Base(args...), name(_name)
	{}

	varset_transition_base()
	{}

	std::string name;
};

template<typename T>
class varset_system_base : public T
{
	using _Base = T;
public:
	using transition_t = typename _Base::transition_t;
	using state_t = typename _Base::state_t;

	template<typename... Params>
	varset_system_base(Params... args)
		:_Base(args...)
	{}

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
			if (transition_available(base_state, transition))
				fun(transition);
		}
	}
protected:
	std::vector<transition_t> m_transitions;
};

#endif
