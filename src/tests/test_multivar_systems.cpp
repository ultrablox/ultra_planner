#include "test_helpers.h"
#include "transition_system_helpers.h"
#include <core/varset_system/boolvar_system.h>
#include <core/varset_system/floatvar_system.h>
#include <core/varset_system/multivar_system.h>
#include <core/varset_system/combinedvar_system.h>
#include <core/transition_system.h>

/*
bool operator<(const boolvar_transition & lhs, const boolvar_transition & rhs)
{
	return lhs.name < rhs.name;
}

bool operator<(const floatvar_transition & lhs, const floatvar_transition & rhs)
{
	return lhs.name < rhs.name;
}*/


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

	//bvsystem_t::transition_t ab_trans(2, 3);

	bvsystem_t::transition_t ab_trans(bit_vector({ 1, 0, 0, 0, 0 }), bit_vector({ 1, 0, 0, 0, 0 }), bit_vector({ 0, 1, 0, 0, 0 }), bit_vector({ 1, 1, 0, 0, 0 }));
	ab_trans.name = "a->b";
	bsystem.add_transition(ab_trans); //a -> b, !a

	bvsystem_t::transition_t bc_trans(bit_vector({ 0, 1, 0, 0, 0 }), bit_vector({ 0, 1, 0, 0, 0 }), bit_vector({ 0, 0, 1, 0, 0 }), bit_vector({ 0, 1, 1, 0, 0 }));
	bc_trans.name = "b->c";
	bsystem.add_transition(bc_trans); //b -> c, !b

	bvsystem_t::transition_t cd_trans(bit_vector({ 0, 0, 1, 0, 0 }), bit_vector({ 0, 0, 1, 0, 0 }), bit_vector({ 0, 0, 0, 1, 0 }), bit_vector({ 0, 0, 1, 1, 0 }));
	cd_trans.name = "c->d";
	bsystem.add_transition(cd_trans); //c -> d, !d

	bvsystem_t::transition_t de_trans(bit_vector({ 0, 0, 0, 1, 0 }), bit_vector({ 0, 0, 0, 1, 0 }), bit_vector({ 0, 0, 0, 0, 1 }), bit_vector({ 0, 0, 0, 1, 1 }));
	de_trans.name = "d->e";
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

	fvsystem_t::transition_t incA_trans(5, std::vector<fvsystem_t::transition_t::initialize_element>({ make_tuple(0, floatvar_transition_base::effect_type_t::Increase, 3.0) }));
	incA_trans.name = "a += 3";
	fsystem.add_transition(incA_trans);

	fvsystem_t::transition_t decA_trans(5, std::vector<fvsystem_t::transition_t::initialize_element>({ make_tuple(0, floatvar_transition_base::effect_type_t::Decrease, 2.0) }));
	decA_trans.name = "a -= 2";
	fsystem.add_transition(decA_trans);

	//Test available transitions
	assert_test(test_available_transitions(fsystem, fvsystem_t::state_t({ 0.0f, 0.0f, 0.0f }), std::vector<fvsystem_t::transition_t>({ incA_trans, decA_trans })), "Floatvar System: available transitions");

	//Test applying transitions
	assert_test(test_apply_transition(fsystem, fvsystem_t::state_t({ 1.0, 1.0, 1.0 }), fvsystem_t::state_t({ 4.0, 1.0, 1.0 }), incA_trans), "Floatvar System: applying transition");
	assert_test(test_apply_transition(fsystem, fvsystem_t::state_t({ 1.0, 1.0, 1.0 }), fvsystem_t::state_t({ -1.0, 1.0, 1.0 }), decA_trans), "Floatvar System: applying transition");

	//Difference
	assert_test(fsystem.difference(fvsystem_t::state_t({ 1.0, 1.0, 1.0 }), fvsystem_t::state_t({ 4.0, 1.0, 1.0 })) == incA_trans, "Floatvar System: state difference");
	assert_test(fsystem.difference(fvsystem_t::state_t({ 1.0, 1.0, 1.0 }), fvsystem_t::state_t({ -1.0, 1.0, 1.0 })) == decA_trans, "Floatvar System: state difference");
}

void test_multivar_systems()
{
	using mvsystem_t = transition_system<multivar_system>;
	mvsystem_t msystem;// ({ "V1", "V2", "V3" });
	msystem.add_var("V1", { "A", "B", "C" });
	msystem.add_var("V2", { "A", "B" });
	msystem.add_var("V3", { "A", "B", "C", "D", "E" });

	msystem.build();

	auto v1_trans_AB = msystem.create_transition({ make_tuple("V1", "A", "B"), make_tuple("V1", "A", "B") });
	v1_trans_AB.name = "V1: A->B";
	msystem.add_transition(v1_trans_AB);

	auto v1_trans_AC = msystem.create_transition({ make_tuple("V1", "A", "C") });
	v1_trans_AC.name = "V1: A->C";
	msystem.add_transition(v1_trans_AC);

	auto v2_trans_BA = msystem.create_transition({ make_tuple("V2", "B", "A") });
	v2_trans_BA.name = "V2: B->A";
	msystem.add_transition(v2_trans_BA);

	//Test available transitions
	assert_test(test_available_transitions(msystem, msystem.create_state({ "A", "A", "A" }), std::vector<mvsystem_t::transition_t>({ v1_trans_AB, v1_trans_AC })), "Multivar System: available transitions");
	assert_test(test_available_transitions(msystem, msystem.create_state({ "C", "B", "A" }), std::vector<mvsystem_t::transition_t>({ v2_trans_BA })), "Multivar System: available transitions");


	//Test applying transitions
	assert_test(test_apply_transition(msystem, msystem.create_state({ "A", "A", "A" }), msystem.create_state({ "C", "A", "A" }), v1_trans_AC), "Multivar System: applying transition");
	assert_test(test_apply_transition(msystem, msystem.create_state({ "C", "B", "A" }), msystem.create_state({ "C", "A", "A" }), v2_trans_BA), "Multivar System: applying transition");
}

