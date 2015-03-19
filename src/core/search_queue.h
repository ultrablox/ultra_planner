
#ifndef UltraSolver_search_queue_h
#define UltraSolver_search_queue_h

#include <core/containers/complex_vector.h>
#include <core/containers/complex_queue.h>
#include <core/containers/complex_stack.h>
#include <map>
#include <vector>
#include <functional>

#define COMMENT_EVENTS 0

struct node_estimation_t
{
	explicit node_estimation_t(float _path_cost = 0.0f, float _heuristic_estimation = std::numeric_limits<float>::max())
		:path_cost(_path_cost), heuristic_estimation(_heuristic_estimation)
	{}

	/*
	Returns sum of passed path and expected estimation. Usually used
	by optimal algorithms.
	*/
	float total_estimation() const
	{
		return path_cost + heuristic_estimation;
	}

	friend std::ostream & operator<<(std::ostream & os, const node_estimation_t & est)
	{
		os << est.path_cost << '+' << est.heuristic_estimation << '=' << est.total_estimation();
		return os;
	}

	float path_cost;	//Real cost going from initial node to current (in uni graphs - the number of passed transitions/edges)
	float heuristic_estimation;	//Estimation of the left path (based on heuristic)
};

template<typename T, typename C, typename S, bool ExtMemory>
class search_queue
{
public:
	using value_t = T;
	using combined_value_t = std::pair<node_estimation_t, value_t>;
	typedef C comparator_t;
	using value_streamer = S;
	//typedef std::vector<value_t> layer_container_t;
	//typedef std::list<value_t> layer_container_t;
	//using layer_container_t = complex_queue<value_t, value_streamer>;
	
	static const int MaxPrimaryLayers = 100;

	//static_assert(std::is_pod<priority_component_t>::value, "Priority conmonent must be POD");

	class combined_val_streamer_t
	{
	public:
		combined_val_streamer_t(const value_streamer & value_str)
			:m_valStreamer(value_str), m_serializedSize(value_str.serialized_size() + sizeof(node_estimation_t))
		{
		}

		size_t serialized_size() const
		{
			return m_serializedSize;
		}

		void serialize(void * dst, const combined_value_t & val) const
		{
			char * ptr = (char*)dst;
			memcpy(ptr, &val.first, sizeof(node_estimation_t));
			m_valStreamer.serialize(ptr + sizeof(node_estimation_t), val.second);
		}

		void deserialize(const void * src, combined_value_t & val) const
		{
			const char * ptr = (char*)src;
			memcpy(&val.first, ptr, sizeof(node_estimation_t));
			m_valStreamer.deserialize(ptr + sizeof(node_estimation_t), val.second);
		}
	private:
		const value_streamer m_valStreamer;
		size_t m_serializedSize;
	};

	//using layer_container_t = complex_stack<combined_value_t, combined_val_streamer_t, 8192U, ExtMemory>;
	using layer_container_t = complex_queue<combined_value_t, combined_val_streamer_t, 8192U, ExtMemory>;
public:

	search_queue(const value_streamer & node_streamer)
		:m_secondaryData(combined_val_streamer_t(node_streamer)), m_nodeStreamer(node_streamer)
	{

	}

	~search_queue()
	{
		for (auto & layer : m_primaryData)
			delete layer.second;

		m_primaryData.clear();
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

/*	const size_t top(std::vector<value_t> & dest, size_t max_count, bool more_than_one_group = false, priority_component_t * first_node_data = nullptr) const
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

		if (m_primaryData.empty())
			return 0;

		if (first_node_data && (requested != max_count))
			*first_node_data = m_primaryData.begin()->first;

		auto group_it = m_primaryData.begin();
		for (auto it = group_it->second.begin(); (it != group_it->second.end()) && (max_count > 0); ++it, --max_count)
			dest.push_back(*it);

		return requested - max_count;
	}*/

	/*
	Returns true, if new node has the best estimation.
	*/
	bool push(const combined_value_t & val)
	{
		auto group_it = m_primaryData.find(val.first);
		if (group_it != m_primaryData.end())
		{
			group_it->second->push(val);//group_it->second.push_back(val.second);
			return false;
		}
		else
		{
			bool res = m_primaryData.empty() ? true : m_cmp(val.first, m_primaryData.begin()->first);
			m_primaryData.insert(make_pair(val.first, new layer_container_t(m_nodeStreamer, val, "queue_layer_" + to_string(val.first))));

#if COMMENT_EVENTS
			if (res)
			{
				cout << "Added layer with best estimation: " << val.first << std::endl;
			}
#endif
			return res;

			/*else if (m_primaryData.size() < MaxPrimaryLayers)
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
			}*/
		}
	}

	/*void pop()
	{
		if (m_primaryData.empty())
			extract_secondary_data();

		
		auto top_it = m_primaryData.begin();
		//top_it->second.pop_back();
		//top_it->second.erase(top_it->second.begin());
		top_it->second->pop();

		if (top_it->second->empty())
		{
			delete top_it->second;
			m_primaryData.erase(top_it);
		}
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
		}*/

/*		auto group_it = m_primaryData.begin();
		if (group_it->second.size() <= count)
			m_primaryData.erase(group_it);
		else
		{
			auto it = group_it->second.begin();
			std::advance(it, count);
			group_it->second.erase(group_it->second.begin(), it);
		}
	}*/

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

	node_estimation_t best_estimation() const
	{
		if (m_primaryData.empty())
			return node_estimation_t();
		else
		{
			auto it = m_primaryData.begin();
			return it->first;
		}
	}

	template<typename Fun>
	int pop_and_call(Fun fun, int max_count = 1)
	{
		if (m_primaryData.empty())
			return 0;

		int poped_count = 0;
		auto it = m_primaryData.begin();
		while ((!it->second->empty()) && (poped_count < max_count))
		{
			fun(it->second->top().first, it->second->top().second);
			it->second->pop();
			++poped_count;
		}

		if (it->second->empty())
		{
#if COMMENT_EVENTS
			cout << "Layer with estimation " << it->first << " expanded" << std::endl;
#endif
			delete it->second;
			m_primaryData.erase(it);
		}

		return poped_count;
	}
private:
	void extract_secondary_data()
	{
		//Extract best nodes from secondary data
		throw runtime_error("Not implemented");
	}
private:
	std::map<node_estimation_t, layer_container_t*, comparator_t> m_primaryData;
	complex_vector<combined_value_t, combined_val_streamer_t> m_secondaryData;
	const value_streamer m_nodeStreamer;
	comparator_t m_cmp;
};

#endif
