
#ifndef UltraCore_block_h
#define UltraCore_block_h

#include <limits>

template<unsigned int S>
struct hashset_block
{
	static const size_t Size = S;

	hashset_block(size_t _id = std::numeric_limits<size_t>::max())
	{
		meta.item_count = 0;
		meta.id = _id;
		meta.next = _id;
		meta.prev = _id;

		memset(data, 0, DataSize);
	}

	size_t first_hash() const
	{
		return *((const size_t*)data);
	}

	size_t last_hash(size_t record_size) const
	{
		const char * ptr = data + record_size * (item_count() - 1);
		return *((const size_t*)ptr);
	}

	void set_meta(size_t _id, size_t _prev, size_t _next, int _item_count)
	{
		meta.id = _id;
		meta.prev = _prev;
		set_next(_next);
		set_item_count(_item_count);
	}

	void inc_item_count(int delta = 1)
	{
		//m_itemCount += delta;
		set_item_count(meta.item_count + delta);
	}

	int item_count() const
	{
		return meta.item_count;
	}

	void set_item_count(int new_count)
	{
		//if (new_count > 5)
		//	throw out_of_range("!");
		meta.item_count = new_count;
	}

	void set_next(size_t new_next)
	{
		//cout << "Setting next to " << new_next << std::endl;
		meta.next = new_next;
	}

	size_t next() const
	{
		return meta.next;
	}

	struct meta_t
	{
		size_t id, prev, next;
		int item_count;
	};

	meta_t meta;

	static const size_t DataSize = Size - sizeof(meta_t);
	char data[DataSize];
};

#endif
