

#ifndef UltraCore_complex_queue_h
#define UltraCore_complex_queue_h

#include "streamer.h"
#include <stxxl/queue>
#include <stdexcept>

template<typename T, typename S = base_type_streamer<T>, unsigned int BlockSize = 8192, bool ExtMem = false>
class complex_queue
{
	using value_type = T;
	using streamer_t = S;


	struct block_t
	{
		static const unsigned int DataSize = BlockSize - 2 * sizeof(unsigned int);

		block_t()
			:first(0), last(0)
		{}

		unsigned int first, last;
		char data[DataSize];
	};


	using base_container_t = typename std::conditional<ExtMem, stxxl::queue<block_t>, std::queue<block_t>>::type;

public:
	complex_queue(const streamer_t & streamer)
		:m_streamer(streamer), m_valsPerBlock(block_t::DataSize / streamer.serialized_size()), m_size(0)
	{

	}

	complex_queue(const streamer_t & streamer, const value_type & first_val)
		:m_streamer(streamer), m_valsPerBlock(block_t::DataSize / streamer.serialized_size()), m_size(0)
	{
		push(first_val);
	}

	void push(const value_type & val)
	{
		if (m_baseContainer.empty())
			m_baseContainer.push(block_t());
		else
		{
			block_t & last_el = m_baseContainer.back();
			if (last_el.last == m_valsPerBlock)
				m_baseContainer.push(block_t());
		}
		
		block_t & last_el = m_baseContainer.back();
		m_streamer.serialize(last_el.data + last_el.last * m_streamer.serialized_size(), val);
		++last_el.last;
		++m_size;
	}

	value_type top() const
	{
#if _DEBUG
		assert(m_size > 0);
#endif

		const block_t & first_el = m_baseContainer.front();

		value_type val;
		m_streamer.deserialize(first_el.data + first_el.first * m_streamer.serialized_size(), val);
		return val;
	}

	void pop()
	{
#if _DEBUG
		assert(m_size > 0);
#endif
		block_t & first_el = m_baseContainer.front();
		++first_el.first;

		if (first_el.first == first_el.last)
			m_baseContainer.pop();
		
		--m_size;
	}

	bool empty() const
	{
		//return m_baseContainer.empty();
		return (m_size == 0);
	}
private:
	const streamer_t m_streamer;
	base_container_t m_baseContainer;
	size_t m_valsPerBlock;
	size_t m_size;
};

#endif
