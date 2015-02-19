#include "test_helpers.h"
#include "transition_system_helpers.h"
#include <core/varset_system/boolvar_system.h>
#include <core/varset_system/floatvar_system.h>
#include <core/varset_system/combinedvar_system.h>
#include <core/transition_system.h>


bool operator<(const boolvar_transition & lhs, const boolvar_transition & rhs)
{
	return lhs.name < rhs.name;
}

bool operator<(const floatvar_transition & lhs, const floatvar_transition & rhs)
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
	bvsystem_t::transition_t ab_trans("a->b", { 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0 }, { 0, 1, 0, 0, 0 }, { 1, 1, 0, 0, 0 });
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
	Simple transition system. {3 floats}
	*/
	fvsystem_t fsystem(3);

	fvsystem_t::transition_t incA_trans(5, { make_tuple(0, floatvar_transition::effect_type_t::Increase, 3.0) }, "a += 3");
	fsystem.add_transition(incA_trans);

	fvsystem_t::transition_t decA_trans(5, { make_tuple(0, floatvar_transition::effect_type_t::Decrease, 2.0) }, "a -= 2");
	fsystem.add_transition(decA_trans);

	//Test available transitions
	assert_test(test_available_transitions(fsystem, fvsystem_t::state_t({ 0.0f, 0.0f, 0.0f }), std::vector<fvsystem_t::transition_t>({ incA_trans, decA_trans })), "Floatvar System: available transitions");

	//Test applying transitions
	assert_test(test_apply_transition(fsystem, fvsystem_t::state_t({ 1.0, 1.0, 1.0 }), fvsystem_t::state_t({ 4.0, 1.0, 1.0 }), incA_trans), "Floatvar System: applying transition");
	assert_test(test_apply_transition(fsystem, fvsystem_t::state_t({ 1.0, 1.0, 1.0 }), fvsystem_t::state_t({ -1.0, 1.0, 1.0 }), decA_trans), "Floatvar System: applying transition");
}

void test_combined_systems()
{
	using cvsystem_t = transition_system<combinedvar_system>;
	
	cvsystem_t csystem(5, 3);	//5 bools + 3 floats

//	cvsystem_t::transition_t combined_transition({ boolvar_transition({ 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0 }, { 0, 1, 0, 0, 0 }, { 1, 1, 0, 0, 0 }), floatvar_transition(5, { make_tuple(0, floatvar_transition::effect_type_t::Decrease, 2.0) }) });
}

void test_multivar_systems()
{
	test_boolvar_systems();
	test_floatvar_systems();
	test_combined_systems();
}