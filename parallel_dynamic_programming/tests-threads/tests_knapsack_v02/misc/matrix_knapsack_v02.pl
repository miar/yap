:- yap_flag(tabling_mode,[local,load_answers,local_trie]).
:-['data_items'].

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                         KNAPSACK                         %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% knap (i, j, v[i,j]).
%:- table knap(index, index, max).
:- table knap/3.

knap(0,_,0).
knap(_, 0, 0).

knap(I, W, V):- 
    I > 0, W > 0,  
    item(I, Wi, Bi),
    Wi > W,
    I1 is I - 1,	     
    knap(I1, W, V).

knap(I, W, V) :- 
    I > 0, W > 0, 
    item(I, Wi, Bi),                
    Wi =< W,
    I1 is I - 1,
    W1 is W - Wi,
    knap(I1, W1, V1),
    knap(I1,  W, V2),
    knap1(Bi, V1, V2, V).

knap1(Bi, V1, V2, V):- 
    V3 is V1 + Bi,
    max(V3, V2, V).

max(V1, V2, V1):- 
    V1 > V2, !.
max(V1, V2, V2).

loop_w(I, WL, WL) :- !,               
    knap(I, WL, _).

loop_w(I, W, WL) :- 
    knap(I, W, _),
    W1 is W + 1,
    loop_w(I, W1, WL).

loop_i(IL, IL, W, WL) :- !, 
    loop_w(IL, W, WL).

loop_i(I, IL, W, WL) :- 
    loop_w(I, W, WL),
    I1 is I + 1,
    loop_i(I1, IL, W, WL).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                 GO STUFF                                 %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%% compute(ILast, WFirst, WLast):- loop_i(1, ILast, WFirst, WLast). 

%%%go_no_threads :- data_call(I,W), loop_i(1, I, W). 
%% :- go_no_threads. 
%% :- show_all_tables, halt.