void test_combined_systems()
{
	using cvsystem_t = transition_system<combinedvar_system>;
	
	cvsystem_t csystem(5, 3);	//5 bools + 3 floats

	cvsystem_t::transition_t ab_trans(boolvar_transition_base({ 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0 }, { 0, 1, 0, 0, 0 }, { 1, 1, 0, 0, 0 }), floatvar_transition_base(5, { make_tuple(0, floatvar_transition_base::effect_type_t::Decrease, 2.0) }));
	ab_trans.name = "a->b, !a, A -= 2.0";
	csystem.add_transition(ab_trans);

	cvsystem_t::transition_t bc_trans(boolvar_transition_base({ 0, 1, 0, 0, 0 }, { 0, 1, 0, 0, 0 }, { 0, 0, 1, 0, 0 }, { 0, 1, 1, 0, 0 }), floatvar_transition_base(5, { make_tuple(1, floatvar_transition_base::effect_type_t::Assign, 1.0) }));
	bc_trans.name = "b->c, !c, B = 1.0";
	csystem.add_transition(bc_trans);

	cvsystem_t::transition_t cd_trans(boolvar_transition_base({ 0, 0, 1, 0, 0 }, { 0, 0, 1, 0, 0 }, { 0, 0, 0, 1, 0 }, { 0, 0, 1, 1, 0 }), floatvar_transition_base(5, { make_tuple(1, floatvar_transition_base::effect_type_t::Increase, 0.5) }));
	cd_trans.name = "c->d, !c, B += 0.5";
	csystem.add_transition(cd_trans);

	csystem.build_transitions_index();

	//Test available transitions
	assert_test(test_available_transitions(csystem, cvsystem_t::state_t({ 1, 0, 0, 0, 0 }, { 3.0f, 2.0f, 6.0f}), std::vector<cvsystem_t::transition_t>({ ab_trans })), "Combined Var System: available transitions");
	assert_test(test_available_transitions(csystem, cvsystem_t::state_t({ 0, 1, 0, 0, 0 }, { 3.0f, 2.0f, 6.0f }), std::vector<cvsystem_t::transition_t>({ bc_trans })), "Combined Var System: available transitions");
	assert_test(test_available_transitions(csystem, cvsystem_t::state_t({ 0, 0, 1, 0, 0 }, { 3.0f, 2.0f, 6.0f }), std::vector<cvsystem_t::transition_t>({ cd_trans })), "Combined Var System: available transitions");

	//Test applying transitions
	assert_test(test_apply_transition(csystem, cvsystem_t::state_t({ 1, 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f }), cvsystem_t::state_t({ 0, 1, 0, 0, 0 }, { -1.0f, 1.0f, 1.0f }), ab_trans), "Combined Var System: applying transition");
	assert_test(test_apply_transition(csystem, cvsystem_t::state_t({ 0, 1, 0, 0, 0 }, { 0.0f, 0.0f, 0.0f }), cvsystem_t::state_t({ 0, 0, 1, 0, 0 }, { 0.0f, 1.0f, 0.0f }), bc_trans), "Combined Var System: applying transition");
	assert_test(test_apply_transition(csystem, cvsystem_t::state_t({ 0, 0, 1, 0, 0 }, { 1.0f, 1.0f, 1.0f }), cvsystem_t::state_t({ 0, 0, 0, 1, 0 }, { 1.0f, 1.5f, 1.0f }), cd_trans), "Combined Var System: applying transition");

	//Difference
	assert_test(csystem.difference(cvsystem_t::state_t({ 1, 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f }), cvsystem_t::state_t({ 0, 1, 0, 0, 0 }, { -1.0f, 1.0f, 1.0f })) == ab_trans, "Combined Var System: state difference");
	assert_test(csystem.difference(cvsystem_t::state_t({ 0, 1, 0, 0, 0 }, { 0.0f, 0.0f, 0.0f }), cvsystem_t::state_t({ 0, 0, 1, 0, 0 }, { 0.0f, 1.0f, 0.0f })) == bc_trans, "Combined Var System: state difference");
	assert_test(csystem.difference(cvsystem_t::state_t({ 0, 0, 1, 0, 0 }, { 1.0f, 1.0f, 1.0f }), cvsystem_t::state_t({ 0, 0, 0, 1, 0 }, { 1.0f, 1.5f, 1.0f })) == cd_trans, "Combined Var System: state difference");
}

void test_varset_systems()
{
	test_boolvar_systems();
	test_floatvar_systems();
	test_multivar_systems();
	test_combined_systems();
}