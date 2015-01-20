
#ifndef UltraCore_complex_vector_h
#define UltraCore_complex_vector_h

#include "utils/helpers.h"
#include <functional>
#include <vector>
#include <type_traits>
#include <stxxl.h>

using namespace std;

template<typename T, bool ExtMemory = false>
class complex_vector
{
	friend class proxy;
	friend class iterator_base;
public:
	typedef complex_vector<T> _Self;
	typedef T value_type;
	typedef int base_type_t;
	typedef std::function<void(const void*, value_type &)> deserialize_fun_type;
	typedef std::function<void(void*, const value_type &)> serialize_fun_type;

	typedef typename stxxl::VECTOR_GENERATOR<base_type_t>::result ext_base_vec_t;
	typedef std::vector<base_type_t> int_ext_base_vec_t;
	typedef typename std::conditional<ExtMemory, ext_base_vec_t, int_ext_base_vec_t>::type base_vec_t;

	template<typename VecT>
	struct iterator_base
	{
		typedef forward_iterator_tag iterator_category;
		typedef T value_type;
		typedef ptrdiff_t difference_type;
		typedef difference_type distance_type;	// retained
		typedef value_type* pointer;
		typedef value_type& reference;


		struct proxy : public value_type
		{
			proxy(void * _ptr, VecT & _vec)
				:m_ptr(_ptr), m_vec(_vec)
			{
				m_vec.m_deserializeFun(m_ptr, *this);
			}

			/*proxy(const proxy && rhs)
				:m_vec(rhs.m_vec)
			{
				memcpy(m_ptr, rhs.m_ptr, m_vec.m_elementSize * sizeof(typename VecT::base_type_t));
			}*/

			proxy& operator=(const value_type &val)
			{
				m_vec.m_serializeFun(m_ptr, val);
				return *this;
			}

			proxy& operator=(const proxy & rhs)
			{
				//memcpy(m_ptr, rhs.m_ptr, m_vec.m_elementSize * sizeof(typename VecT::base_type_t));
				m_vec.m_serializeFun(m_ptr, rhs);
				return *this;
			}

			bool operator==(const value_type & rhs) const
			{
				value_type tmp;
				m_vec.m_deserializeFun(m_ptr, tmp);
				return tmp == rhs;
			}

			bool operator<(const proxy & rhs) const
			{
				value_type v1, v2;
				m_vec.m_deserializeFun(m_ptr, v1);
				rhs.m_vec.m_deserializeFun(rhs.m_ptr, v2);
				return v1 < v2;
			}
		
		private:
			void * m_ptr;
			VecT & m_vec;
		};

		typedef proxy proxy_t;

		iterator_base(size_t _step, size_t _pos, VecT & vec)
			:step(_step), pos(_pos), m_vec(vec)
		{
		}

		proxy operator*()
		{
			return proxy(&m_vec.m_data[pos], m_vec);
		}

		//Prefix
		iterator_base& operator++()
		{
			pos += step;
			return *this;
		}

		//Postfix
		iterator_base operator++(int)
		{
			iterator_base tmp(*this);
			operator++();
			return tmp;
		}

		//Prefix
		iterator_base& operator--()
		{
			pos -= step;
			return *this;
		}

		//Postfix
		iterator_base operator--(int)
		{
			iterator_base tmp(*this);
			operator--();
			return tmp;
		}


		iterator_base& operator=(const iterator_base &rhs)
		{
			this->pos = rhs.pos;
			return *this;
		}

		friend bool operator==(const iterator_base & lhs, const iterator_base & rhs)
		{
			return (lhs.pos == rhs.pos);// && (lhs.step == rhs.step);
		}

		friend bool operator!=(const iterator_base & lhs, const iterator_base & rhs)
		{
			return !(lhs == rhs);
		}

		friend size_t operator-(const iterator_base & lhs, const iterator_base & rhs)
		{
			return (lhs.pos - rhs.pos) / lhs.step;
		}

		friend iterator_base operator-(const iterator_base & lhs, size_t delta)
		{
			return iterator_base(lhs.step, lhs.pos - delta * lhs.step, lhs.m_vec);
		}

		friend iterator_base operator+(const iterator_base & lhs, size_t delta)
		{
			return iterator_base(lhs.step, lhs.pos + delta * lhs.step, lhs.m_vec);
		}

		friend bool operator<(const iterator_base & lhs, const iterator_base & rhs)
		{
			return (lhs.pos < rhs.pos);
		}

		const size_t step;
		size_t pos;
		VecT & m_vec;
	};

	typedef iterator_base<_Self> iterator;
	typedef iterator_base<const _Self> const_iterator;

	template<typename SerFun, typename DesFun>
	complex_vector(int serialized_element_size, SerFun s_fun, DesFun d_fun)
		:m_serializeFun(s_fun), m_deserializeFun(d_fun), m_elementSize(integer_ceil(serialized_element_size, sizeof(base_type_t))), m_valueTmp(serialized_element_size)
	{
	}

	void push_back(const value_type & new_val)
	{
		m_serializeFun(&m_valueTmp[0], new_val);

		for(int i = 0; i < m_elementSize; ++i)
			m_data.push_back(m_valueTmp[i]);

		/*size_t old_size = m_data.size();
		m_data.resize(old_size + m_elementSize);
		for (size_t i = 0; i < m_elementSize; ++i)
			m_data[old_size + i] = m_valueTmp[i];*/
	}

	value_type operator[](size_t index) const
	{
		value_type res;
		m_deserializeFun(&m_data[index * m_elementSize], res);
		return std::move(res);
	}

	iterator begin()
	{
		return iterator(m_elementSize, 0, *this);
	}

	iterator end()
	{
		return iterator(m_elementSize, m_data.size(), *this);
	}

	const_iterator begin() const
	{
		return const_iterator(m_elementSize, 0, *this);
	}

	const_iterator end() const
	{
		return const_iterator(m_elementSize, m_data.size(), *this);
	}

	void insert(iterator iter, const value_type & new_val)
	{
		m_data.insert(m_data.begin() + iter.pos, m_elementSize, 0);
		m_serializeFun(&m_data[iter.pos], new_val);		
	}

	void clear()
	{
		m_data.clear();
	}

	size_t size() const
	{
		return m_data.size() / m_elementSize;
	}

	bool empty() const
	{
		return m_data.empty();
	}
private:
	serialize_fun_type m_serializeFun;
	deserialize_fun_type m_deserializeFun;

	//Number of base elements per one value_type
	const int m_elementSize;

	base_vec_t m_data;

	mutable std::vector<base_type_t> m_valueTmp;
};


#endif
