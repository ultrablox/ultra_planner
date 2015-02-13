
#ifndef UltraCore_avl_tree_h
#define UltraCore_avl_tree_h

#include <functional>
#include <ostream>
#include <iostream>
#include <algorithm>

template<typename T, typename S, typename C = std::less<T>, typename E = std::equal_to<T>>
class binary_tree
{
protected:
	using key_t = T;
	using satelite_data_t = S;
	using compare_t = C;
	using equal_t = E;
	
	struct node_t
	{
		node_t(key_t _key = key_t(), node_t * _parent = nullptr)
			:key(_key), left(nullptr), right(nullptr), parent(_parent), height(0), prev(nullptr), next(nullptr)
		{}

		~node_t()
		{
			delete left;
			delete right;
		}

		key_t key;

		typename std::enable_if<!std::is_void<satelite_data_t>::value, satelite_data_t>::type data;
		node_t *left, *right, *parent, *prev, *next;
		int height;
	};

public:
	struct iterator
	{
		friend class binary_tree;
		//friend class avl_tree;
	public:
		iterator()
			:p_node(nullptr)
		{}

		key_t & operator*()
		{
			return p_node->key;
		}

		iterator& operator++()
		{
			p_node = p_node->next;
			return *this;
		}

		iterator operator+(int delta)
		{
			node_t * nd = p_node;
			while (delta > 0)
			{
				nd = nd->next;
				--delta;
			}

			return iterator(nd);
		}

		satelite_data_t & data()
		{
			return p_node->data;
		}
	
		friend bool operator!=(const iterator & lhs, const iterator & rhs)
		{
			return (lhs.p_node != rhs.p_node);
		}

		friend bool operator==(const iterator & lhs, const iterator & rhs)
		{
			return (lhs.p_node == rhs.p_node);
		}

		node_t * node() const
		{
			return p_node;
		}
	private:
		iterator(node_t * _node)
			:p_node(_node)
		{}

		node_t * p_node;
	};

	binary_tree()
		:m_pRoot(nullptr), m_pFirst(nullptr)
	{
	}

	virtual ~binary_tree()
	{
		delete m_pRoot;
	}

	iterator find(const key_t & val) const
	{
		return iterator(find_node(m_pRoot, val));
	}

	iterator lower_bound(const key_t & val) const
	{
		return iterator(lower_bound(m_pRoot, val));
	}

	iterator begin() const
	{
		return iterator(m_pFirst);
	}

	iterator end() const
	{
		return iterator(nullptr);
	}

	std::pair<iterator, bool> insert(const key_t & val)
	{
		bool first_node = (m_pRoot == nullptr);

		node_t * res_node = insert_to_node(nullptr, m_pRoot, val);

		if (first_node)
			m_pFirst = res_node;
		
		//Update heights
		update_height(res_node);

		return std::make_pair(iterator(res_node), res_node != nullptr);
	}

	std::pair<iterator, bool> insert(const std::pair<key_t, satelite_data_t> & val)
	{
		auto res = insert(val.first);
		if (res.second)
			res.first.data() = val.second;
		return res;
	}

	friend std::ostream & operator<<(std::ostream & os, const binary_tree & bin_tree)
	{
		os << "Binary tree:" << std::endl;
		bin_tree.print_node(os, 0, bin_tree.m_pRoot);
		return os;
	}

	size_t size() const
	{
		return subtree_size(m_pRoot);
	}

	bool validate_height() const
	{
		return validate_node_height(m_pRoot, m_pRoot->height);
	}

	template<typename Fun>
	void for_each(Fun fun) const
	{
		//for_each_node(m_pRoot, fun);
		for (auto it = begin(); it != end(); ++it)
			fun(it.p_node->data);
	}

private:
	node_t * insert_to_node(node_t * p_parent, node_t *& dest, const key_t & val)
	{
		if (dest == nullptr)
		{
			dest = new node_t(val, p_parent);
			//Build next/prev links
			if (p_parent != nullptr)
			{
				if (m_cmp(val, p_parent->key))//Insertion to the left
				{
					dest->next = p_parent;
					dest->prev = p_parent->prev;
					if (p_parent->prev)
						p_parent->prev->next = dest;
					else
						m_pFirst = dest;
					p_parent->prev = dest;
				}
				else//Insertion to the right
				{
					dest->next = p_parent->next;
					dest->prev = p_parent;
					if (p_parent->next)
						p_parent->next->prev = dest;
					p_parent->next = dest;
				}

			}
			return dest;
		}
		else
		{
			if (dest->key == val)
				return nullptr;
			else
				return insert_to_node(dest, m_cmp(val, dest->key) ? dest->left : dest->right, val);
		}
	}

	node_t * find_node(node_t * base_node, const key_t & val) const
	{
		if (base_node == nullptr)
			return nullptr;
		else
		{
			if (base_node->key == val)
				return base_node;
			else
				return find_node(m_cmp(val, base_node->key) ? base_node->left : base_node->right, val);
		}
	}

