
#ifndef UltraPlanner_UPDDLSolver_h
#define UltraPlanner_UPDDLSolver_h

#include <solver/USolver.h>
#include <solver/UMetric.h>
#include <multicore_solver/UMultiCoreSolver.h>
#include <cuda_solver/UCUDASolver.h>
#include <solver/heuristics/UPlanningGraphHeuristic.h>
#include <solver/heuristics/UPlanningGraph.h>
#include <solver/heuristics/UGoalCountHeuristic.h>
#include <pddl/UTaskGenerator.h>
#include <vector>
//================================PDDL======================================

class UHeuristic;

class UPDDLSolverBase : public UInstantiator
{
public:
	UPDDLSolverBase(UPDDLDomain * domain, UPDDLProblem * problem);
	void interpretState(const UTransitionSystem & tr_sys, const UState & state) const;
	void interpretSearchNode(const USearchNode & node) const;
	void interpretTransition(const UTransition & transition) const;
	void interpretTransitionPath(const UTransitionSystem & tr_sys, const TransitionPath & path) const;

	UTransition createTransition(const UPDDLActionInstance & action_instance) const;

	std::vector<UPDDLActionInstance> getActionInstances(const std::vector<size_t> & indices) const;

	void instantiatePredicates(const TypesIndex & ti);

	bool isReachable(const UPDDLActionInstance & inst) const;
	void bruteforceActionInstances(const TypesIndex & type_index, std::vector<UPDDLActionInstance> & target_array, UPDDLActionInstance new_instance, const int param_index) const;
	void instantiateActions(const TypesIndex & types_index);
	void instantiateFunctions(const TypesIndex & types_index);
	//void buildPlanningGraph(UTransitionSystem & transition_system, const UState & initial_state, const UPartialState & goal);
protected:
	UPDDLDomain * m_pDomain;
	UPDDLProblem * m_pProblem;

	struct
	{
		std::vector<UPDDLPredicateInstance> variableInstances, constantInstances;
		std::vector<UPDDLPredicate*> variable, constant;
	} mPredicates;

	struct
	{
		std::vector<UPDDLFunctionInstance> variableInstances, constantInstances;
		std::vector<UPDDLFunction*> variable, constant;
	} mFunctions;

	std::vector<UPDDLActionInstance> mActionInstances;

	//UHeuristic * pg_heuristic;
};

template<class T>
class UPDDLSolver : public T, public UPDDLSolverBase
{
	typedef T _Base;
public:
	UPDDLSolver(size_t solver_output_buffer_size)
		:_Base(solver_output_buffer_size), UPDDLSolverBase(0, 0)
	{
	}

