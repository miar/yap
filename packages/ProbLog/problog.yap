%%% -*- Mode: Prolog; -*-
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ProbLog inference
% 
% assumes probabilistic facts as Prob::Fact and clauses in normal Prolog format
%
% provides following inference modes (16/12/2008):
% - approximation with interval width Delta (IJCAI07): problog_delta(+Query,+Delta,-Low,-High,-Status)
% - bounds based on single probability threshold: problog_threshold(+Query,+Threshold,-Low,-High,-Status)
% - as above, but lower bound only: problog_low(+Query,+Threshold,-Low,-Status)
% - lower bound based on K most likely proofs: problog_kbest(+Query,+K,-Low,-Status)
% - explanation probability (ECML07): problog_max(+Query,-Prob,-FactsUsed)
% - exact probability: problog_exact(+Query,-Prob,-Status)
% - sampling: problog_montecarlo(+Query,+Delta,-Prob)
%
%
% angelika.kimmig@cs.kuleuven.be
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
:- module(problog, [problog_delta/5,
	problog_threshold/5,
	problog_low/4,
	problog_kbest/4,
	problog_kbest_save/6,
	problog_max/3,
	problog_exact/3,
	problog_montecarlo/3,
	get_fact_probability/2,
	set_fact_probability/2,
	get_fact/2,
	tunable_fact/2,
	non_ground_fact/1,
	export_facts/1,
	problog_help/0,
	problog_dir/1,
	set_problog_flag/2,
	problog_flag/2,
	problog_flags/0]).

:- style_check(all).
:- yap_flag(unknown,error).

% problog related modules
:- use_module('problog/flags',[set_problog_flag/2,
	problog_flag/2,
	problog_flags/0]).

:- use_module('problog/print', [print_sep_line/0,
	print_inference/2]).

:- use_module('problog/tptree',[init_ptree/1,
	delete_ptree/1,
	insert_ptree/2,
	count_ptree/2,
	prune_check_ptree/2,
	merge_ptree/3,
	bdd_ptree_map/4,
	bdd_ptree/3]).

% general yap modules
:- ensure_loaded(library(lists)).
:- ensure_loaded(library(terms)).
:- ensure_loaded(library(random)).
:- ensure_loaded(library(system)).
:- ensure_loaded(library(rbtrees)).

% op attaching probabilities to facts
:- op( 550, yfx, :: ).

%%%%%%%%%%%%%%%%%%%%%%%%
% control predicates on various levels
%%%%%%%%%%%%%%%%%%%%%%%%

% global over all inference methods, internal use only
:- dynamic problog_predicate/2.
% global over all inference methods, exported
:- dynamic tunable_fact/2.
:- dynamic non_ground_fact/1.
:- dynamic problog_dir/1.
% global, manipulated via problog_control/2
:- dynamic up/0.
:- dynamic limit/0.
:- dynamic mc/0.
:- dynamic remember/0.
% local to problog_delta
:- dynamic low/2.
:- dynamic up/2.
:- dynamic stopDiff/1.
% local to problog_kbest
:- dynamic current_kbest/3.
% local to problog_max
:- dynamic max_probability/1.
:- dynamic max_proof/1.
% local to problog_montecarlo
:- dynamic mc_prob/1.
% to keep track of the groundings for non-ground facts
:- dynamic grounding_is_known/2.

% for fact where the proabability is a variable
:- dynamic dynamic_probability_fact/1.
:- dynamic dynamic_probability_fact_extract/2.

% directory where ProblogBDD executable is located
% automatically set during loading -- assumes it is in same place as this file (problog.yap)
%:- getcwd(PD),retractall(problog_dir(_)),assert(problog_dir(PD)).
:- yap_flag(shared_object_search_path,PD),retractall(problog_dir(_)),assert(problog_dir(PD)).

%%%%%%%%%%%%%%%%%%%%%%%%
% help
%%%%%%%%%%%%%%%%%%%%%%%%

problog_help :-
	format('~2nProbLog inference currently offers the following inference methods:~n',[]),
	show_inference,
	format('~2nThe following global parameters are available:~n',[]),
	problog_flags,
	print_sep_line,
	format('~n     use problog_help/0 to display this information~n',[]),
	format('~n     use problog_flags/0 to display current parameter values~2n',[]),
	print_sep_line,
	nl,
	flush_output.

show_inference :-
	format('~n',[]),
	print_sep_line,
	print_inference(call,description),
	print_sep_line,
	print_inference('problog_delta(+Query,+Delta,-Low,-High,-Status)','approximation with interval width Delta (IJCAI07)'), 
	print_inference('problog_threshold(+Query,+Threshold,-Low,-High,-Status)','bounds based on single probability threshold'), 
	print_inference('problog_low(+Query,+Threshold,-Low,-Status)','lower bound based on single probability threshold'), 
	print_inference('problog_kbest(+Query,+K,-Low,-Status)','lower bound based on K most likely proofs'), 
	print_inference('problog_max(+Query,-Prob,-FactsUsed)','explanation probability (ECML07)'),
	print_inference('problog_exact(+Query,-Prob,-Status)','exact probability'),
	print_inference('problog_montecarlo(+Query,+Delta,-Prob)','sampling with 95\%-confidence-interval-width Delta'),
	print_sep_line.
	
