
#ifndef UltraCore_transition_index_h
#define UltraCore_transition_index_h

#include <iostream>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iostream>

using namespace std;

namespace indexing
{

	struct node_base_t
	{
		enum class kind_t { node, leaf };

		node_base_t(kind_t _kind)
			:m_kind(_kind)
		{}

		kind_t kind() const
		{
			return m_kind;
		}
		
		const kind_t m_kind;
	};

	struct node_t : public node_base_t
	{
		node_t()
			:node_base_t(kind_t::node)
		{}

		int m_varIndex;
		//0 - linked with 0, 1 - linked with 1
		node_base_t * m_children[2];
	};

	template<typename T>
	struct leaf_t : public node_base_t
	{
		using transition_t = T;

		template<typename It>
		leaf_t(It first, It last)
			:node_base_t(kind_t::leaf), m_transitions(first, last)
		{}

		std::vector<transition_t*> m_transitions;
	};

	template<typename T, typename S>
	class transition_index
	{
		using transition_t = T;
		using state_t = S;
		using leaf_type = leaf_t<T>;

	public:
		transition_index()
			:m_pRoot(nullptr), m_totalTransitions(0)
		{}

		/*transition_index(const transition_index & rhs) //No copy
			:m_pRoot(nullptr)
		{}*/

		template<typename It>
		void build(It first, It last)
		{
			cout << "Building transition index node (" << std::distance(first, last) << " elements)..." << std::endl;

			m_totalTransitions = std::distance(first, last);

			std::vector<transition_t*> transition_ptrs(std::distance(first, last));
			int i = 0;
			for (auto it = first; it != last; ++it)
				transition_ptrs[i++] = &(*it);
			m_pRoot = build_node(transition_ptrs.begin(), transition_ptrs.end(), std::set<int>(), 2);
		}

		template<typename AF, typename F>
		void forall_available_transitions(const state_t & base_state, AF afun, F fun) const
		{
			node_base_t * cur_node = m_pRoot;

			while (cur_node->kind() != node_base_t::kind_t::leaf)
			{
				node_t * pnode = static_cast<node_t*>(cur_node);
				if (base_state.bool_part[pnode->m_varIndex])
					cur_node = pnode->m_children[1];
				else
					cur_node = pnode->m_children[0];
			}

			int successed = 0;
			auto p_leaf = static_cast<leaf_type*>(cur_node);
			for (auto & p_tr : p_leaf->m_transitions)
			{
				if (afun(base_state, *p_tr))
				{
					fun(*p_tr);
					++successed;
				}
			}

			std::cout << "Made indexed " << p_leaf->m_transitions.size() << " comparisons, " << successed << " success, instead of " << m_totalTransitions << std::endl;
		}
	private:
		template<typename It>
		node_base_t * build_node(It first, It last, std::set<int> indexed_vars, int max_depth)
		{
			int initial_number = std::distance(first, last);
			//cout << "Building transition index node (" << std::distance(first, last) << " elements)..." << std::endl;

			if (max_depth <= 1)
				return new leaf_t<transition_t>(first, last);
			else
			{
				//Build distribution
				std::unordered_map<int, std::vector<transition_t*>> var_count;
				for (auto tr_it = first; tr_it != last; ++tr_it)
				{
					for (auto var_idx : (*tr_it)->bool_part.m_conditionVarIndices)
					{
						if (indexed_vars.find(var_idx) == indexed_vars.end())
							var_count[var_idx].push_back(*tr_it);
					}
				}

				//Find best group
				/*cout << "Get distribution:" << std::endl;

				for (auto el : var_count)
				cout << el.first << " is linked with " << el.second.size() << " transitions." << std::endl;*/

				auto best_gr = std::max_element(var_count.begin(), var_count.end(), [](const std::pair<int, std::vector<transition_t*>> & lhs, const std::pair<int, std::vector<transition_t*>> & rhs){
					return lhs.second.size() < rhs.second.size();
				});

				//cout << "Best group has " << best_gr->second.size() << " elements among " << std::distance(first, last) << std::endl;

				if (best_gr->second.size() > 10)
				{
					std::sort(best_gr->second.begin(), best_gr->second.end());

					//Filter transitions felt into the group
					auto last_it = std::remove_if(first, last, [&](const transition_t * p_tr){
						return std::binary_search(best_gr->second.begin(), best_gr->second.end(), p_tr);
					});

					int removed_count = std::distance(last_it, last);

					indexed_vars.insert(best_gr->first);
					node_t * new_node = new node_t;
					new_node->m_varIndex = best_gr->first;

					//Build false-value child
					auto group_last_it = last_it;

					for (auto & best_tr : best_gr->second)
					{
						if (!best_tr->bool_part.condition.value.at(best_gr->first))
							*group_last_it++ = best_tr;
					}
					new_node->m_children[0] = build_node(first, group_last_it, indexed_vars, max_depth - 1);

					//Build true-value child
					group_last_it = last_it;
					for (auto & best_tr : best_gr->second)
					{
						if (best_tr->bool_part.condition.value.at(best_gr->first))
							*group_last_it++ = best_tr;
					}
					new_node->m_children[1] = build_node(first, group_last_it, indexed_vars, max_depth - 1);

					return new_node;
				}
				else
				{
					//cout << "Creating leaf (" << std::distance(first, last) << " elements)..." << std::endl;
					return new leaf_t<transition_t>(first, last);
				}
			}
		}

	private:
		node_base_t * m_pRoot;
		int m_totalTransitions;
	};

};

#endif
