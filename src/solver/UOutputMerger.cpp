
#include "UOutputMerger.h"
#include "USolver.h"
#include <stxxl.h>
//#include <stxxl/bits/common/binary_buffer.h>
#include <stxxl/sort>

using namespace stxxl;

UOutputMerger::UOutputMerger(USolver & solver_)
	:m_solver(solver_)
{

}

void UOutputMerger::merge()
{
	cout << "Merging estimated output... (" << m_solver.bufEstimatedOutput().size() << " groups) \n";
	auto start_tp = high_resolution_clock::now();

	//We don't need groups with less estimation, let's merge only the best
	//Now, we need all


	for(auto & group : m_solver.bufEstimatedOutput())
	{
		auto group_index = group.first;

		//Erase other nodes - don't let them eat memory
		/*auto second_it = m_solver.bufEstimatedOutput().begin();
		++second_it;
		m_solver.bufEstimatedOutput().erase(second_it, m_solver.bufEstimatedOutput().end());*/
	
		const size_t start_element_count = group.second.size();
		cout << "Group has " << start_element_count << " elements.\n";
		size_t new_count;

		const size_t group_node_count_before = m_solver.nodeFactory().nodeGroup(group_index).size();

		cout << "\tGroup estimation is " << group_index << "\n";

		/*if(group.second.size() < max_count_in_internal)
		{
			cout << "Merging in internal memory...\n";
			new_count = mergeInInternalMemory(group_index, group.second);
		}
		else*/
		{
			cout << "Merging in external memory...\n";
			clearCache();
			new_count = mergeInExternalMemory(group_index, group.second);
		}

		cout << "\t\tmerged " << new_count << " new nodes (" << static_cast<float>(new_count)/ start_element_count * 100.0f << "%)\n";
	}

	m_solver.bufEstimatedOutput().clear();

	auto end_tp = high_resolution_clock::now();
		cout << "\tfinished in " << duration_cast<microseconds>(end_tp - start_tp).count() * 1e-6 << " sec.\n";

	/*if(new_count > 0)
		return NodeRefIntervalType(m_solver.nodeFactory().nodeRef(NodeAddressT(group_index, group_node_count_before)), m_solver.nodeFactory().nodeRef(NodeAddressT(group_index, group_node_count_before + new_count)));
	else
		return NodeRefIntervalType();*/
}

void UOutputMerger::clearCache()
{
	m_mergingCache.storage.clear();
	m_mergingCache.group_index = NodeEstimationT();
}

void UOutputMerger::preloadLocalMergingCache(const NodeEstimationT & group_index)
{
	//Unload all other groups from internal memory
	/*for(auto & gr : m_data)
	{
		cout << "Vector memory usage is " << fixed << internal_memory_usage(gr.second) << " MBs\n";
		
		if(gr.first != group_index)
		{
			cout << "\t\tunloading container for group " << gr.first << "\n";
			gr.second.resize(0);
		}
	}*/

	/*if(group_index != m_mergingCache.group_index)
	{
		m_mergingCache.storage.clear();
		m_mergingCache.group_index = group_index;

		//Preload all old nodes from such group
		auto & group_db_container = m_solver.nodeFactory().nodeGroup(group_index);
		cout << "\t\tpreloading " << group_db_container.size() << " old states\n";
		for(auto el : group_db_container)
			m_mergingCache.storage.insert(m_solver.stateFactory()[el.stateIndex]);
	}
	else
	{
		const float load_factor = m_mergingCache.storage.load_factor();
		cout << "\t\tGroup already in memory. Load factor is " << setprecision(2) << load_factor << ".";
		if(load_factor > 0.6)
		{
			cout << " Rehashing...";
			m_mergingCache.storage.rehash(m_mergingCache.storage.bucket_count() * 2);
			cout << " done.\n";
		}
		else
			cout << "\n";
	}*/
}