%%%%%%%%%%%%%%%%%%%%%%%%
% initialization of global parameters
%%%%%%%%%%%%%%%%%%%%%%%%

init_global_params :-
	set_problog_flag(bdd_time,60), 
	set_problog_flag(first_threshold,0.1), 
	L is 10**(-30), 
	set_problog_flag(last_threshold,L), 
	set_problog_flag(id_stepsize,0.5), 
	set_problog_flag(prunecheck,off), 
	set_problog_flag(maxsteps,1000),
	set_problog_flag(mc_batchsize,1000),
	set_problog_flag(mc_logfile,'log.txt'),
	set_problog_flag(bdd_file,example_bdd),
	set_problog_flag(dir,output),
	set_problog_flag(save_bdd,false),
%	problog_flags,
	print_sep_line,
	format('~n     use problog_help/0 for information~n',[]),
	format('~n     use problog_flags/0 to display current parameter values~2n',[]),
	print_sep_line,
	nl,
	flush_output.

% parameter initialization to be called after returning to user's directory:
:- initialization(init_global_params).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% internal control flags
% if on
% - up: collect stopped derivations to build upper bound 
% - limit: iterative deepening reached limit -> should go to next level
% - mc: using problog_montecarlo, i.e. proving with current sample instead of full program
% - remember: save BDD files containing script, params and mapping
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
problog_control(on,X) :-
	call(X),!.
problog_control(on,X) :-
	assert(X).
problog_control(off,X) :-
	retractall(X).
problog_control(check,X) :-
	call(X).

:- problog_control(off,up).
:- problog_control(off,mc).
:- problog_control(off,limit).
:- problog_control(off,remember).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% nice user syntax Prob::Fact
% automatic translation to internal hardware access format
%
% probabilities =1 are dropped -> normal Prolog fact
% 
% internal fact representation 
% - prefixes predicate name with problog_
% - adds unique ID as first argument
% - adds logarithm of probability as last argument
% - keeps original arguments in between
%
% for each predicate appearing as probabilistic fact, wrapper clause is introduced:
% - head is most general instance of original fact
% - body is corresponding version of internal fact plus call to add_to_proof/2 to update current state during proving
% example: edge(A,B) :- problog_edge(ID,A,B,LogProb), add_to_proof(ID,LogProb).
%
% dynamic predicate problog_predicate(Name,Arity) keeps track of predicates that already have wrapper clause
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

user:term_expansion(_P::( _Goal :- _Body ), _Error) :-
	throw(error('we do not support this (yet?)!')).

user:term_expansion(P::Goal,Goal) :-
	P \= t(_),
	P =:= 1,
	!.

user:term_expansion(P::Goal, problog:ProbFact) :- 
	copy_term((P,Goal),(P_Copy,Goal_Copy)),
	functor(Goal, Name, Arity),
        atomic_concat([problog_,Name],ProblogName),
	Goal =.. [Name|Args],
	append(Args,[LProb],L1),
	probclause_id(ID),
	ProbFact =.. [ProblogName,ID|L1],
	(
	    (\+ var(P), P = t(TrueProb))
	-> 
	    (
		assert(tunable_fact(ID,TrueProb)),
		LProb is log(0.5)
	    );
	    (
		ground(P)
	    ->
	        LProb is log(P);
		(
		    % Probability is a variable... check wether it appears in the term
		    (
			variable_in_term(Goal,P)
		    ->
		        true;
			(
			    format(user_error,'If you use probabilisitic facts with a variable as probabilility, the variable has to appear inside the fact.~n',[]),
			    format(user_error,'You used ~q in your program.~2n',[P::Goal]),
			    throw(non_ground_fact_error(P::Goal))
			)
		    ),
		    LProb=log(P),
		    assert(dynamic_probability_fact(ID)),
		    assert(dynamic_probability_fact_extract(Goal_Copy,P_Copy))
		)
	    )
        ),
	(
	    ground(Goal)
	->
	    true;
	    assert(non_ground_fact(ID))
	),
	problog_predicate(Name, Arity, ProblogName).
	    

% introduce wrapper clause if predicate seen first time
problog_predicate(Name, Arity, _) :-
	problog_predicate(Name, Arity), !.

