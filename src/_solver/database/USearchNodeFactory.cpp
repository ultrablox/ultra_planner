
#include "USearchNodeFactory.h"
#include "UStateFactory.h"
#include "UStateReference.h"

USearchNodeFactory * USearchNodeFactory::m_pSingleton;

//==============Data===============

USearchNodeData::USearchNodeData(const StateAddress & parentAddress_, StateAddress stateIndex_, bool expanded_)
	:parentIndex(parentAddress_), stateIndex(stateIndex_), expanded(expanded_)
{
}

//==================================Factory=============================

USearchNodeFactory::USearchNodeFactory()
{
	m_pSingleton = this;

	cout << "Creating node factory...\n";
	//cout << "flagsData memory usage is " << fixed << internal_memory_usage(m_flagsData) << " MBs\n";
	//cout << "floatsData memory usage is " << fixed << internal_memory_usage(m_floatsData) << " MBs\n";
}

USearchNodeReference USearchNodeFactory::createNode(const NodeEstimationT & estimation_data, const UStateReference & state_ref)
{
	auto & container_ref = m_data_[estimation_data];
	
	NodeAddressT new_addr(estimation_data, container_ref.size());

	//container_ref.push_back(USearchNodeData(parent_address, state_ref.address(), false));

	//return USearchNodeReference(state_ref.address(), new_addr, parent_address, false);
	return USearchNodeReference();
}

UNodeReference USearchNodeFactory::createNode(const UStateReference & state_ref)
{
	auto estimation = state_ref.estimation();
	auto & container_ref = m_data[estimation];

	UNodeReference new_ref(state_ref.address(), state_ref.hash());
	container_ref.push_back(new_ref);
	return new_ref;
}
/*
USearchNodeReference USearchNodeFactory::groupBegin(const NodeEstimationT & group) const
{
	auto & container_ref = m_data_.find(group)->second;
	auto node_data = container_ref[0];

	NodeAddressT addr(group, 0);
	return nodeRef(node_data, addr);
}

USearchNodeReference USearchNodeFactory::groupEnd(const NodeEstimationT & group) const
{
	auto & container_ref = m_data_.find(group)->second;
	NodeAddressT addr(group, container_ref.size());
	return nodeRef(USearchNodeData(), addr);
}*/

NodeEstimationT USearchNodeFactory::bestGroupIndex()
{
	return m_data_.begin()->first;
}
/*
USearchNodeReference USearchNodeFactory::nodeRef(const NodeAddressT & address) const
{
	auto & container_ref = m_data_.find(address.groupIndex)->second;

	auto node_data = container_ref[address.localIndex];

	return nodeRef(node_data, address);
}

USearchNodeReference USearchNodeFactory::nodeRef(const USearchNodeData & node_data, const NodeAddressT & address) const
{
	return USearchNodeReference(node_data.stateIndex, address, node_data.parentIndex, node_data.expanded);
}*/

UNodeEstimatedRef USearchNodeFactory::estimatedNodeRef(const UStateReference & state_ref) const
{
	return UNodeEstimatedRef(state_ref.address(), state_ref.estimation());
}
/*
std::vector<USearchNodeReference> USearchNodeFactory::getUnexpandedNodes(const NodeEstimationT & node_group, size_t max_count) const
{
	cout << "Searching for " << max_count << " unexpanded nodes in group " << node_group << "\n";

	std::vector<USearchNodeReference> result;

	auto & groupContainer = m_data_.find(node_group)->second;

	

	//size_t i(0);
	const size_t total_count = groupContainer.size();

	for(auto it = groupContainer.rbegin(); (it != groupContainer.rend()) && (max_count > 0); ++it)
	{
		if(!it->expanded)
		{
			--max_count;

			NodeAddressT addr;
			addr.groupIndex = node_group;
			addr.localIndex = distance(it, groupContainer.rend()) - 1;
			//addr.localIndex = distance(groupContainer.begin(), it);
			//addr.localIndex = i;

			result.push_back(nodeRef(*it, addr));
		}
	}

	cout << "Searching finished, " << result.size() << " nodes found\n";
	return result;
}*/

size_t USearchNodeFactory::expandedNodeCount() const
{
	size_t res(0);

	for(auto gr : m_data_)
	{
		for(auto el : gr.second)
		{
			if(el.expanded)
				++res;
		}
	}

	return res;
}

void USearchNodeFactory::flush() const
{
	for(auto & gr : m_data_)
		gr.second.flush();
}

void USearchNodeFactory::clear()
{
	size_t total_count(0);
	for(auto & gr : m_data_)
		total_count += gr.second.size();

	cout << "Node factory contains " << m_data_.size() << " groups, total " << total_count << " elements.\n";

	m_data_.clear();
}

const USearchNodeFactory::data_type & USearchNodeFactory::data() const
{
	return m_data_;
}

void USearchNodeFactory::serialize(ofstream & fout) const
{
	cout << "Dumping " << m_data_.size() << " node groups...\n";

	//Group count
	serialize_value<size_t>(fout, m_data_.size());

	for(auto & ng : m_data_)
	{
		cout << "Serializing group " << ng.first << ", " << ng.second.size() << " nodes...\n";
		serialize_value(fout, ng.first);
		serialize_vector(fout, ng.second);
	}

	cout << "Total " << expandedNodeCount() << " expanded nodes.\n";
}

int USearchNodeFactory::deserialize(ifstream & fin)
{
	size_t group_count = deserialize_value<size_t>(fin);

	cout << "Loading " << group_count << " groups...\n";

	for(size_t g = 0; g < group_count; ++g)
	{
		NodeEstimationT gr = deserialize_value<NodeEstimationT>(fin);
		auto vec = deserialize_vector<node_data_container_type>(fin);
		m_data_.insert(make_pair(gr, vec));
	}

	cout << "Total " << expandedNodeCount() << " expanded nodes.\n";

	return 0;
}
