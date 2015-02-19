
#ifndef UltraCore_varset_system_base_h
#define UltraCore_varset_system_base_h

template<typename T>
class varset_system_base
{
public:
	using transition_t = T;

	varset_system_base(int _size)
		:m_size(_size)
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
protected:
	std::vector<transition_t> m_transitions;
	int m_size;
};

#endif
