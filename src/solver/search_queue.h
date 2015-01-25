
#ifndef UltraSolver_search_queue_h
#define UltraSolver_search_queue_h

#include <core/complex_vector.h>
#include <core/complex_queue.h>
#include <map>
#include <vector>
#include <functional>

template<typename T, typename C, typename S>
class search_queue
{
	typedef T combined_value_t;
	typedef typename combined_value_t::first_type priority_component_t;
	typedef typename combined_value_t::second_type value_t;
	typedef C comparator_t;
	using value_streamer = S;
	//typedef std::vector<value_t> layer_container_t;
	//typedef std::list<value_t> layer_container_t;
	using layer_container_t = complex_queue<value_t, value_streamer>;
	static const int MaxPrimaryLayers = 3;

	//static_assert(std::is_pod<priority_component_t>::value, "Priority conmonent must be POD");

	class combined_val_streamer_t
	{
	public:
		combined_val_streamer_t(const value_streamer & value_str)
			:m_valStreamer(value_str), m_serializedSize(value_str.serialized_size() + sizeof(priority_component_t))
		{
		}

		size_t serialized_size() const
		{
			return m_serializedSize;
		}

		void serialize(void * dst, const combined_value_t & val) const
		{
			char * ptr = (char*)dst;
			memcpy(ptr, &val.first, sizeof(combined_value_t));
			m_valStreamer.serialize(ptr + sizeof(combined_value_t), val.second);
		}

		void deserialize(const void * src, combined_value_t & val)
		{
			const char * ptr = (char*)src;
			memcpy(&val.first, ptr, sizeof(combined_value_t));
			m_valStreamer.deserialize(ptr + sizeof(combined_value_t), val.second);
		}
	private:
		const value_streamer m_valStreamer;
		size_t m_serializedSize;
	};
public:

	search_queue(const value_streamer & node_streamer)
		:m_secondaryData(combined_val_streamer_t(node_streamer)), m_nodeStreamer(node_streamer)
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
		//return combined_value_t(it->first, *it->second.begin());
		return combined_value_t(it->first, it->second->top());
	}

	const size_t top(std::vector<value_t> & dest, size_t max_count, bool more_than_one_group = false, priority_component_t * first_node_data = nullptr) const
	{
		size_t requested = max_count;
		/*auto group_it = m_primaryData.begin();
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
*/
/*		if (m_primaryData.empty())
			return 0;

		if (first_node_data && (requested != max_count))
			*first_node_data = m_primaryData.begin()->first;

		auto group_it = m_primaryData.begin();
		for (auto it = group_it->second.begin(); (it != group_it->second.end()) && (max_count > 0); ++it, --max_count)
			dest.push_back(*it);*/

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
			group_it->second->push(val.second);//group_it->second.push_back(val.second);
		else
		{
			if (m_primaryData.empty())
			{
				//m_primaryData.insert(make_pair(val.first, layer_container_t(1, val.second)));
				m_primaryData.insert(make_pair(val.first, new layer_container_t(m_nodeStreamer, val.second)));
			}
			else if (m_primaryData.size() < MaxPrimaryLayers)
				m_primaryData.insert(make_pair(val.first, new layer_container_t(m_nodeStreamer, val.second)));//m_primaryData.insert(make_pair(val.first, layer_container_t(1, val.second)));
			else if (val.first < m_primaryData.rbegin()->first)
			{
				//m_primaryData.insert(make_pair(val.first, layer_container_t(1, val.second)));
				m_primaryData.insert(make_pair(val.first, new layer_container_t(m_nodeStreamer, val.second)));
				
				if (m_primaryData.size() > MaxPrimaryLayers)
				{
					//Compress last layer into secondary data
					auto rit = m_primaryData.rbegin();

					//cout << "Erasing search queue layer (" << rit->second.size() << " nodes)..." << std::endl;

					//for (auto & node : rit->second)
					//	m_secondaryData.push_back(combined_value_t(rit->first, std::move(node)));


					std::advance(rit, 1);
					auto it = rit.base();
					delete it->second;
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
		//top_it->second.pop_back();
		//top_it->second.erase(top_it->second.begin());
		top_it->second->pop();

		if (top_it->second->empty())
			m_primaryData.erase(top_it);
	}

	void pop(size_t count)
	{
		/*auto it = m_primaryData.begin();
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
		}*/

/*		auto group_it = m_primaryData.begin();
		if (group_it->second.size() <= count)
			m_primaryData.erase(group_it);
		else
		{
			auto it = group_it->second.begin();
			std::advance(it, count);
			group_it->second.erase(group_it->second.begin(), it);
		}*/
	}

	int layer_count() const
	{
		return m_primaryData.size();
	}

	size_t secondary_nodes_count() const
	{
		return m_secondaryData.size();
	}

	size_t top_layer_size() const
	{
		if (m_primaryData.empty())
			return 0;
		else
			return m_primaryData.begin()->second.size();
	}
private:
	void extract_secondary_data()
	{
		//Extract best nodes from secondary data
		throw runtime_error("Not implemented");
	}
private:
	std::map<priority_component_t, layer_container_t*, comparator_t> m_primaryData;
	complex_vector<combined_value_t, combined_val_streamer_t> m_secondaryData;
	const value_streamer m_nodeStreamer;
};

#endif