	node_t * lower_bound(node_t * base_node, const key_t & val) const
	{
		/*if (base_node == nullptr)
			return nullptr;

		if (base_node->key == val)
			return base_node;
		else
		{
			node_t * child_lower_bound = lower_bound(m_cmp(val, base_node->key) ? base_node->left : base_node->right, val);
			if (child_lower_bound == nullptr)
			{
				if (m_cmp(base_node->key, val))
					return base_node;
			}
			
			return child_lower_bound;
		}*/
		node_t * last_node = base_node;
		while (base_node)
		{
			last_node = base_node;
			base_node = m_cmp(val, base_node->key) ? base_node->left : base_node->right;
		}

		return m_cmp(val, last_node->key) ? last_node->prev : last_node;
	}


	void print_node(std::ostream & os, int offset, const node_t * p_node) const
	{
		for (int i = 0; i < offset; ++i)
			os << " .";

		if (p_node)
		{
			os << "{" << p_node->key << ", h=" << p_node->height << "}" << std::endl;

			if (p_node->left || p_node->right)
			{
				print_node(os, offset + 1, p_node->left);
				print_node(os, offset + 1, p_node->right);
			}
		}
		else
			std::cout << "#" << std::endl;
	}

	size_t subtree_size(node_t * p_node) const
	{
		return p_node ? 1 + subtree_size(p_node->left) + subtree_size(p_node->right) : 0;
	}

	bool validate_node_height(node_t * p_node, int expected_height) const
	{
		bool left_val = p_node->left ? validate_node_height(p_node->left, p_node->left->height) : true,
			right_val = p_node->right ? validate_node_height(p_node->right, p_node->right->height) : true;
		return left_val && right_val && (calculate_node_height(p_node) == expected_height);
	}

	int calculate_node_height(node_t * p_node) const
	{
		if (!p_node)
			return 0;

		int left_h = 0, right_h = 0;
		if (p_node->left)
			left_h = calculate_node_height(p_node->left) + 1;

		if (p_node->right)
			right_h = calculate_node_height(p_node->right) + 1;

		return std::max(left_h, right_h);
	}

	template<typename Fun>
	void for_each_node(node_t * p_node, Fun fun) const
	{
		if (p_node)
		{
			fun(p_node->data);
			for_each_node(p_node->left, fun);
			for_each_node(p_node->right, fun);
		}
	}
protected:
	void update_height(node_t * p_node)
	{
		while (p_node)
		{
			p_node->height = std::max(subtree_height(p_node->left), subtree_height(p_node->right));
			p_node = p_node->parent;
		}
	}

	int subtree_height(const node_t * p_node) const
	{
		return p_node ? p_node->height + 1 : 0;
	}

	int node_height(const node_t * p_node) const
	{
		return p_node ? p_node->height : 0;
	}

protected:
	node_t * m_pRoot, *m_pFirst;
	compare_t m_cmp;
	equal_t m_eq;
};


/*
Simpliest AVL-tree base on 
http://www.cs.unb.ca/tech-reports/documents/TR95_100.pdf
*/
template<typename T, typename S, typename C = std::less<T>, typename E = std::equal_to<T>>
class avl_tree : public binary_tree<T, S, C, E>
{
	using _Base = binary_tree<T, S, C, E>;
	using key_t = typename _Base::key_t;
	using satelite_data_t = typename _Base::satelite_data_t;
	using node_t = typename _Base::node_t;
public:
	using iterator = typename _Base::iterator;


	std::pair<iterator, bool> insert(const std::pair<key_t, satelite_data_t> & val)
	{
		auto res = _Base::insert(val);
		if (res.second)
		{
			//Ballance
			for (node_t * n = res.first.node(); n != nullptr; n = n->parent)
			{
				int left_height = _Base::subtree_height(n->left),
					right_height = _Base::subtree_height(n->right);

				if (abs(left_height - right_height) > 1)
				{
					node_t * par = n->parent;
					node_t * ballanced_root = ballance_node(n);

					if (par)
					{
						if (par->left == n)
							par->left = ballanced_root;
						else if (par->right == n)
							par->right = ballanced_root;
						else
							throw std::runtime_error("Invalid parent link");

						_Base::update_height(par);
					}
					else
						this->m_pRoot = ballanced_root;

					

					/*if (ballanced_root->parent)
					{
						

						
					}
					else//This was root
					{
						m_pRoot = ballanced_root;
					}*/

					//cout << *this;
				}
					
			}
		}
		return res;
	}
	/*bool insert(const value_t & val)
	{
		node_t * res = _Base::insert(val);
		if (res == nullptr)
			return false;

		return true;
	}*/

private:
	//Returns subroot of ballance operation
	enum class ballance_operation_t { rotate_left, rotate_right, double_rotate_left, double_rotate_right, unknown};

