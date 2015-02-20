
#include "UPlanningGraph.h"

UPlanningGraph::~UPlanningGraph()
{
/*	for(auto & l : mLayers)
		delete l;
	mLayers.clear();*/
}

void UPlanningGraph::addLayer(Layer * new_layer)
{
	mLayers.push_back(shared_ptr<Layer>(new_layer));
}

void UPlanningGraph::createLayer(const UState & state)
{
	addLayer(new StateLayer(state));
}

UPlanningGraph::TransitionLayer * UPlanningGraph::createLayer(const std::set<size_t> & transitions)
{
	auto layer = new TransitionLayer();
	layer->transitions = transitions;
	addLayer(layer);

	return layer;
}

int UPlanningGraph::layerIndex(const UState & state) const
{
	int layer_index = mLayers.size()-1;
	//cout << "\n";
	while(layer_index > 0)
	{
		StateLayer * cur_layer = static_cast<StateLayer*>(mLayers[layer_index].get());

		//cur_layer->state.flags.print();
		//state.flags.print();

		if(!cur_layer->state.flags.equalMasked(state.flags, state.flags))
			break;
		layer_index -= 2;
		//cout << "=============================\n";
	}

	//cout << layer_index;
	return layer_index;
}

int UPlanningGraph::layerCount() const
{
	return mLayers.size();
}

set<size_t> UPlanningGraph::actions() const
{
	set<size_t> result;
	for(int i = 1; i < layerCount(); i += 2)
	{
		auto layer = static_cast<TransitionLayer*>(mLayers[i].get());
		result.insert(layer->transitions.begin(), layer->transitions.end());
	}

	return std::move(result);
}