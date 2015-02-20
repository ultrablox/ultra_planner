
#ifndef UltraCore_streamer_h
#define UltraCore_streamer_h

class streamer_base
{
public:
	streamer_base(size_t serialized_size = 0)
		:m_serializedSize(serialized_size)
	{}

	size_t serialized_size() const
	{
		return m_serializedSize;
	}
protected:
	size_t m_serializedSize;
};

template<typename T>
class base_type_streamer : public streamer_base
{
public:
	using type_t = T;
	base_type_streamer()
		:streamer_base(sizeof(type_t))
	{
	}

	void serialize(void * dst, const type_t & val) const
	{
		memcpy(dst, &val, m_serializedSize);
	}

	void deserialize(const void * src, type_t & val) const
	{
		memcpy(&val, src, m_serializedSize);
	}

};

#endif
