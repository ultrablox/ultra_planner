
#ifndef UltraSolver_state_space_solver_h
#define UltraSolver_state_space_solver_h

#include "search_algorithms/astar_engine.h"
#include "search_algorithms/greedy_bfs_engine.h"
#include "search_algorithms/batched_engine.h"
#include <core/algorithm/graph.h>
#include <core/UExternalMemoryController.h>
#include <thread>
#include <chrono>

using namespace std::chrono;

template<typename T>
struct default_goal_checker
{
	using transition_system_t = T;
	using state_t = typename transition_system_t::state_t;

	default_goal_checker(const transition_system_t & _system)
	:m_system(_system)
	{}

	bool operator()(const state_t & state) const
	{
		return m_system.is_solved(state);
	}
private:
	const transition_system_t & m_system;
};

template<typename T, typename G = default_goal_checker<T>>
class state_space_solver
{
	/*template<typename X>
	struct engine_generator
	{

	};*/

	using transition_system_t = T;
	using graph_t = transition_system_graph<transition_system_t>;
	using state_t = typename transition_system_t::state_t;
	using streamer_t = typename graph_t::vertex_streamer_t;
	using goal_checker_t = G;

public:

	state_space_solver(std::istream & in_stream, std::ostream & out_stream)
		:m_system(transition_system_t::deserialize_problem_size(in_stream)), m_initialState(m_system.size()), m_outStream(out_stream), m_goalChecker(m_system), m_detailedPlan(false)
	{
		m_system.deserialize_state(in_stream, m_initialState);

		size_t state_power = 0;//m_system.max_state_count_10();

		m_maxStateCount = pow(10, state_power);
		cout << "Search space contains maximum 10^" << state_power << " states" << std::endl;
	}

	state_space_solver(const transition_system_t & tr_system, std::ostream & out_stream, const state_t & initial_state/*, goal_checker_t && gc*/)
		:m_system(tr_system), m_outStream(out_stream), m_goalChecker(m_system), m_initialState(initial_state), m_detailedPlan(false)
	{
	}

	template<template<typename> class HeuristicT>
	bool solve(bool ext_mem, const std::string & alg_name)
	{
		state_t result_state; //dummy variable
		return solve<HeuristicT>(ext_mem, alg_name, result_state);
	}

	template<template<typename> class HeuristicT>
	bool solve(bool ext_mem, const std::string & alg_name, state_t& result_state)
	{
		using heuristic_t = HeuristicT<transition_system_t>;
		if (ext_mem)
		{
			//UExternalMemoryController ext_memory_ctrl;
			return select_memory<true, heuristic_t>(alg_name, result_state);
		}
		else
			return select_memory<false, heuristic_t>(alg_name, result_state);
	}
private:
	template<bool ExtMem, typename HeuristicT>
	bool select_memory(const std::string & alg_name, state_t& result_state)
	{
		if (alg_name == "A*")
			return select_engine<astar_engine<graph_t, HeuristicT, ExtMem>>(result_state);
		else if (alg_name == "BA*")
			return select_engine<batched_engine<graph_t, HeuristicT, ExtMem>>(result_state);
		else if (alg_name == "GBFS")
			return select_engine<greedy_bfs_engine<graph_t, HeuristicT, ExtMem>>(result_state);
		else
			return false;
	}

	template<typename EngType>
	bool select_engine(state_t& result_state)
	{
		graph_t graph(m_system);

		EngType search_engine(streamer_t(graph.transition_system()));

		std::list<typename transition_system_t::transition_t> plan;
		std::thread monitor_thread([&](){
			this->monitoring_loop(search_engine);
		});

		cout << "Starting solving process..." << std::endl;

		std::vector<state_t> path;

		auto start_tp = high_resolution_clock::now();
		
		float plan_cost = 0.0f;

		bool found = search_engine(graph, m_initialState, [&](const state_t & state){
			return m_goalChecker(state);
		}, path, &plan_cost);

		auto end_tp = high_resolution_clock::now();
		double search_timecost = duration_cast<microseconds>(end_tp - start_tp).count() * 1e-6;

		if (found)
		{
			result_state = path.back();
			auto plan = m_system.build_transition_path(path.begin(), path.end());
		
			m_outStream << "Solution found (length " << plan.size() << ", cost " << plan_cost << ")" << std::endl;

			m_outStream << "Plan: ";
			print_range(m_outStream, plan.begin(), plan.end(), '\n');
			m_outStream << std::endl;

			if (m_detailedPlan)
			{
				m_outStream << "Detailed plan description:" << std::endl;
				m_system.trace_solution(m_outStream, plan.begin(), plan.end(), path.begin());
			}

			std::ofstream stats_file("stats.txt");
			stats_file << "wall_time:" << search_timecost << std::endl;
			stats_file << "plan_length:" << plan.size() << std::endl;
			stats_file << "plan_cost:" << plan_cost << std::endl;
			stats_file << search_engine.get_stats();
		}

		monitor_thread.join();

		return found;
	}

	template<typename EngType>
	void monitoring_loop(const EngType & search_engine)
	{
		std::ofstream monitor_data("monitoring_data.csv", std::ofstream::out);

		monitor_data << "Time,s;Merging Speed,nodes/sec;Cache Load,%;Nodes Merged;" << std::endl;

		std::chrono::milliseconds print_dura(10000);
		std::chrono::milliseconds dura(500);

		auto begin_tp = std::chrono::steady_clock::now();

		const int print_counter = print_dura.count() / dura.count();

		int counter = print_counter;

		cout << "Started solving monitor, interval " << dura.count() << " msecs" << std::endl;
		double last_explored_count = 0.0;
		while (!search_engine.finished())
		{
			if (!(counter--))
			{
				counter = print_counter;

				auto stats = search_engine.get_stats();

				cout << "=============Monitoring================" << std::endl;
				cout << stats;

				double explored_node_count = stats.state_count;
				size_t pow10_count = explored_node_count > 1 ? ceil(log10(explored_node_count)) : 0;

				float merge_speed = (explored_node_count - last_explored_count) / print_dura.count() * 1e3;
				cout << "Observed 10^" << pow10_count << " states (" << explored_node_count / m_maxStateCount << "%), speed = " << merge_speed << " nodes/sec" << std::endl;

				last_explored_count = explored_node_count;

				//Output to monitoring data
				auto cur_tp = std::chrono::steady_clock::now();
				monitor_data << std::chrono::duration_cast<std::chrono::seconds>(cur_tp - begin_tp).count() << ';' << merge_speed << ';' << stats.storage_stats[0].cache_load * 100.0f << ';' << explored_node_count << ';' << std::endl;
			}

			std::this_thread::sleep_for(dura);
		}
	}
private:
	transition_system_t m_system;
	double m_maxStateCount;
	//std::istream & m_inStream;
	std::ostream & m_outStream;
	state_t m_initialState;
	goal_checker_t m_goalChecker;
	bool m_detailedPlan;
};

#endif
