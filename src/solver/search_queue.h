
#ifndef UltraSolver_search_queue_h
#define UltraSolver_search_queue_h

#include <core/complex_vector.h>
#include <map>
#include <vector>
#include <functional>

template<typename T, typename C>
class search_queue
{
	typedef T combined_value_t;
	typedef typename combined_value_t::first_type priority_component_t;
	typedef typename combined_value_t::second_type value_t;
	typedef C comparator_t;
	static const int MaxPrimaryLayers = 10;

	//static_assert(std::is_pod<priority_component_t>::value, "Priority conmonent must be POD");
public:

	search_queue(int serialized_value_size, std::function<void(char *, const value_t &)> ser_fun, std::function<void(const char *, value_t &)> des_fun)
		:m_secondaryData(serialized_value_size + sizeof(priority_component_t), 
			[=](void * dst, const combined_value_t & val){
				char * ptr = (char*)dst;
				memcpy(ptr, &val.first, sizeof(combined_value_t));
				ser_fun(ptr + sizeof(combined_value_t), val.second);
			},
			[=](const void * src, combined_value_t & val){
				const char * ptr = (char*)src;
				memcpy(&val.first, ptr, sizeof(combined_value_t));
				des_fun(ptr + sizeof(combined_value_t), val.second);
			}
		)
	{

	}

	bool empty() const
	{
		return m_primaryData.empty();
	}

	const combined_value_t top()
	{
		if (m_primaryData.empty())
			extract_secondary_data();

		auto it = m_primaryData.begin();
		return combined_value_t(it->first, *it->second.rbegin());
	}

	const size_t top(std::vector<value_t> & dest, size_t max_count, bool more_than_one_group = false) const
	{
		size_t requested = max_count;
		auto group_it = m_primaryData.begin();
		while ((group_it != m_primaryData.end()) && (max_count > 0))
		{
			auto last_it = group_it->second.size() < max_count ? group_it->second.rend() : group_it->second.rbegin() + max_count;

			for (auto it = group_it->second.rbegin(); it != last_it; ++it, --max_count)
				dest.push_back(*it);

			if (more_than_one_group)
				++group_it;
			else
				break;
		}

		return requested - max_count;
	}

	/*
	Returns true, if some layer was moved to secondary. The
	second element contains the minimum value of primary nodes
	priority.
	*/
	std::pair<bool, priority_component_t> push(const combined_value_t & val)
	{
		auto group_it = m_primaryData.find(val.first);
		if (group_it != m_primaryData.end())
			group_it->second.push_back(val.second);
		else
		{
			if (m_primaryData.empty())
				m_primaryData.insert(make_pair(val.first, std::vector<value_t>(1, val.second)));
			else if (m_primaryData.size() < MaxPrimaryLayers)
				m_primaryData.insert(make_pair(val.first, std::vector<value_t>(1, val.second)));
			else if (val.first < m_primaryData.rbegin()->first)
			{
				m_primaryData.insert(make_pair(val.first, std::vector<value_t>(1, val.second)));
				
				if (m_primaryData.size() > MaxPrimaryLayers)
				{
					//Compress last layer into secondary data
					auto rit = m_primaryData.rbegin();

					cout << "Erasing search queue layer (" << rit->second.size() << " nodes)..." << std::endl;

					for (auto & node : rit->second)
						m_secondaryData.push_back(combined_value_t(rit->first, std::move(node)));


					std::advance(rit, 1);
					auto it = rit.base();
					m_primaryData.erase(it);

					return make_pair(true, m_primaryData.rbegin()->first);
				}
			}
			else
			{
				//cout << "Must queue secondary data..." << std::endl;
				//throw runtime_error("Not implemented");
				m_secondaryData.push_back(val);
			}
		}
		
		return make_pair(false, priority_component_t());
	}

	void pop()
	{
		if (m_primaryData.empty())
			extract_secondary_data();

		
		auto top_it = m_primaryData.begin();
		top_it->second.pop_back();
		//top_it->second.erase(top_it->second.begin());

		if (top_it->second.empty())
			m_primaryData.erase(top_it);
	}

	void pop(size_t count)
	{
		auto it = m_primaryData.begin();
		while ((!m_primaryData.empty()) && (count > 0))
		{
			if (it->second.size() <= count)
			{
				count -= it->second.size();
				m_primaryData.erase(it);
				it = m_primaryData.begin();
			}
			else
			{
				auto last_it = it->second.rbegin();
				std::advance(last_it, count);
				it->second.erase(last_it.base(), it->second.end());
			}
		}
	}

	int layer_count() const
	{
		return m_primaryData.size();
	}

	size_t secondary_nodes_count() const
	{
		return m_secondaryData.size();
	}
private:
	void extract_secondary_data()
	{
		//Extract best nodes from secondary data
		throw runtime_error("Not implemented");
	}
private:
	std::map<priority_component_t, std::vector<value_t>, comparator_t> m_primaryData;
	complex_vector<combined_value_t> m_secondaryData;
};

#endif
