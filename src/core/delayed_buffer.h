
#ifndef UltraCore_delayed_buffer_h
#define UltraCore_delayed_buffer_h

#include <vector>
#include <unordered_map>

template<typename T>
struct insertion_request
{
	using value_type = T;

	using callback_t = std::function<void(const value_type & val)>;

	insertion_request()
	{}

	insertion_request(size_t _hash_val, const value_type & _val, callback_t _callback, size_t serialized_element_size)
		:val(_val), callback(_callback), hash_val(_hash_val), serialized_val(serialized_element_size)
	{}

	byte_range to_byte_range() const
	{
		return byte_range((char*)&serialized_val[0], serialized_val.size());
	}

	size_t hash_val;
	value_type val;
	callback_t callback;
	std::vector<char> serialized_val;
};

template<typename T>
class delayed_buffer
{
	using value_type = T;
	using insertion_request_t = insertion_request<T>;
public:
	

	struct request_cmp_t
	{
		bool operator()(const insertion_request_t & lhs, const insertion_request_t & rhs) const
		{
			return lhs.val == rhs.val;
		}
	};

	using groupt_t = std::vector<insertion_request_t>;

	delayed_buffer(int expected_group_size)
		:m_size(0), m_expectedGroupSize(expected_group_size)
	{}

	template<typename Fun>
	void for_each_range(Fun fun)
	{
		for (auto & group : m_data)
			fun(group.first, group.second);
	}

	void clear()
	{
		m_data.clear();
		m_size = 0;
	}

	void erase(size_t key)
	{
		auto it = m_data.find(key);
		m_size -= it->second.size();
		m_data.erase(it);
	}

	size_t insert(size_t key, const insertion_request_t & new_req)
	{
		++m_size;
		auto it = m_data.find(key);
		if (it == m_data.end())
		{
			groupt_t new_group(1, new_req);
			new_group.reserve(m_expectedGroupSize);
			
			m_data.insert(make_pair(key, std::move(new_group)));
			return 1;
		}
		else
		{
			it->second.push_back(new_req);
			return it->second.size();
		}
	}

	groupt_t & operator[](size_t key)
	{
		return m_data.find(key)->second;
	}


	size_t size() const
	{
		return m_size;
		/*size_t result = 0;
		for (auto & el : m_data)
			result += el.second.size();
		return result;*/
	}

	int range_count() const
	{
		return m_data.size();
	}
private:
	std::unordered_map<size_t, groupt_t> m_data;
	size_t m_size;
	int m_expectedGroupSize;
};
#endif