	node_t * ballance_node(node_t * node)
	{
		ballance_operation_t op = ballance_operation_t::unknown;

		
		if (node->right)
		{
			int h = _Base::subtree_height(node->left);
			if ((_Base::subtree_height(node->right->left) == h) && (_Base::subtree_height(node->right->right) == h + 1))
				op = ballance_operation_t::rotate_left;
			else if ((_Base::subtree_height(node->right->right) == h) && ((_Base::subtree_height(node->right->right) == h) || (_Base::subtree_height(node->right->right) == h + 1)))
				op = ballance_operation_t::double_rotate_left;
		}
		
		if (node->left)
		{
			int h = _Base::subtree_height(node->right);
			if ((_Base::subtree_height(node->left->left) == h + 1) && (_Base::subtree_height(node->left->right) == h))
				op = ballance_operation_t::rotate_right;
			else if ((_Base::subtree_height(node->left->left) == h) && ((_Base::subtree_height(node->left->right) == h) || (_Base::subtree_height(node->left->right) == h + 1)))
				op = ballance_operation_t::double_rotate_right;
		}
		
		switch (op)
		{
		case ballance_operation_t::rotate_left:
			return rotate_left(node);
		case ballance_operation_t::rotate_right:
			return rotate_right(node);
		case ballance_operation_t::double_rotate_right:
			return double_rotate_right(node);
		case ballance_operation_t::double_rotate_left:
			return double_rotate_left(node);
		default:
			std::cout << *this;
			throw std::runtime_error("Invalid situation");
		}
	}

	node_t * rotate_left(node_t * r)
	{
		node_t *node_B = r->right->left,
			*x = r->right;

		r->right = node_B;
		x->left = r;

		//Parents
		x->parent = r->parent;
		r->parent = x;
		if (node_B)
			node_B->parent = r;

		//Heights
		r->height = std::max(_Base::subtree_height(r->left), _Base::subtree_height(r->right));
		x->height = std::max(_Base::subtree_height(x->left), _Base::subtree_height(x->right));

		return x;
	}

	node_t * rotate_right(node_t * r)
	{

		node_t *x = r->left,
			*node_B = x->right;

		r->left = node_B;
		x->right = r;

		//Parents
		x->parent = r->parent;
		r->parent = x;
		if (node_B)
			node_B->parent = r;

		//Heights
		r->height = std::max(_Base::subtree_height(r->left), _Base::subtree_height(r->right));
		x->height = std::max(_Base::subtree_height(x->left), _Base::subtree_height(x->right));

		return x;
	}

	node_t * double_rotate_right(node_t * r)
	{
		node_t *x = r->left,
			*w = x->right,
			*node_B = w->left,
			*node_C = w->right;

		x->right = node_B;
		r->left = node_C;

		w->left = x;
		w->right = r;

		//Parents
		w->parent = r->parent;
		x->parent = w;
		r->parent = w;
		if (node_B)
			node_B->parent = x;
		if (node_C)
			node_C->parent = r;

		//Heights
		x->height = std::max(_Base::subtree_height(x->left), _Base::subtree_height(x->right));
		r->height = std::max(_Base::subtree_height(r->left), _Base::subtree_height(r->right));
		w->height = std::max(_Base::subtree_height(w->left), _Base::subtree_height(w->right));

		return w;
	}

	node_t * double_rotate_left(node_t * r)
	{
		node_t *x = r->right,
			*w = x->left,
			*node_B = w->left,
			*node_C = w->right;

		x->left = node_C;
		r->right = node_B;

		w->left = r;
		w->right = x;

		//Parents
		w->parent = r->parent;
		x->parent = w;
		r->parent = w;
		if (node_B)
			node_B->parent = r;
		if (node_C)
			node_C->parent = x;


		//Heights
		x->height = std::max(_Base::subtree_height(x->left), _Base::subtree_height(x->right));
		r->height = std::max(_Base::subtree_height(r->left), _Base::subtree_height(r->right));
		w->height = std::max(_Base::subtree_height(w->left), _Base::subtree_height(w->right));

		return w;
	}
};

template<typename K, typename V>
class range_map
{
	using key_t = K;
	using value_t = V;
	using combined_value_t = std::pair<key_t, value_t>;
	/*struct cmp_t
	{
		bool operator()(const combined_value_t & lhs, const combined_value_t & rhs) const
		{
			return lhs.first < rhs.first;
		}

		bool operator()(const value_t & key, const combined_value_t & rhs) const
		{
			return key < rhs.first;
		}
	};

	struct keq_eq_t
	{
		bool operator()(const key_t & key, const combined_value_t & cv) const
		{
			return key == cv.first;
		}
	};*/

	using tree_t = avl_tree<key_t, value_t>;
	using iterator = typename tree_t::iterator;
public:
	std::pair<iterator, bool> insert(const combined_value_t & val_pair)
	{
		return m_tree.insert(val_pair);
	}

	std::pair<iterator, bool> insert(const key_t & key, const value_t & val)
	{
		return insert(std::make_pair(key, val));
	}

	/*
	Finds the node, which is a left point of interval where
	given key is.
	*/
	iterator find(const key_t & key) const
	{
		return m_tree.lower_bound(key);
	}

	void print() const
	{
		std::cout << m_tree;
	}

	size_t size() const
	{
		return m_tree.size();
	}

	template<typename Fun>
	void for_each(Fun fun) const
	{
		m_tree.for_each(fun);
	}

	iterator end() const
	{
		return m_tree.end();
	}
private:
	avl_tree<key_t, value_t> m_tree;

};

#endif