    void solve(const UPlanningTask & planning_task)
    {
		auto & transition_system = std::get<0>(planning_task);

		//Configurate state factory
		_Base::m_database.setDimensions(transition_system.boolCount(), transition_system.floatCount());

        //Calculate object types index
/*        TypesIndex types_index;
        for(auto obj : m_pProblem->objects)
        {
            auto cur_type = obj->type;
            while(cur_type)
            {
                types_index[cur_type].push_back(obj);
                cur_type = cur_type->parent;
            }
        }
        
        //============================Instancing===================================
        UInstantiator instantiator;
        //Predicates
        instantiatePredicates(types_index);
        
        
        //Functions
        instantiateFunctions(types_index);
        
        
        
        
        UState initial_state = init_state_ref;
        //(mPredicates.variableInstances.size(), mFunctions.variableInstances.size());
        initial_state.clear();
        //auto xx = initial_state.predicateStates.toIndices();
        
        //Predicate instances states
        for(auto gpi : m_pProblem->initialState.predicates)
        {
            auto pred_inst_it = std::find_if(mPredicates.variableInstances.begin(), mPredicates.variableInstances.end(), [=](const UPDDLPredicateInstance & pi){
                return (pi.object == gpi.object) && (pi.params == gpi.params);
            });
            
            //For variable predicate
            if(pred_inst_it != mPredicates.variableInstances.end())
            {
                auto pred_index = std::distance(mPredicates.variableInstances.begin(), pred_inst_it);
                initial_state.setPredicateState(pred_index, true);
            }
            else
            {
                mPredicates.constantInstances.push_back(gpi);
            }
        }
        
        //Variable function instances values
        for(int i = 0; i < mFunctions.variableInstances.size(); ++i)
        {
            auto fit = std::find_if(m_pProblem->initialState.functions.begin(), m_pProblem->initialState.functions.end(), [=](const UPDDLFunctionInstance & fi){return fi.object == mFunctions.variableInstances[i].object;});
            if(fit == m_pProblem->initialState.functions.end())
                initial_state.functionValues[i] = 0.0f;
            else
            {
                initial_state.functionValues[i] = fit->value;
                m_pProblem->initialState.functions.erase(fit);
            }
        }
        
        init_state_ref = initial_state;
        cout << "done.\n";
        
        //Constant function values
        mFunctions.constantInstances = m_pProblem->initialState.functions;
        
        //Instantiate actions
        instantiateActions(types_index);
        
        //===============Generate goal description===================
        UPartialState goal;
        goal.flags.resize(mPredicates.variableInstances.size());
        goal.flags.clear();
        
        for(auto gpi : m_pProblem->goal.predicates)
        {
            auto pred_inst_it = std::find(mPredicates.variableInstances.begin(), mPredicates.variableInstances.end(), gpi);
            if(pred_inst_it == mPredicates.variableInstances.end())
                throw core_exception("Unable to locate predicate instance.");
            auto pred_index = std::distance(mPredicates.variableInstances.begin(), pred_inst_it);
            
            goal.flags.set(pred_index, 1);
        }
        
        //goal.flags.print();
        
        //===============Create transition system====================
        cout << "Creating transition system...";
        UTransitionSystem transition_system(mPredicates.variableInstances.size());
        for(auto action : mActionInstances)
            transition_system.addTransition(createTransition(action));
        
        //Saving flags descriptions
        vector<string> descrs;
        for(auto & vi : mPredicates.variableInstances)
            descrs.push_back(to_string(vi));
        transition_system.setFlagsDescription(descrs);
        
        cout << " done.\n";
        
        //transition_system.print();
        
        
        
        //auto r = pg_h->estimate(initial_state, goal);
        //Remove univolved actions
        auto involved_actions = UPlanningGraphHeuristic::buildPlanningGraph(transition_system.toRelaxed(), initial_state, goal).actions();
              
        //buildPlanningGraph(transition_system, initial_state, goal);
        //initial_state, goal
        
        
        
        
        //=================Solve=================
        
        //interpretTransition(transition_system.transition(21));
        //interpretTransition(transition_system.transition(22));
        cout << "Starting solving\n";
        
        
        */

		//==================Planning graph================
		auto pg_h = new UPlanningGraphHeuristic(transition_system);
        _Base::m_estimator.setHeuristic(0, pg_h);
        //_Base::m_estimator.setHeuristic(1, new UGoalCountHeuristic());

		//Generate initial state
        auto init_state_ref = _Base::m_database.createState();
		init_state_ref = std::get<1>(planning_task);
		init_state_ref.setEstimation(m_estimator(std::get<1>(planning_task), std::get<2>(planning_task)));

        UMetric metric;
        
		auto res = _Base::solve(transition_system, init_state_ref, std::get<2>(planning_task), &metric, 0, get<3>(planning_task));
        
        _Base::dump();
        
        cout << "Found " << res.size() << " plans.\n";
        for(auto r : res)
        {
            interpretState(transition_system, r.finalState);
            interpretTransitionPath(transition_system, r.path);
        }
        //auto node_it = mOutputMerger.mExploredNodes.begin()+50;
        //for(int i = 0; i < 6; ++i)
        //	++node_it;
        //interpretSearchNode(*node_it);
        //mActionInstances[ai].print();
        //interpretTransition(transition_system.transition(ai));
    }
};

#endif
