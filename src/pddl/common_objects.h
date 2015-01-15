#ifndef UltraPlanner_common_objects_h
#define UltraPlanner_common_objects_h

#include <vector>
#include <iostream>
#include <sstream>

struct UPDDLObject;

template<typename T>
struct UPDDLTypedInstance
{
//	using ObjectType = T;
	UPDDLTypedInstance(T * _object = nullptr, const std::vector<UPDDLObject*> & _params = std::vector<UPDDLObject*>())
		:object(_object), params(_params)
	{
	}

	void print(std::ostream & os = std::cout, bool new_line = true) const
	{
		//object->print(os);
		os << object->name << " ";

		for(auto param : params)
			os << param->name << " ";

		if(new_line)
			os << "\n";
	}
	friend bool operator==(const UPDDLTypedInstance & ti1, const UPDDLTypedInstance & ti2)
	{
		return (ti1.object == ti2.object) && (ti1.params == ti2.params);
	}

	std::vector<UPDDLObject*> params;
	T * object;
};

template<typename T>
const std::string to_string(const UPDDLTypedInstance<T> & ti)
{
    std::stringstream ss;
	ti.print(ss, false);

	//std::string res;
	//ss >> res;
	return ss.str();
}

#endif
