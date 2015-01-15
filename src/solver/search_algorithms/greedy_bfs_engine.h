
#ifndef UltraSolver_greedy_bfs_engine_h
#define UltraSolver_greedy_bfs_engine_h

#include "queued_engine.h"

template<class E>
struct greedy_bfs_node_priority_cmp
{
	typedef E element_t;

	bool operator()(const element_t & lhs, const element_t & rhs) const
	{
		return get<0>(lhs) < get<0>(rhs);
	}
};

template<typename N, typename H, bool ExtMemory>
class greedy_bfs_engine : public queued_search_engine<N, float, greedy_bfs_node_priority_cmp, ExtMemory>
{
	typedef float estimation_t;
	typedef public queued_search_engine<N, float, greedy_bfs_node_priority_cmp, ExtMemory> _Base;
	typedef H heuristic_t;
public:
	template<typename Gr>
	greedy_bfs_engine(Gr & graph)
		:_Base(graph)
	{}

	template<typename GraphT>
	bool operator()(GraphT & graph, state_t init_node, state_t goal_state, std::vector<state_t> & solution_path)
	{
		heuristic_t h_fun(graph.transition_system());

		enqueue(create_node(init_node, 0), std::numeric_limits<float>::max());

		float best_data = std::numeric_limits<float>::max();
		comparison_t current_data;

		while ((!m_searchQueue.empty()) && m_goalNodes.empty())
		{
			search_node_t cur_node = dequeue(&current_data);

			if (get<0>(current_data) < best_data)
			{
				best_data = get<0>(current_data);
				cout << "Best heuristic: " << best_data << std::endl;

				/*graph.transition_system().serialize_state(cout, get<2>(cur_node));*/
			}


			graph.forall_adj_verts(get<2>(cur_node), [&](const state_t & state){

				//Check that node is not expanded or discovered by trying to add
				auto res = m_database.add(state);
				if (res)
				{
					//search_node_t new_node = create_node(state, get<0>(cur_node));
					search_node_t new_node = m_database.create_node(state, get<0>(cur_node), get<3>(cur_node) +1);
					if (state == goal_state)
						m_goalNodes.push_back(new_node);
					else
					{
						auto res = enqueue(new_node, h_fun(state));
						if (res.first)
							m_database.compress_keys_less(res.second);
					}
						
				}
			});
		}

		if (!m_goalNodes.empty())
			solution_path = backtrace_path(m_goalNodes[0]);

		m_finished = true;

		return !m_goalNodes.empty();
	}
};

#endif