StateAddress UOutputMerger::createNewNode(const NodeEstimationT & group_index, const UState & int_mem_state, StateAddress addr)
{
	auto new_state_ref = m_solver.stateFactory().createState(addr);
	new_state_ref = int_mem_state;

//	auto new_node_ref = m_solver.nodeFactory().createNode(group_index, new_state_ref, int_mem_node.parentAddress());
			
	if(m_solver.outputToNewBuffer())
	{
		//auto & new_gr_ref = m_solver.bufExpandNew()[group_index];
		//new_gr_ref.push_back(new_node_ref);
		m_solver.bufExpandNew().push_back(new_state_ref);
	}
	
	//Check goals
	if(m_solver.goal().matches(int_mem_state))
		m_solver.addGoalNode(new_state_ref);

	return new_state_ref.address();
}

size_t UOutputMerger::mergeInInternalMemory(const NodeEstimationT & group_index, USolverBase::NodeLocalContainerType & new_nodes)
{
	const size_t start_element_count = new_nodes.size();
	cout << "\tmerging " << start_element_count << " elements\n";
	
	//Preload local cache
	preloadLocalMergingCache(group_index);

	//Merge new nodes with them
	size_t new_count(0);

/*	while(!new_nodes.empty())
	{
		auto & new_el = *new_nodes.rbegin();
		auto new_it = m_mergingCache.storage.insert(new_el);
			
		if(new_it.second)
		{
			++new_count;
			createNewNode(group_index, new_el);
		}

		//Delete element from src container
		new_nodes.pop_back();
	}
	
*/

	return new_count;
}
/*
binary_buffer to_binary_buffer(const UState & state, size_t new_index)
{
	binary_buffer bb;
	
	//Serialize flags
	for(int i = 0; i < state.flags.mData.size(); ++i)
		bb.put<UBitset::value_type>(state.flags.mData[i]);

	//Serialize floats
	for(int i = 0; i < state.functionValues.size(); ++i)
		bb.put<UFloatVector::value_type>(state.functionValues[i]);
	
	bb.put<size_t>(new_index);

	return bb;
}
*/
struct StateData
{
	explicit StateData(size_t src_index = 0ULL, size_t hash_value = 0ULL, bool is_new = false)
		:srcIndex(src_index), hashValue(hash_value), isNew(is_new)
	{
	}

	size_t srcIndex;
	size_t hashValue;
	bool isNew;
};

ostream & operator<<(ostream & os, const StateData & data)
{
	return os << data.isNew << ":" << data.hashValue << ":" << data.srcIndex;
}

/*
StateData to_state(const binary_buffer & bb, int flag_element_count, int float_count)
{
	StateData res;

	stxxl::binary_reader br(bb);

	for(int i = 0; i < flag_element_count; ++i)
		res.state.flags.mData.push_back(br.get<UBitset::value_type>());

	for(int i = 0; i < float_count; ++i)
		res.state.functionValues.push_back(br.get<UFloatVector::value_type>());

	//res.node_link = br.get<size_t>();
	return res;
}*/

struct StateComparator
{
	bool operator()(const StateData & d1, const StateData & d2) const
	{
		return d1.hashValue < d2.hashValue;
	}

	StateData min_value() const
    {
		return StateData(0, numeric_limits<size_t>::min());
    }

    StateData max_value() const
    {
		return StateData(0, numeric_limits<size_t>::max());
    }

/*	StateComparator(int flag_element_count, int float_count)
		:flagElementCount(flag_element_count), floatCount(float_count), mainBytesCount(sizeof(UBitset::value_type)*flagElementCount + floatCount * sizeof(UFloatVector::value_type))
	{
	}

    bool operator()(const binary_buffer & a, const binary_buffer & b) const
    {
		int r = memcmp(a.data(), b.data(), mainBytesCount);
		return (r < 0);
    }

	bool equal(const binary_buffer & a, const binary_buffer & b) const
	{
		int r = memcmp(a.data(), b.data(), mainBytesCount);
		return (r == 0);
	}

	template<unsigned char val>
	void fillBuffer(binary_buffer & buf)
	{
		UBitset::value_type b_val;
		memset(&b_val, val, sizeof(b_val));
		for(int i = 0; i < flagElementCount; ++i)
			buf.put<UBitset::value_type>(b_val);

		UFloatVector::value_type f_val;
		memset(&f_val, val, sizeof(f_val));
		for(int i = 0; i < floatCount; ++i)
			buf.put<UFloatVector::value_type>(f_val);

		buf.put<size_t>(0);
	}

	binary_buffer min_value()
    {
		binary_buffer res;
		fillBuffer<0x00>(res);
		return res;
    }
    binary_buffer max_value()
    {
		binary_buffer res;
		fillBuffer<0xff>(res);
		return res;
    }
private:
	const int flagElementCount, floatCount, mainBytesCount;*/
};

