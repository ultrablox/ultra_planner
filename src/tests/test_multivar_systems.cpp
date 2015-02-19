#include "test_helpers.h"
#include "transition_system_helpers.h"
#include <core/varset_system/boolvar_system.h>
#include <core/varset_system/floatvar_system.h>
#include <core/transition_system.h>


bool operator<(const boolvar_transition & lhs, const boolvar_transition & rhs)
{
	return lhs.name < rhs.name;
}


void test_boolvar_systems()
{
	using bvsystem_t = transition_system<boolvar_system>;

	/*
	Simple transition system. {a, b, c, d, e}
	*/
	bvsystem_t bsystem(5);

	/*
	Transitions:
	a -> b, !a
	b -> c, !b
	c -> d, !d
	d -> e, !d
	*/
	bvsystem_t::transition_t ab_trans({ 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0 }, { 0, 1, 0, 0, 0 }, { 1, 1, 0, 0, 0 }, "a->b");
	bsystem.add_transition(ab_trans); //a -> b, !a

	bvsystem_t::transition_t bc_trans({ 0, 1, 0, 0, 0 }, { 0, 1, 0, 0, 0 }, { 0, 0, 1, 0, 0 }, { 0, 1, 1, 0, 0 }, "b->c");
	bsystem.add_transition(bc_trans); //b -> c, !b

	bvsystem_t::transition_t cd_trans({ 0, 0, 1, 0, 0 }, { 0, 0, 1, 0, 0 }, { 0, 0, 0, 1, 0 }, { 0, 0, 1, 1, 0 }, "c->d");
	bsystem.add_transition(cd_trans); //c -> d, !d

	bvsystem_t::transition_t de_trans({ 0, 0, 0, 1, 0 }, { 0, 0, 0, 1, 0 }, { 0, 0, 0, 0, 1 }, { 0, 0, 0, 1, 1 }, "d->e");
	bsystem.add_transition(de_trans); //d -> e, !d

	//Test available transitions
	assert_test(test_available_transitions(bsystem, bvsystem_t::state_t({ 1, 0, 0, 0, 0 }), std::vector<bvsystem_t::transition_t>({ ab_trans })), "Boolvar System: available transitions");
	assert_test(test_available_transitions(bsystem, bvsystem_t::state_t({ 0, 1, 0, 0, 0 }), std::vector<bvsystem_t::transition_t>({ bc_trans })), "Boolvar System: available transitions");
	assert_test(test_available_transitions(bsystem, bvsystem_t::state_t({ 0, 0, 1, 0, 0 }), std::vector<bvsystem_t::transition_t>({ cd_trans })), "Boolvar System: available transitions");
	assert_test(test_available_transitions(bsystem, bvsystem_t::state_t({ 0, 0, 0, 1, 0 }), std::vector<bvsystem_t::transition_t>({ de_trans })), "Boolvar System: available transitions");
	assert_test(test_available_transitions(bsystem, bvsystem_t::state_t({ 0, 0, 0, 0, 1 }), std::vector<bvsystem_t::transition_t>()), "Boolvar System: available transitions");

	//Test applying transitions
	assert_test(test_apply_transition(bsystem, bvsystem_t::state_t({ 1, 0, 0, 0, 0 }), bvsystem_t::state_t({ 0, 1, 0, 0, 0 }), ab_trans), "Boolvar System: applying transition");
	assert_test(test_apply_transition(bsystem, bvsystem_t::state_t({ 0, 1, 0, 0, 0 }), bvsystem_t::state_t({ 0, 0, 1, 0, 0 }), bc_trans), "Boolvar System: applying transition");
	assert_test(test_apply_transition(bsystem, bvsystem_t::state_t({ 0, 0, 1, 0, 0 }), bvsystem_t::state_t({ 0, 0, 0, 1, 0 }), cd_trans), "Boolvar System: applying transition");
	assert_test(test_apply_transition(bsystem, bvsystem_t::state_t({ 0, 0, 0, 1, 0 }), bvsystem_t::state_t({ 0, 0, 0, 0, 1 }), de_trans), "Boolvar System: applying transition");

	//Difference
	assert_test(bsystem.difference(bvsystem_t::state_t({ 1, 0, 0, 0, 0 }), bvsystem_t::state_t({ 0, 1, 0, 0, 0 })) == ab_trans, "Boolvar System: state difference");
	assert_test(bsystem.difference(bvsystem_t::state_t({ 0, 1, 0, 0, 0 }), bvsystem_t::state_t({ 0, 0, 1, 0, 0 })) == bc_trans, "Boolvar System: state difference");
	assert_test(bsystem.difference(bvsystem_t::state_t({ 0, 0, 1, 0, 0 }), bvsystem_t::state_t({ 0, 0, 0, 1, 0 })) == cd_trans, "Boolvar System: state difference");
	assert_test(bsystem.difference(bvsystem_t::state_t({ 0, 0, 0, 1, 0 }), bvsystem_t::state_t({ 0, 0, 0, 0, 1 })) == de_trans, "Boolvar System: state difference");

	//Test streamer
}

void test_floatvar_systems()
{
	using fvsystem_t = transition_system<floatvar_system>;

	/*
	Simple transition system. {5 floats}
	*/
	fvsystem_t bsystem(5);
}

void test_multivar_systems()
{
	test_boolvar_systems();
	test_floatvar_systems();
}