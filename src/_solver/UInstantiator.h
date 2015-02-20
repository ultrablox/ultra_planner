
#ifndef UltraPlanner_UInstantiator_h
#define UltraPlanner_UInstantiator_h

#include <vector>
#include <map>
#include <pddl/UPDDLParametrizedObject.h>

//using namespace std;

class UInstantiator
{
public:
	typedef std::map<const UPDDLType *, std::vector<UPDDLObject*>> TypesIndex;

	template<typename T>
	void bruteforceInstances(const TypesIndex & type_index, std::vector<T> & target_array, T new_instance, const int param_index, size_t & current_index) const
	{
		if(param_index <  new_instance.object->parameters.size())
		{
			auto current_param_type = new_instance.object->parameter(param_index)->type;

			auto type_group = type_index.find(current_param_type);
			for(auto obj : type_group->second)
			{
				new_instance.params[param_index] = obj;
				bruteforceInstances<T>(type_index, target_array, new_instance, param_index + 1, current_index);
			}
		}
		else
		{
			//target_array.insert(target_array.end(), new_instance);
			target_array[current_index++] = new_instance;
		}
	}

	/*size_t calculateDestinationContainerSize()
	{

	}*/

	template<typename SourceType, typename InstanceType>
	void instantiateTypedObjects(const TypesIndex & types_index, const std::vector<SourceType*> & object_pool, std::vector<InstanceType> & destination_pool) const
	{
		//Calculate total to set destination vector size
		size_t sum = 0;
		for(auto intance_object : object_pool)
		{
			size_t obj_sum = 1;
			for(auto param : intance_object->parameters)
			{
				//Get count of objects of given type
				auto type_group = types_index.find(param->type);
				obj_sum *= type_group->second.size();
			}

			sum += obj_sum;
		}

		destination_pool.resize(sum);

		size_t current_index = 0;

		//Create instances
		for(auto intance_object : object_pool)
		{
			InstanceType instance;
			instance.object = intance_object;
			instance.params.resize(intance_object->parameters.size());
			bruteforceInstances<InstanceType>(types_index, destination_pool, instance, 0, current_index);
		}
	}
};

#endif