struct ResultComparator
{
	ResultComparator(size_t hash_val)
		:hashVal(hash_val)
	{
	}

	bool operator()(const StateData & data) const
	{
		return data.hashValue != hashVal;
	}

	size_t hashVal;
};

bool operator==(const StateData & data, const ResultComparator & cmp)
{
	return cmp(data);
}

template<typename T>
class set_eq : public std::vector<T>
{
	typedef std::vector<T> _Base;
public:
	bool push_back(const T & el, bool check_equality = true)
	{
		if(check_equality)
		{
			auto old_it = std::find(_Base::begin(), _Base::end(), el);
			if(old_it == _Base::end())
			{
				_Base::push_back(el);
				return true;
			}
			else
				return false;
		}
		else
		{
			_Base::push_back(el);
			return true;
		}
	}
};

size_t UOutputMerger::mergeInExternalMemory(const NodeEstimationT & group_index, USolverBase::NodeLocalContainerType & new_nodes)
{
	//Create local cache
	//UStateFactory local_cache(m_solver.stateFactory().flagElementCount(), m_solver.stateFactory().floatCount());
	stxxl::VECTOR_GENERATOR<StateData>::result ext_container;
	

	//Fill it with old states
	auto & group_db_container = m_solver.nodeFactory().nodeGroup(group_index);

	cout << "\tpreloading " << group_db_container.size() << " old states...";
	
	for(auto el : group_db_container)
	{
		auto state_ref = m_solver.stateFactory()[el.stateIndex];
		ext_container.push_back(StateData(state_ref.address(), state_ref.hash(), false));
	}

	cout << " done.\n";

	//Add new states
	for(size_t i = 0; i < new_nodes.size(); ++i)
	{
		ext_container.push_back(StateData(i, new_nodes[i].state.hash(), true));
	}

	//Sort mixed states
	cout << "\tsorting... ";
	size_t memory_to_use = 30ULL * 1024 * 1024;
	stxxl::sort(ext_container.begin(), ext_container.end(), StateComparator(), memory_to_use);

	cout << " done.\n";

	//for(int i = 0; i < 1000; ++i)
	//	std::cout << ext_container[i] << "\n";

	//Go linear and analize
	cout << "\tanalizing sorted sequence...";

	size_t new_count(0);

	//cout << "total " << ext_container.size() << " elements...";
	auto cur_left = ext_container.begin();
	while(cur_left != ext_container.end())
	{
		//Move these elements to internal memory
		std::vector<StateData> current_group;

		size_t current_hash = cur_left->hashValue;
		while((cur_left != ext_container.end()) && (cur_left->hashValue == current_hash))
		{
			current_group.push_back(*cur_left);
			++cur_left;
		}

		//Sort them to make old nodes first (we strongly know - they are not equal)
		std::sort(current_group.begin(), current_group.end(), [](const StateData & d1, const StateData & d2){
			return !d1.isNew && d2.isNew;
		});

		//Add state to internal set to check they are new
		set_eq<UState> int_states;

		for(auto int_it = current_group.begin(); int_it != current_group.end(); ++int_it)
		{
			if(!int_it->isNew) //Old state - just insert with no questions
			{
				auto state_ref = m_solver.stateFactory()[int_it->srcIndex];
				int_states.push_back(UState(state_ref), false);
			}
			else //New state - check if it is really new
			{
				auto & node_ref = new_nodes[int_it->srcIndex];
				auto res = int_states.push_back(node_ref.state, true);
				if(res)
				{
					StateAddress new_state_addr = createNewNode(group_index, node_ref.state, node_ref.parentAddress);
					group_db_container.push_back(USearchNodeData(node_ref.parentAddress, new_state_addr, false));
					++new_count;
				}
			}
		}
	}

	cout << " done.\n";

	return new_count;
}