problog_predicate(Name, Arity, ProblogName) :-
	functor(OriginalGoal, Name, Arity),
	OriginalGoal =.. [_|Args],
	append(Args,[Prob],L1),
	ProbFact =.. [ProblogName,ID|L1],
	prolog_load_context(module,Mod),
	
	assert( (Mod:OriginalGoal :- ProbFact, 
	                             (
					 non_ground_fact(ID)
				     ->
				         (non_ground_fact_grounding_id(OriginalGoal,G_ID),
					   atomic_concat([ID,'_',G_ID],ID2));
					 ID2=ID
				     ),
				     % take the log of the probability (for non ground facts with variable as probability
				     ProbEval is Prob,
				     add_to_proof(ID2,ProbEval)
		 )),

	assert( (Mod:problog_not(OriginalGoal) :- ProbFact,
	                                          (
						      non_ground_fact(ID)
						  ->
						     ( non_ground_fact_grounding_id(OriginalGoal,G_ID),
						        atomic_concat([ID,'_',G_ID],ID2));
						      ID2=ID
						  ),
						% take the log of the probability (for non ground facts with variable as probability
						  ProbEval is Prob,
						  add_to_proof_negated(ID2,ProbEval)
		 )),
	    
	assert(problog_predicate(Name, Arity)),
	ArityPlus2 is Arity+2,
	dynamic(problog:ProblogName/ArityPlus2).	

% generate next global identifier
probclause_id(ID) :-
	nb_getval(probclause_counter,ID), !,
	C1 is ID+1,
	nb_setval(probclause_counter,C1), !.
probclause_id(0) :-
	nb_setval(probclause_counter,1).

non_ground_fact_grounding_id(Goal,ID) :-
	(
	    ground(Goal)
	->
	    true;
	    (
		format(user_error,'The current program uses non-ground facts.~n', []),
		format(user_error,'If you query those, you may only query fully-grounded versions of the fact.~n',[]),
		format(user_error,'Within the current proof, you queried for ~q which is not ground.~n~n', [Goal]),
		throw(error(non_ground_fact(Goal)))
	    )
	),
	(
	    grounding_is_known(Goal,ID)
	->
	    true;
	    (
		nb_getval(non_ground_fact_grounding_id_counter,ID),
		ID2 is ID+1,
		nb_setval(non_ground_fact_grounding_id_counter,ID2),
		assert(grounding_is_known(Goal,ID))
	    )
	).

reset_non_ground_facts :-
	nb_setval(non_ground_fact_grounding_id_counter,0),
	retractall(grounding_is_known(_,_)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% access/update the probability of ID's fact
% hardware-access version: naively scan all problog-predicates
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_fact_probability(ID,Prob) :-
	get_internal_fact(ID,ProblogTerm,_ProblogName,ProblogArity),
	arg(ProblogArity,ProblogTerm,Log),
	Prob is exp(Log).
set_fact_probability(ID,Prob) :-
	get_internal_fact(ID,ProblogTerm,ProblogName,ProblogArity),
	retract(ProblogTerm),  
	ProblogTerm =.. [ProblogName|ProblogTermArgs],
	nth(ProblogArity,ProblogTermArgs,_,KeepArgs),
	NewLogProb is log(Prob),
	nth(ProblogArity,NewProblogTermArgs,NewLogProb,KeepArgs),
	NewProblogTerm =.. [ProblogName|NewProblogTermArgs],
	assert(NewProblogTerm).

get_internal_fact(ID,ProblogTerm,ProblogName,ProblogArity) :-
	problog_predicate(Name,Arity),   
	atomic_concat([problog_,Name],ProblogName),
	ProblogArity is Arity+2,
	functor(ProblogTerm,ProblogName,ProblogArity),
	arg(1,ProblogTerm,ID),
	call(ProblogTerm).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% writing those facts with learnable parameters to File
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export_facts(File) :-
	tell(File),
	export_facts,
	flush_output,
	told.
export_facts :-
	tunable_fact(ID,_),
	once(write_tunable_fact(ID)),
	fail.
export_facts.

write_tunable_fact(ID) :-
	get_internal_fact(ID,ProblogTerm,ProblogName,ProblogArity),
	ProblogTerm =.. [_Functor,ID|Args],
	atomic_concat('problog_',OutsideFunctor,ProblogName),
	Last is ProblogArity-1,
	nth(Last,Args,LogProb,OutsideArgs),
	OutsideTerm =.. [OutsideFunctor|OutsideArgs],
	Prob is exp(LogProb),
	format('~w :: ~q.~n',[Prob,OutsideTerm]).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% recover fact for given id
% list version not exported (yet?)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_fact(ID,OutsideTerm) :-
	get_internal_fact(ID,ProblogTerm,ProblogName,ProblogArity),
	ProblogTerm =.. [_Functor,ID|Args],
	atomic_concat('problog_',OutsideFunctor,ProblogName),
	Last is ProblogArity-1,
	nth(Last,Args,_LogProb,OutsideArgs),
	OutsideTerm =.. [OutsideFunctor|OutsideArgs].

get_fact_list([],[]).
get_fact_list([ID|IDs],[Fact|Facts]) :-
	(ID=not(X) -> Fact=not(Y); Fact=Y, ID=X),
	get_fact(X,Y),
	get_fact_list(IDs,Facts).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ProbLog inference, core methods
% 
% state of proving saved in two backtrackable global variables
% - problog_current_proof holds list of IDs of clauses used
% - problog_probability holds the sum of their log probabilities
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% called "inside" probabilistic facts to update current state of proving
% if number of steps exceeded, fail
% if fact used before, succeed and keep status as is
% if not prunable, calculate probability and 
%    if threshold exceeded, add stopped derivation to upper bound and fail 
%       else update state and succeed
add_to_proof(ID,Prob) :-
	montecarlo_check(ID),
	b_getval(problog_steps,MaxSteps),
	b_getval(problog_probability, CurrentP),
	nb_getval(problog_threshold, CurrentThreshold),
	b_getval(problog_current_proof, IDs),

%%%% Bernd, changes for negated ground facts
        \+ memberchk(not(ID),IDs),
%%%% Bernd, changes for negated ground facts

	( MaxSteps =< 0 -> 
	    fail
	;
	  ( memberchk(ID, IDs) ->
	    true
	  ;
	    \+ prune_check([ID|IDs],1),
	    multiply_probabilities(CurrentP, Prob, NProb),
	    ( NProb < CurrentThreshold ->
	      upper_bound([ID|IDs]),
	      fail
	    ;
	      b_setval(problog_probability, NProb),
	      b_setval(problog_current_proof, [ID|IDs])
	    )
	  ),
	  Steps is MaxSteps-1,
	  b_setval(problog_steps,Steps)
	).

%%%% Bernd, changes for negated ground facts
add_to_proof_negated(ID,Prob) :-
	(
	    problog_control(check,mc)
	->
	% the sample has to fail if the fact is negated
	    \+ montecarlo_check(ID);
	    true
	),
	b_getval(problog_steps,MaxSteps),
	b_getval(problog_probability, CurrentP),
	nb_getval(problog_threshold, CurrentThreshold),
	b_getval(problog_current_proof, IDs),

        \+ memberchk(ID,IDs),
	( MaxSteps =< 0 -> 
	    fail
	;
	  ( memberchk(not(ID), IDs) ->
	    true
	  ;
%	    \+ prune_check([ID|IDs],1),
	    InverseProb is log(1 - exp(Prob)),
	    multiply_probabilities(CurrentP, InverseProb, NProb),
	    ( NProb < CurrentThreshold ->
	      upper_bound([not(ID)|IDs]),   %% checkme
	      fail
	    ;
	      b_setval(problog_probability, NProb),
	      b_setval(problog_current_proof, [not(ID)|IDs])
	    )
	  ),
	  Steps is MaxSteps-1,
	  b_setval(problog_steps,Steps)
	).
%%%% Bernd, changes for negated ground facts


% if in monte carlo mode, check array to see if fact can be used
montecarlo_check(ID) :-
	(
	 problog_control(check,mc)
	->
	 (
          array_element(mc_sample,ID,V),
	  (
	   V == 1 -> true
	  ;
	   V == 2 -> fail
	  ;
	   new_sample(ID)
	  )
	 )
	;
	  true
	).

new_sample(ID) :-
	get_fact_probability(ID,Prob),
	random(R),
	R<Prob,
	!,
        update_array(mc_sample,ID,1).
new_sample(ID) :-
        update_array(mc_sample,ID,2),
        fail.

% if threshold reached, remember this by setting limit to on, then
% if up is on, store stopped derivation in second trie
% 
% List always length>=1 -> don't need []=true-case for tries
upper_bound(List) :-
	problog_control(on,limit),
	problog_control(check,up),
	reverse(List,R),
	(prune_check(R,2) -> true; insert_ptree(R,2)).

multiply_probabilities(CurrentLogP, LogProb, NLogProb) :-
	NLogProb is CurrentLogP+LogProb.	

% this is called by all inference methods before the actual ProbLog goal
% to set up environment for proving
init_problog(Threshold) :-
	reset_non_ground_facts,
	LT is log(Threshold),
	b_setval(problog_probability, 0.0),
	b_setval(problog_current_proof, []),
	nb_setval(problog_threshold, LT),
	problog_flag(maxsteps,MaxS),
	b_setval(problog_steps, MaxS),
	problog_control(off,limit).

% idea: proofs that are refinements of known proof can be pruned as they don't add probability mass
% note that current ptree implementation doesn't provide the check as there's no efficient method known so far...
prune_check(Proof,TreeID) :-
	problog_flag(prunecheck,on),
	prune_check_ptree(Proof,TreeID).

% to call a ProbLog goal, patch all subgoals with the user's module context
% (as logical part is there, but probabilistic part in problog)
problog_call(Goal) :-
	yap_flag(typein_module,Module),
%%% if user provides init_db, call this before proving goal
	(current_predicate(_,Module:init_db) -> call(Module:init_db); true),
	put_module(Goal,Module,ModGoal),
	call(ModGoal).

put_module((Mod:Goal,Rest),Module,(Mod:Goal,Transformed)) :-
	!,
	put_module(Rest,Module,Transformed).
put_module((Goal,Rest),Module,(Module:Goal,Transformed)) :-
	!,
	put_module(Rest,Module,Transformed).
put_module((Mod:Goal),_Module,(Mod:Goal)) :-
	!.
put_module(Goal,Module,Module:Goal).

% end of core

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% evaluating a DNF given as trie using BDD
% input: ID of trie to be used
% output: probability and status (to catch potential failures/timeouts from outside)
% 
% with internal BDD timeout (set using problog flag bdd_time)
%
% bdd_ptree/3 constructs files for ProblogBDD from the trie
%
% if calling ProblogBDD doesn't exit successfully, status will be timeout
%
% writes number of proofs in trie and BDD time to standard user output
%
% if remember is on, input files for ProblogBDD will be saved
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

eval_dnf(ID,Prob,Status) :- 
	((ID = 1, problog_flag(save_bdd,true)) -> problog_control(on,remember); problog_control(off,remember)),
	count_ptree(ID,NX),
	(
	    NX=1
	->
	    format(user,'1 proof~n',[]);
            format(user,'~w proofs~n',[NX])
	),
	problog_flag(dir,DirFlag),
	problog_flag(bdd_file,BDDFileFlag),
	atomic_concat([DirFlag,BDDFileFlag],BDDFile),
	problog_flag(bdd_par_file,BDDParFileFlag),
	atomic_concat([DirFlag,BDDParFileFlag],BDDParFile),
	(problog_control(check,remember) ->
	    bdd_ptree_map(ID,BDDFile,BDDParFile,Mapping),
	    atomic_concat([DirFlag,'save_map'],MapFile),
	    tell(MapFile),
	    format('mapping(~q).~n',[Mapping]),
	    flush_output,
	    told
	;
	bdd_ptree(ID,BDDFile,BDDParFile)
        ), 
	problog_flag(bdd_time,BDDTime),
	problog_flag(bdd_result,ResultFileFlag),
	atomic_concat([DirFlag,ResultFileFlag],ResultFile),
	problog_dir(PD),
	atomic_concat([PD,'/ProblogBDD -l ',BDDFile,' -i ',BDDParFile,' -m p -t ', BDDTime,' > ', ResultFile],Command),
	statistics(walltime,_),
	shell(Command,Return), 
	(
	    Return =\= 0
	->
	    Status = timeout
	;
	    (
		statistics(walltime,[_,E3]),
		format(user,'~w ms BDD processing~n',[E3]),
		see(ResultFile),
		read(probability(Prob)),
		seen,
		delete_file(ResultFile),
		Status = ok
	    )
	),
	(problog_control(check,remember) ->
	    atomic_concat([DirFlag,'save_script'],SaveBDDFile),
	    rename_file(BDDFile,SaveBDDFile),
	    atomic_concat([DirFlag,'save_params'],SaveBDDParFile),
	    rename_file(BDDParFile,SaveBDDParFile)
	;
	true
        ),
	problog_control(off,remember).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% different inference methods 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% approximate inference: bounds based on single probability threshold
% problog_threshold(+Goal,+Threshold,-LowerBound,-UpperBound,-Status)
% 
% use backtracking over problog_call to get all solutions
%
% trie 1 collects proofs, trie 2 collects stopped derivations, trie 3 is used to unit them for the upper bound
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

problog_threshold(Goal, Threshold, _, _, _) :-
	problog_control(on,up),
	init_problog_threshold(Threshold),
	problog_call(Goal),
	add_solution,
	fail.
problog_threshold(_, _, LP, UP, Status) :-
	compute_bounds(LP, UP, Status).

init_problog_threshold(Threshold) :-
	init_ptree(1),
	init_ptree(2),
	init_problog(Threshold).

add_solution :-
	b_getval(problog_current_proof, IDs),
	(IDs == [] -> R = true ; reverse(IDs,R)),
	insert_ptree(R,1).

compute_bounds(LP, UP, Status) :-
	eval_dnf(1,LP,StatusLow),
	(StatusLow \== ok ->
	    Status = StatusLow
	;
	merge_ptree(1,2,3),
	eval_dnf(3,UP,Status)),
	delete_ptree(1),
	delete_ptree(2),
	delete_ptree(3).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% approximate inference: lower bound based on all proofs above probability threshold
% problog_low(+Goal,+Threshold,-LowerBound,-Status)
% 
% same as problog_threshold/5, but lower bound only (no stopped derivations stored) 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

problog_low(Goal, Threshold, _, _) :-
	problog_control(off,up),
	init_problog_low(Threshold),
	problog_call(Goal),
	add_solution,
	fail.
problog_low(_, _, LP, Status) :-
	eval_dnf(1,LP,Status),
	delete_ptree(1).

init_problog_low(Threshold) :-
	init_ptree(1),
	init_problog(Threshold).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% approximate inference: bounds by iterative deepening up to interval width Delta
% problog_delta(+Goal,+Delta,-LowerBound,-UpperBound,-Status)
%
% wraps iterative deepening around problog_threshold, i.e.
% - starts with threshold given by first_threshold flag
% - if Up-Low >= Delta, multiply threshold by factor given in id_stepsize flag and iterate
% (does not use problog_threshold as trie 1 is kept over entire search)
%
% local dynamic predicates low/2, up/2, stopDiff/1
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

problog_delta(Goal, Delta, Low, Up, Status) :-
	problog_control(on,up),
	problog_flag(first_threshold,InitT),
	init_problog_delta(InitT,Delta),
	problog_delta_id(Goal,Status),
	delete_ptree(1),
	delete_ptree(2),
	(retract(low(_,Low)) -> true; true),
	(retract(up(_,Up)) -> true; true).


init_problog_delta(Threshold,Delta) :-
	retractall(low(_,_)),
	retractall(up(_,_)),
	retractall(stopDiff(_)),
	init_ptree(1),
	init_ptree(2),
	assert(low(0,0.0)),
	assert(up(0,1.0)),
	assert(stopDiff(Delta)),
	init_problog(Threshold).

problog_delta_id(Goal, _) :-
	problog_call(Goal),
	add_solution,     % reused from problog_threshold
	fail.
problog_delta_id(Goal, Status) :-
	evaluateStep(Ans,StatusE),
	problog_flag(last_threshold_log,Stop),
	nb_getval(problog_threshold,Min),
	(StatusE \== ok ->
	    Status = StatusE
	;
	(
	    Ans = 1 ->
	    Status = ok
	;
	    Min =<  Stop ->
	    Status = stopreached
	;
	    problog_control(check,limit) ->
	    problog_control(off,limit),
	    problog_flag(id_stepsize_log,Step),
	    New is Min+Step,
	    nb_setval(problog_threshold,New),
	    problog_delta_id(Goal, Status)
	;
	true
	)).
	
% call the dnf evaluation where needed
evaluateStep(Ans,Status)  :- once(evalStep(Ans,Status)).

evalStep(Ans,Status) :-
	stopDiff(Delta),
	count_ptree(1,NProofs),
	count_ptree(2,NCands),
	format(user,'~w proofs, ~w stopped derivations~n',[NProofs,NCands]),
	flush_output(user),
	eval_lower(NProofs,Low,StatusLow),
	(StatusLow \== ok ->
	    Status = StatusLow
	;
	    up(_,OUP),
	    IntDiff is OUP-Low,
	    ((IntDiff < Delta; IntDiff =:= 0) -> 
		Up=OUP, StatusUp = ok
	    ;
	        eval_upper(NCands,Up,StatusUp),
		delete_ptree(2),
		init_ptree(2),
		delete_ptree(3)
	    ),
	    (StatusUp \== ok ->
		Status = StatusUp
	    ;
	        Diff is Up-Low,
		format(user,'difference:  ~6f~n',[Diff]),
		flush_output(user),
		((Diff < Delta; Diff =:= 0) -> Ans = 1; Ans = 0),
		Status = ok)).

% no need to re-evaluate if no new proofs found on this level
eval_lower(N,P,ok) :-
	low(N,P).
% evaluate if there are proofs
eval_lower(N,P,Status) :-
	N > 0,
	low(OldN,_),
	N \= OldN,
	eval_dnf(1,P,Status),
	(Status = ok -> 
	    retract(low(_,_)),
	    assert(low(N,P)),
	    format(user,'lower bound: ~6f~n',[P]),
	    flush_output(user)
	;
	true).

% if no stopped derivations, up=low
eval_upper(0,P,ok) :-
	retractall(up(_,_)),
	low(N,P),
	assert(up(N,P)).
% else merge proofs and stopped derivations to get upper bound
% in case of timeout or other problems, skip and use bound from last level
eval_upper(N,UpP,ok) :-
	N > 0,
	merge_ptree(1,2,3),
	eval_dnf(3,UpP,StatusUp),
	(StatusUp = ok ->
	    retract(up(_,_)),
	    assert(up(N,UpP))
	;
	format(user,'~w - continue using old up~n',[StatusUp]),
	flush_output(user),
	up(_,UpP)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% explanation probability - returns list of facts used or constant 'unprovable' as third argument
% problog_max(+Goal,-Prob,-Facts)
%
% uses iterative deepening with samw parameters as bounding algorithm
% threshold gets adapted whenever better proof is found
%
% uses local dynamic predicates max_probability/1 and max_proof/1
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

problog_max(Goal, Prob, Facts) :-
	problog_control(off,up),
	problog_flag(first_threshold,InitT),
	init_problog_max(InitT),
	problog_max_id(Goal, Prob, FactIDs),
	( FactIDs = [_|_] -> get_fact_list(FactIDs,Facts);
	    Facts = FactIDs).

init_problog_max(Threshold) :-
	retractall(max_probability(_)),
	retractall(max_proof(_)),
	assert(max_probability(-999999)),
	assert(max_proof(unprovable)),
	init_problog(Threshold).

update_max :-
	b_getval(problog_probability,CurrP),
	max_probability(MaxP),
	(CurrP =< MaxP ->
	    fail
	;
	b_getval(problog_current_proof, IDs),
	reverse(IDs,R),
	retractall(max_proof(_)),
	assert(max_proof(R)),
	nb_setval(problog_threshold, CurrP),
	retractall(max_probability(_)),
	assert(max_probability(CurrP))).

problog_max_id(Goal, _Prob, _Clauses) :-
	problog_call(Goal),
	update_max,
	fail.
problog_max_id(Goal, Prob, Clauses) :-
	max_probability(MaxP),
	nb_getval(problog_threshold, LT),
	problog_flag(last_threshold_log,ToSmall),
	((MaxP >= LT ; \+ problog_control(check,limit); LT < ToSmall) ->
	    ((max_proof(unprovable), problog_control(check,limit), LT < ToSmall) ->
		problog_flag(last_threshold,Stopping),
		Clauses = unprovable(Stopping) 
	    ; max_proof(Clauses)),
	   Prob is exp(MaxP)
       ;
	problog_flag(id_stepsize_log,Step),
	NewLT is LT+Step,
	nb_setval(problog_threshold, NewLT),
	problog_control(off,limit),
	problog_max_id(Goal, Prob, Clauses)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% lower bound using k best proofs
% problog_kbest(+Goal,+K,-Prob,-Status)
%
% does iterative deepening search similar to problog_max, but for k(>=1) most likely proofs
% afterwards uses BDD evaluation to calculate probability (also for k=1 -> uniform treatment in learning)
%
% uses dynamic local predicate current_kbest/3 to collect proofs,
% only builds trie at the end (as probabilities of single proofs are important here)
%
% note: >k proofs will be used if the one at position k shares its probability with others, 
% as all proofs with that probability will be included 
%
% version with _save at the end  renames files for ProblogBDD to keep them
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
problog_kbest_save(Goal, K, Prob, Status, BDDFile, ParamFile) :-
	problog_kbest(Goal, K, Prob, Status),
	( Status=ok ->
	    problog_flag(bdd_file,InternBDDFlag),
	    problog_flag(bdd_par_file,InternParFlag),
	    problog_flag(dir,DirFlag),
	    atomic_concat([DirFlag,InternBDDFlag],InternBDD),
	    atomic_concat([DirFlag,InternParFlag],InternPar),
	    rename_file(InternBDD,BDDFile),
	    rename_file(InternPar,ParamFile)
	;
	true).

problog_kbest(Goal, K, Prob, Status) :-
	problog_control(off,up),
	problog_flag(first_threshold,InitT),
	init_problog_kbest(InitT),
	problog_kbest_id(Goal, K),
	retract(current_kbest(_,ListFound,_NumFound)),
	build_prefixtree(ListFound),
	eval_dnf(1,Prob,Status),
	delete_ptree(1).

init_problog_kbest(Threshold) :-
	retractall(current_kbest(_,_,_)),
	assert(current_kbest(-999999,[],0)),  %(log-threshold,proofs,num_proofs)  
	init_ptree(1),
	init_problog(Threshold).

problog_kbest_id(Goal, K) :-
	problog_call(Goal),
	update_kbest(K),
	fail.
problog_kbest_id(Goal, K) :-
	current_kbest(CurrentBorder,_,Found),
	nb_getval(problog_threshold, Min),
	problog_flag(last_threshold_log,ToSmall),
	((Found>=K ; \+ problog_control(check,limit) ; Min < CurrentBorder ; Min < ToSmall) ->
	   true
       ;
	problog_flag(id_stepsize_log,Step),
	NewLT is Min+Step,
	nb_setval(problog_threshold, NewLT),
	problog_control(off,limit),
	problog_kbest_id(Goal, K)).

update_kbest(K) :-
	b_getval(problog_probability,NewLogProb),
	current_kbest(LogThreshold,_,_),
	(NewLogProb>=LogThreshold -> 
	    b_getval(problog_current_proof,RevProof),
	    reverse(RevProof,Proof),
	    update_current_kbest(K,NewLogProb,Proof)
	;
	    fail).

update_current_kbest(_,NewLogProb,Cl) :-
	current_kbest(_,List,_),
	memberchk(NewLogProb-Cl,List),
	!.
update_current_kbest(K,NewLogProb,Cl) :-
	retract(current_kbest(OldThres,List,Length)),
	sorted_insert(NewLogProb-Cl,List,NewList),
	NewLength is Length+1,
	(NewLength < K ->
	    assert(current_kbest(OldThres,NewList,NewLength))
	;
	(NewLength>K -> 
	    First is NewLength-K+1,
	    cutoff(NewList,NewLength,First,FinalList,FinalLength)
	   ; FinalList=NewList, FinalLength=NewLength),
	FinalList=[NewThres-_|_],
	nb_setval(problog_threshold,NewThres),
	assert(current_kbest(NewThres,FinalList,FinalLength))).

sorted_insert(A,[],[A]).
sorted_insert(A-LA,[B1-LB1|B], [A-LA,B1-LB1|B] ) :-
	A =< B1.
sorted_insert(A-LA,[B1-LB1|B], [B1-LB1|C] ) :-
	A > B1,
	sorted_insert(A-LA,B,C).

% keeps all entries with lowest probability, even if implying a total of more than k
cutoff(List,Len,1,List,Len) :- !.
cutoff([P-L|List],Length,First,[P-L|List],Length) :-
	nth(First,[P-L|List],PF-_),
	PF=:=P,
	!.
cutoff([_|List],Length,First,NewList,NewLength) :-
	NextFirst is First-1,
	NextLength is Length-1,
	cutoff(List,NextLength,NextFirst,NewList,NewLength).

build_prefixtree([]).
build_prefixtree([_-[]|_List]) :-
	!,
	insert_ptree(true,1).
build_prefixtree([_-L|List]) :-
	insert_ptree(L,1),
	build_prefixtree(List).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% exact probability 
% problog_exact(+Goal,-Prob,-Status)
%
% using all proofs = using all proofs with probability > 0
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

problog_exact(Goal,Prob,Status) :-
	problog_low(Goal,0,Prob,Status).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% probability by sampling: 
% running another N samples until 95percentCI-width<Delta 
% lazy sampling using three-valued array indexed by internal fact IDs
%
% still collects actual proofs found in samples in ptree, though this is no longer used  
%   by method itself, only to write number to log-file
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

problog_montecarlo(_,_,_) :-
	non_ground_fact(_),
	!,
	format(user_error,'Current database contains non-ground facts.',[]),
	format(user_error,'Monte Carlo inference is not possible in this case. Try k-best instead.',[]),
	fail.
	

problog_montecarlo(Goal,Delta,Prob) :-
        nb_getval(probclause_counter,ID), !,
        C is ID+1,
        static_array(mc_sample,C,char),
  	problog_control(off,up),
	problog_flag(mc_batchsize,N),
	problog_flag(mc_logfile,File1),
	problog_flag(dir,Dir),
	atomic_concat([Dir,File1],File),
	montecarlo(Goal,Delta,N,File),
	retract(mc_prob(Prob)).

montecarlo(Goal,Delta,K,File) :-
        reset_static_array(mc_sample),
	problog_control(on,mc),
	open(File,write,Log),
	format(Log,'# goal: ~q~n#delta: ~w~n',[Goal,Delta]),
	format(Log,'# num_programs  prob   low   high  diff  time   cache_size  num_pos~2n',[]),
	close(Log),
	statistics(walltime,[T1,_]),
	init_ptree(1),
	format('search for ~q~n',[Goal]),
	montecarlo(Goal,Delta,K,0,File,0,T1),
	problog_control(off,mc),
	delete_ptree(1).

% calculate values after K samples
montecarlo(Goal,Delta,K,SamplesSoFar,File,PositiveSoFar,InitialTime) :-
	SamplesNew is SamplesSoFar+1,
	SamplesNew mod K =:= 0,
	!,
	copy_term(Goal,GoalC),
	(mc_prove(GoalC) -> Next is PositiveSoFar+1; Next=PositiveSoFar),
	Prob is Next/SamplesNew, 
	Epsilon is 2*sqrt(Prob*(1-Prob)/SamplesNew), 
	Low is Prob-Epsilon,
	High is Prob+Epsilon,
	Diff is 2*Epsilon,
	statistics(walltime,[T2,_]),
	Time is (T2-InitialTime)/1000,
	count_ptree(1,CacheSize),
	format('~n~w samples~nestimated probability ~w~n95 percent confidence interval [~w,~w]~n',[SamplesNew,Prob,Low,High]),
	open(File,append,Log),
	format(Log,'~w  ~8f  ~8f  ~8f  ~8f  ~3f  ~w  ~w~n',[SamplesNew,Prob,Low,High,Diff,Time,CacheSize,Next]),
	close(Log),
	((Diff<Delta; Diff =:= 0) -> 	format('Runtime ~w sec~2n',[Time]),assert(mc_prob(Prob))
		    ;	
	                montecarlo(Goal,Delta,K,SamplesNew,File,Next,InitialTime)).

% continue until next K samples done
montecarlo(Goal,Delta,K,SamplesSoFar,File,PositiveSoFar,InitialTime) :-
	SamplesNew is SamplesSoFar+1,
	copy_term(Goal,GoalC),
	(mc_prove(GoalC) -> Next is PositiveSoFar+1; Next=PositiveSoFar),
	montecarlo(Goal,Delta,K,SamplesNew,File,Next,InitialTime).

mc_prove(A) :- !,
	(get_some_proof(A) ->
	 clean_sample
	; 
	 clean_sample,fail
	).	

clean_sample :-
        reset_static_array(mc_sample),
	fail.
clean_sample.

% find new proof
get_some_proof(Goal) :-
	init_problog(0),
	problog_call(Goal),
	b_getval(problog_current_proof,Used),
	(Used == [] -> Proof=true; reverse(Used,Proof)),
	insert_ptree(Proof,1).

