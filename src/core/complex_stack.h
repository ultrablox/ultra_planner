

#ifndef UltraCore_complex_stack_h
#define UltraCore_complex_stack_h

#include "streamer.h"
#include <stxxl/stack>
#include <stdexcept>

template<typename T, typename S = base_type_streamer<T>, unsigned int BlockSize = 8192>
class complex_stack
{
	using value_type = T;
	using streamer_t = S;


	struct block_t
	{
		static const unsigned int DataSize = BlockSize - 1 * sizeof(unsigned int);

		block_t()
			:last(0)
		{}

		unsigned int last;
		char data[DataSize];
	};

	typedef typename stxxl::STACK_GENERATOR<block_t>::result base_container_t;

public:
	complex_stack(const streamer_t & streamer)
		:m_streamer(streamer), m_valsPerBlock(block_t::DataSize / streamer.serialized_size())
	{

	}

	complex_stack(const streamer_t & streamer, const value_type & first_val)
		:m_streamer(streamer), m_valsPerBlock(block_t::DataSize / streamer.serialized_size())
	{
		push(first_val);
	}

	void push(const value_type & val)
	{
		if (m_baseContainer.empty() || (m_baseContainer.top().last == m_valsPerBlock))
			m_baseContainer.push(block_t());

		block_t & last_el = m_baseContainer.top();
		m_streamer.serialize(last_el.data + last_el.last * m_streamer.serialized_size(), val);
		++last_el.last;			
	}

	value_type top() const
	{
		if (m_baseContainer.empty())
			throw out_of_range("Queue is empty");

		const block_t & first_el = m_baseContainer.top();

		value_type val;
		m_streamer.deserialize(first_el.data + (first_el.last - 1) * m_streamer.serialized_size(), val);
		return val;
	}

	void pop()
	{
		block_t & first_el = m_baseContainer.top();
		--first_el.last;

		if (first_el.last == 0)
			m_baseContainer.pop();
	}

	bool empty() const
	{
		return m_baseContainer.empty();
	}
private:
	const streamer_t m_streamer;
	base_container_t m_baseContainer;
	size_t m_valsPerBlock;
};

#endif
