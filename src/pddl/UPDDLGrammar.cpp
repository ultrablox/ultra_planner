#include "UPDDLGrammar.h"

const char * pddl_31_language = "<domain> ::= (define (domain <name>) <require-def> <types-def> <constants-def> <predicates-def> <functions-def> <constraints>)\n";
	/*							"<require-def> ::= (:requirements <require-key>)|" SPECIAL_SYMBOL_EMPTY "\n"
								"<require-key> ::= <require-key>|<:fluents>|<:adl>|<:quantified-preconditions>|:strips|:typing|:negative-preconditions|:disjunctive-preconditions|:equality|:existential-preconditions|:universal-preconditions|:conditional-effects|:numeric-fluents|:durative-actions|:duration-inequalities|:continuous-effects|:derived-predicates|:timed-initial-literals|:preferences|:constraints|:action-costs|" SPECIAL_SYMBOL_EMPTY "\n"
								"<:fluents> = :numeric-fluents + :object-fluents\n"
								"<:adl> = :strips + :typing + :negative-preconditions + :disjunctive-preconditions + :equality + :quantified-preconditions + :conditional-effects\n"
								"<:quantified-preconditions> = :existential-preconditions + :universal-preconditions\n"
								"<types-def> ::= (:types <typed list (name)>)|" SPECIAL_SYMBOL_EMPTY "\n"
								"<constants-def> ::= (:constants <typed list (name)>)|" SPECIAL_SYMBOL_EMPTY "\n"
								"<predicates-def> ::= (:predicates <atomic formula skeleton>+)|" SPECIAL_SYMBOL_EMPTY "\n"
								"<atomic formula skeleton> ::= (<predicate> <typed list (variable)>)\n"
								"<predicate> ::= <name>\n"
								"<variable> ::= ?<name>\n"
								"<atomic function skeleton> ::= (<function-symbol> <typed list (variable)>)\n"
								"<function-symbol> ::= <name>\n"
								"<functions-def> ::=:fluents (:functions <function typed list (atomic function skeleton)>)|" SPECIAL_SYMBOL_EMPTY "\n"
								"<function typed list (x)> ::= x+ - <function type> <function typed list(x)>\n"
								"<function typed list (x)> ::= " SPECIAL_SYMBOL_EMPTY " \n"
								"<function typed list (x)> ::=:numeric-fluents x+\n"
								"<function type> ::=:numeric-fluents number\n"
								"<function type> ::=:typing + :object-fluents <type>\n"
								"<constraints> ::=:constraints (:constraints <con-GD>)|" SPECIAL_SYMBOL_EMPTY "\n"
								"<structure-def> ::= <action-def>\n"
								"<structure-def> ::=:durative-actions <durative-action-def>\n"
								"<structure-def> ::=:derived-predicates <derived-def>\n"
								"<typed list (x)> ::= {x}\n"
								"<typed list (x)> ::=:typing x+ - <type> <typed list(x)>\n"
								"<primitive-type> ::= <name>\n"
								"<primitive-type> ::= object\n"
								"<type> ::= (either <primitive-type>+)\n"
								"<type> ::= <primitive-type>\n"
								"<emptyOr (x)> ::= ()\n"
								"<emptyOr (x)> ::= x\n"
								"<action-def> ::= (:action <action-symbol> :parameters (<typed list (variable)>) <action-def body>)\n"
								"<action-symbol> ::= <name>\n"
								"<action-def body> ::= <action-def body _1> <action-def body _2>\n"
								"<action-def body _1> ::= :precondition <emptyOr (pre-GD)>|" SPECIAL_SYMBOL_EMPTY "\n"
								"<action-def body _2> ::= :effect <emptyOr (effect)>|" SPECIAL_SYMBOL_EMPTY "\n"
								"<pre-GD> ::= <pref-GD>\n"
								"<pre-GD> ::= (and {<pre-GD>})\n"
								"<pre-GD> ::=:universal-preconditions (forall (<typed list(variable)>) <pre-GD>)\n"
								"<pref-GD> ::=:preferences (preference <pref-name> <GD>)\n"
								"<pref-GD> ::= <GD>\n"
								"<pref-name> ::= <name>|" SPECIAL_SYMBOL_EMPTY "\n"
								"<GD> ::= <atomic formula(term)>\n"
								"<GD> ::=:negative-preconditions <literal(term)>\n"
								"<GD> ::= (and {<GD>})\n"
								"<GD> ::=:disjunctive-preconditions (or {<GD>})\n"
								"<GD> ::=:disjunctive-preconditions (not <GD>)\n"
								"<GD> ::=:disjunctive-preconditions (imply <GD> <GD>)\n"
								"<GD> ::=:existential-preconditions (exists (<typed list(variable)>) <GD> )\n"
								"<GD> ::=:universal-preconditions (forall (<typed list(variable)>) <GD> )\n"
								"<GD> ::=:numeric-fluents <f-comp>\n"
								"<f-comp> ::= (<binary-comp> <f-exp> <f-exp>)\n"
								"<literal(t)> ::= <atomic formula(t)>\n"
								"<literal(t)> ::= (not <atomic formula(t)>)\n"
								"<atomic formula(t)> ::= (<predicate> {t})\n"
								"<atomic formula(t)> ::=:equality (= t t)\n"
								"<term> ::= <name>\n"
								"<term> ::= <variable>\n"
								"<term> ::=:object-fluents <function-term>\n"
								"<function-term> ::=:object-fluents (<function-symbol> {<term>})\n"
								"<f-exp> ::=:numeric-fluents <number>\n"
								"<f-exp> ::=:numeric-fluents (<binary-op> <f-exp> <f-exp>)\n"
								"<f-exp> ::=:numeric-fluents (<multi-op> <f-exp> <f-exp>+)\n"
								"<f-exp> ::=:numeric-fluents (- <f-exp>)\n"
								"<f-exp> ::=:numeric-fluents <f-head>\n"
								"<f-head> ::= (<function-symbol> {<term>})\n"
								"<f-head> ::= <function-symbol>\n"
								"<binary-op> ::= <multi-op>\n"
								"<binary-op> ::= -\n"
								"<binary-op> ::= /\n"
								"<multi-op> ::= *\n"
								"<multi-op> ::= +\n"
								"<binary-comp> ::= " SPECIAL_SYMBOL_GREATER "\n"
								"<binary-comp> ::= " SPECIAL_SYMBOL_LESS "\n"
								"<binary-comp> ::= =\n"
								"<binary-comp> ::= " SPECIAL_SYMBOL_LESS "=\n"
								"<binary-comp> ::= " SPECIAL_SYMBOL_GREATER "=\n"
								"<name> ::= <letter> {<any char>}\n"
								"<letter> ::= a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z\n"
								"<any char> ::= <letter> | <digit> | - | _\n"
								"<number> ::= <digit>+ <decimal _opt>\n"
								"<digit> ::= 0|1|2|3|4|5|6|7|8|9\n"
								"<decimal _opt> ::= <decimal>|" SPECIAL_SYMBOL_EMPTY "\n"
								"<decimal> ::= .<digit>+\n"
								"<effect> ::= (and {<c-effect>})\n"
								"<effect> ::= <c-effect>\n"
								"<c-effect> ::=:conditional-effects (forall (<typed list (variable)>) <effect>)\n"
								"<c-effect> ::=:conditional-effects (when <GD> <cond-effect>)\n"
								"<c-effect> ::= <p-effect>\n"
								"<p-effect> ::= (not <atomic formula(term)>)\n"
								"<p-effect> ::= <atomic formula(term)>\n"
								"<p-effect> ::=:numeric-fluents (<assign-op> <f-head> <f-exp>)\n"
								"<p-effect> ::=:object-fluents (assign <function-term> <term>)\n"
								"<p-effect> ::=:object-fluents (assign <function-term> undefined)\n"
								"<cond-effect> ::= (and {<p-effect>})\n"
								"<cond-effect> ::= <p-effect>\n"
								"<assign-op> ::= assign\n"
								"<assign-op> ::= scale-up\n"
								"<assign-op> ::= scale-down\n"
								"<assign-op> ::= increase\n"
								"<assign-op> ::= decrease\n"
								"<durative-action-def> ::= (:durative-action <da-symbol> :parameters (<typed list (variable)>) <da-def body>)\n"
								"<da-symbol> ::= <name>\n"
								"<da-def body> ::= :duration <duration-constraint> :condition <emptyOr (da-GD)> :effect <emptyOr (da-effect)>\n"
								"<da-GD> ::= <pref-timed-GD>\n"
								"<da-GD> ::= (and {<da-GD>})\n"
								"<da-GD> ::=:universal-preconditions (forall (<typed-list (variable)>) <da-GD>)\n"
								"<pref-timed-GD> ::= <timed-GD>\n"
								"<pref-timed-GD> ::=:preferences (preference <pref-name> <timed-GD>)\n"
								"<timed-GD> ::= (at <time-specifier> <GD>)\n"
								"<timed-GD> ::= (over <interval> <GD>)\n"
								"<time-specifier> ::= start\n"
								"<time-specifier> ::= end\n"
								"<interval> ::= all\n"
								"<duration-constraint> ::=:duration-inequalities (and <simple-duration-constraint>+)\n"
								"<duration-constraint> ::= ()\n"
								"<duration-constraint> ::= <simple-duration-constraint>\n"
								"<simple-duration-constraint> ::= (<d-op> ?duration <d-value>)\n"
								"<simple-duration-constraint> ::= (at <time-specifier> <simple-duration-constraint>)\n"
								"<d-op> ::=:duration-inequalities " SPECIAL_SYMBOL_LESS "=\n"
								"<d-op> ::=:duration-inequalities " SPECIAL_SYMBOL_GREATER "=\n"
								"<d-op> ::= =\n"
								"<d-value> ::= <number>\n"
								"<d-value> ::=:numeric-fluents <f-exp>\n"
								"<da-effect> ::= (and {<da-effect>})\n"
								"<da-effect> ::= <timed-effect>\n"
								"<da-effect> ::=:conditional-effects (forall (<typed list (variable)>) <da-effect>)\n"
								"<da-effect> ::=:conditional-effects (when <da-GD> <timed-effect>)\n"
								"<timed-effect> ::= (at <time-specifier> <cond-effect>)\n"
								"<timed-effect> ::=:numeric-fluents (at <time-specifier> <f-assign-da>)\n"
								"<timed-effect> ::=:continuous-effects + :numeric-fluents (<assign-op-t> <f-head> <f-exp-t>)\n"
								"<f-assign-da> ::= (<assign-op> <f-head> <f-exp-da>)\n"
								"<f-exp-da> ::= (<binary-op> <f-exp-da> <f-exp-da>)\n"
								"<f-exp-da> ::= (<multi-op> <f-exp-da> <f-exp-da>+)\n"
								"<f-exp-da> ::= (- <f-exp-da>)\n"
								"<f-exp-da> ::=:duration-inequalities ?duration\n"
								"<f-exp-da> ::= <f-exp>\n"
								"<assign-op-t> ::= increase\n"
								"<assign-op-t> ::= decrease\n"
								"<f-exp-t> ::= (* <f-exp> #t)\n"
								"<f-exp-t> ::= (* #t <f-exp>)\n"
								"<f-exp-t> ::= #t\n"
								"<derived-def> ::= (:derived <atomic formula skeleton> <GD>)\n";*/

