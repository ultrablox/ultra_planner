
#ifndef UltraCore_floatvar_system_h
#define UltraCore_floatvar_system_h

struct floatvar_transition
{

};

class floatvar_system : public varset_system_base<boolvar_transition>
{
	using _Base = varset_system_base<boolvar_transition>;
public:
	using state_t = std::vector<float>;
	using transition_t = floatvar_transition;

	floatvar_system(int var_count)
		:_Base(var_count)
	{}
		
};

#endif
