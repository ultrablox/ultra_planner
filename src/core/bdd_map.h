#ifndef UltraCore_bdd_map_h
#define UltraCore_bdd_map_h

#include "utils/helpers.h"

template<typename V>
class bdd_map
{
	using value_type = V;

	struct node_base_t
	{
		enum class kind_t {value, node};

		node_base_t(kind_t _kind)
			:m_kind(_kind)
		{
		}

		const kind_t m_kind;
	};

	struct node_t : public node_base_t
	{
		node_t(size_t radix_id)
		:node_base_t(kind_t::node), m_pZeroChild(nullptr), m_pOneChild(nullptr), m_radixId(radix_id)
		{
		}

		node_base_t * m_pZeroChild, *m_pOneChild;
		size_t m_radixId;
	};

	struct value_node_t : public node_base_t
	{
		value_node_t(size_t _key, const value_type & val)
		:node_base_t(kind_t::value), m_key(_key), m_value(val)
		{}

		size_t m_key;
		value_type m_value;
	};

public:
	bdd_map()
		:m_radixCount(sizeof(size_t)* 8), m_pRoot(nullptr)
	{
	}

	bool insert(size_t key, const value_type & value)
	{
		return insert_into_node(m_pRoot, key, value);
	}

	void print_node(ostream & os, int offset, const node_base_t * p_node) const
	{
		for (int i = 0; i < offset; ++i)
			os << " .";

		if (p_node->m_kind == node_base_t::kind_t::value)
		{
			auto val_node = static_cast<const value_node_t*>(p_node);
			os << "V{" << val_node->m_key << " => " << val_node->m_value << "}" << std::endl;
		}
		else
		{
			auto node = static_cast<const node_t*>(p_node);
			os << "N[r=" << node->m_radixId << "]" << std::endl;
			print_node(os, offset + 1, node->m_pZeroChild);
			print_node(os, offset + 1, node->m_pOneChild);
		}
			
	}

	friend ostream & operator<<(ostream & os, const bdd_map & bmap)
	{
		os << "BDD Map tree:" << std::endl;
		bmap.print_node(os, 0, bmap.m_pRoot);
		return os;
	}

	void print_node_line(const node_base_t * p_node) const
	{
		if (p_node->m_kind == node_base_t::kind_t::value)
		{
			auto val_node = static_cast<const value_node_t*>(p_node);
			cout << val_node->m_key;
		}
		else
		{
			auto node = static_cast<const node_t*>(p_node);
			print_node_line(node->m_pZeroChild);
			cout << ',';
			print_node_line(node->m_pOneChild);
		}
	}

	void print_line()
	{
		print_node_line(m_pRoot);
	}
private:
	bool insert_into_node(node_base_t *& dest, size_t key, const value_type & value)
	{
		if (dest == nullptr)
		{
			dest = new value_node_t(key, value);
			return true;
		}
		else
		{
			if (dest->m_kind == node_base_t::kind_t::value)
				return insert_into_val_node(dest, key, value);
			else
			{
				auto node_ = static_cast<node_t*>(dest);

				if (check_radix(key, node_->m_radixId))
					return insert_into_node(node_->m_pOneChild, key, value);
				else
					return insert_into_node(node_->m_pZeroChild, key, value);
			}
		}
	}

	bool insert_into_val_node(node_base_t *& dest, size_t key, const value_type & value)
	{
		//Check for equality
		value_node_t * old_val_node = static_cast<value_node_t*>(dest);
		if (old_val_node->m_key == key)
			return false;

		//Create new value node
		value_node_t * new_val_node = new value_node_t(key, value);

		//Find different radix
		auto radix_diff = different_radix(key, old_val_node->m_key);

		//Create node over both
		node_t * new_node = new node_t(radix_diff.first);
		if (radix_diff.second)
		{
			new_node->m_pZeroChild = new_val_node;
			new_node->m_pOneChild = old_val_node;
		}
		else
		{
			new_node->m_pZeroChild = old_val_node;
			new_node->m_pOneChild = new_val_node;
		}

		//Replace old value place with the new node
		dest = new_node;

		return true;
	}
private:
	node_base_t * m_pRoot;
	size_t m_radixCount;
};

#endif