UPDDLGrammar::UPDDLGrammar()
	:UGrammar()
{

	/*
	<predicate> ::= <name>
	*/
//	createRule("<predicate>", SPECIAL_SYMBOL_IDENTIFIER, 0);

	/*
	<variable> ::= ?<name>
	*/
//	createRule("<variable>", SPECIAL_SYMBOL_IDENTIFIER, 0);


	/*
	<function-symbol> ::= <name>
	*/
//	createRule("<function-symbol>", SPECIAL_SYMBOL_IDENTIFIER, 0);

	/*
	<action-def> ::= (:action <action-symbol>
						:parameters (<typed list (variable)>)
						<action-def body>)
						*/
//	createRule("<action-def>", "(", ":action", "<action-symbol>", ":parameters", "<typed list (variable)>",/* "<action-def body>",*/ ")", (const char*)0);

	/*
	<action-symbol> ::= <name>
	*/
//	createRule("<action-symbol>", SPECIAL_SYMBOL_IDENTIFIER, 0);

	/*
	<action-def body> ::=	[:precondition <emptyOr (pre-GD)>]
							[:effect <emptyOr (effect)>]

	=====>

	<action-def body> ::= <action-def body precondition> <action-def body effect>
	<action-def body precondition> ::= [:precondition <emptyOr (pre-GD)>]
	<action-def body precondition> ::= EPS
	<action-def body effect> ::= [:effect <emptyOr (effect)>]
	<action-def body effect> ::= EPS

	*/
//	createRule("<action-def body>", "<action-def body precondition>", "<action-def body effect>", 0);

//	createRule("<action-def body precondition>", "[", ":precondition", "<emptyOr (pre-GD)>", "]", 0);
//	createRule("<action-def body precondition>", SPECIAL_SYMBOL_EMPTY, 0);

//	createRule("<action-def body effect>", "[", ":effect", "<emptyOr (effect)>", "]", 0);
//	createRule("<action-def body effect>", SPECIAL_SYMBOL_EMPTY, 0);


	/*
	<emptyOr (X)> ::= () | <X>
	*/
	//pre-GD
//	createRule("<emptyOr (pre-GD)>", "(", ")", 0);
//	createRule("<emptyOr (pre-GD)>", "<pre-GD>", 0);
	
	//effect
//	createRule("<emptyOr (effect)>", "(", ")", 0);
//	createRule("<emptyOr (effect)>", "<effect>", 0);

	/*
	<pre-GD> ::= <pref-GD> | (and <pre-GD>*) 
	======>
	<pre-GD> ::= <pref-GD> | (and <pre-GDs>)
	<pre-GDs> ::= <pref-GD> <pre-GDs> | EPS
	*/
//	createRule("<pre-GD>", "<pref-GD>", 0);
//	createRule("<pre-GD>", "(", "and", "<pre-GDs>", ")", 0);
//	createRule("<pre-GDs>", "<pref-GD>", "<pre-GDs>", 0);
//	createRule("<pre-GDs>", SPECIAL_SYMBOL_EMPTY, 0);

	/*
	<pref-GD> ::= <GD>
	*/
//	createRule("<pref-GD>", "<GD>", 0);

	/*
	<GD> ::= <atomic formula(term)>
	*/
//	createRule("<GD>", "<atomic formula(term)>", 0);

	/*
	<typed list (variable)> ::= <variable>*
	=====>
	<typed list (variable)> ::= <variables>
	<variables> ::= <variable> <variables> | EPSILON
	*/
//	createRule("<typed list (variable)>", "<variables>", 0);
//	createRule("<variables>", "<variable>", "<variables>", 0);
//	createRule("<variables>", SPECIAL_SYMBOL_EMPTY, 0);


	/*
	<atomic formula(X)> ::= (<predicate> X*)
	=====>
	<atomic formula(X)> ::= (<predicate> <Xs>)
	<Xs> ::= <term> <Xs> | EPS
	*/
	//<atomic formula(term)>
//	createRule("<atomic formula(term)>", "(", "<predicate>", "<terms>", ")", 0);
//	createRule("<terms>", "<term>", "<terms>", 0);
//	createRule("<terms>", SPECIAL_SYMBOL_EMPTY, 0);


	/*
	<term> ::= <name>
	<term> ::= <variable>
	*/
//	createRule("<term>", SPECIAL_SYMBOL_IDENTIFIER, 0);
//	createRule("<term>", "<variable>", 0);

	/*
	<effect> ::= (and <c-effect>*)
	<effect> ::= <c-effect>
	==========>
	<effect> ::= <c-effect>
	<effect> ::= (and <c-effects>)
	<c-effects> ::= <c-effect> <c-effects> | EPS
	*/
//	createRule("<effect>", "<c-effect>", 0);
//	createRule("<effect>", "(", "and", "<c-effects>", ")", 0);
//	createRule("<c-effects>", "<c-effect>", "<c-effects>", 0);
//	createRule("<c-effects>", SPECIAL_SYMBOL_IDENTIFIER, 0);

	/*
	<c-effect> ::= <p-effect>
	*/
//	createRule("<c-effect>", "<p-effect>", 0);

	/*
	<p-effect> ::= (not <atomic formula(term)>)
	<p-effect> ::= <atomic formula(term)>
	*/
//	createRule("<p-effect>", "<atomic formula(term)>", 0);
//	createRule("<p-effect>", "(", "not", "<atomic formula(term)>", ")", 0);
}
