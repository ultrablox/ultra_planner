
#ifndef UltraPlanner_UPlanningGraph_h
#define UltraPlanner_UPlanningGraph_h

#include <set>
#include <vector>
#include <core/transition_system/UState.h>
#include <memory>

class UState;
class UTransition;

class UPlanningGraph
{
public:
	struct Layer
	{
		enum class Type {State, Transition};
		virtual ~Layer()
		{}
		virtual Type type() const = 0;
	};

	struct StateLayer : public Layer
	{
		StateLayer(const UState & _state)
			:state(_state)
		{
		}

		virtual Type type() const override
		{
			return Type::State;
		}
		
		const UState state;
	};

	struct TransitionLayer : public Layer
	{
		virtual Type type() const override
		{
			return Type::Transition;
		}

		std::set<size_t> transitions;
	};

	~UPlanningGraph();

	//Checks and adds layer
	void addLayer(Layer * new_layer);

	//Creates state layer
	void createLayer(const UState & state);

	//Creates transition layer
	UPlanningGraph::TransitionLayer * createLayer(const std::set<size_t> & transitions);

	int layerIndex(const UState & state) const;

	int layerCount() const;

	//Returns all actions
	set<size_t> actions() const;
private:
	std::vector<std::shared_ptr<Layer>> mLayers;
};

#endif
