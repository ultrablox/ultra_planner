
#include "USearchDatabase.h"

void USearchDatabase::clear()
{
	USearchNodeFactory::clear();
	UStateFactory::clear();
}

void USearchDatabase::dump(const std::string & file_name) const
{
	cout << "Dumping search database into '" << file_name << "'...\n";

	ofstream dump_file(file_name, ios::out | ios::binary);

	if(!dump_file)
	{
		cout << "Unable to create dump file.\n";
		return;
	}

	mTransitionSystem.serialize(dump_file);

	UStateFactory::serialize(dump_file);
	USearchNodeFactory::serialize(dump_file);
	
	dump_file.close();

	cout << "Dumping finished.\n";
}

int USearchDatabase::loadFromDump(const std::string & file_name)
{
	ifstream dump_file(file_name, ios::in | ios::binary);
	if(!dump_file)
	{
		cout << "Unable to open dump file.\n";
		return 1;
	}

	int res = mTransitionSystem.deserialize(dump_file);
	if(res)
	{
		cout << "Failed to deserialize transition system.\n";
		return res;
	}
		
	res = UStateFactory::deserialize(dump_file);
	if(res)
	{
		cout << "Failed to deserialize state factory.\n";
		return res;
	}

	res = USearchNodeFactory::deserialize(dump_file);
	if(res)
		cout << "Failed to deserialize node factory.\n";

	dump_file.close();

	return res;
}

void USearchDatabase::setTransitionSystem(const UTransitionSystem & tr_sys)
{
	mTransitionSystem = tr_sys;
}

const UTransitionSystem & USearchDatabase::transitionSystem() const
{
	return mTransitionSystem;
}

/*
TransitionPath USearchDatabase::buildTransitionPath(const USearchNodeReference & first_node_ref, const USearchNodeReference & last_node_ref)
{
	TransitionPath path;

	auto cur_node_ref = last_node_ref;
	while((cur_node_ref != first_node_ref) && cur_node_ref.hasParent())
	{
		auto parent_node_ref = cur_node_ref.parent();
		size_t cur_index = mTransitionSystem.transitionBetween((*this)[parent_node_ref.stateIndex()], (*this)[cur_node_ref.stateIndex()]);
		path.push_back(cur_index);
		cur_node_ref = parent_node_ref;
	}

	return TransitionPath(path.rbegin(), path.rend());
}

DetailedTransitionPath USearchDatabase::buildTransitionPath(const USearchNodeReference & first_node_ref)
{
	DetailedTransitionPath path;

	for(auto it = first_node_ref; it.hasParent(); it = it.parent())
	{
		auto parent_ref = it.parent();
		size_t cur_index = mTransitionSystem.transitionBetween((*this)[parent_ref.stateIndex()], (*this)[it.stateIndex()]);
		path.push_back(make_pair(cur_index, parent_ref));
	}

	return DetailedTransitionPath(path.rbegin(), path.rend());
}*/

TransitionPath USearchDatabase::buildTransitionPath(const UStateReference & last_state_ref)
{
	TransitionPath path;

	StateAddress cur_addr = last_state_ref.address();

	while(cur_addr != 0)
	{
		StateAddress parent_address = (*this)[cur_addr].parentAddress();

		size_t cur_index = mTransitionSystem.transitionBetween((*this)[parent_address], (*this)[cur_addr]);
		path.push_back(cur_index);
		cur_addr = parent_address;
	}
	/*auto cur_node_ref = last_node_ref;
	while((cur_node_ref != first_node_ref) && cur_node_ref.hasParent())
	{
		auto parent_node_ref = cur_node_ref.parent();
		size_t cur_index = mTransitionSystem.transitionBetween((*this)[parent_node_ref.stateIndex()], (*this)[cur_node_ref.stateIndex()]);
		path.push_back(cur_index);
		cur_node_ref = parent_node_ref;
	}
*/
	return std::move(TransitionPath(path.rbegin(), path.rend()));
}
