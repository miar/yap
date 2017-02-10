
:- yap_flag(tabling_mode,[local,load_answers,local_trie]).

:-['data_items'].

%  knap (i, j, v[i,j]).
%:- table knap(index, index, max).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                         KNAPSACK                         %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%








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

loop_w(I, W, W) :- !,               
    knap(I, W, _).

loop_w(I, J, W) :- 
    knap(I, J, _),
    J1 is J + 1,
    loop_w(I, J1, W).

loop_i(NI, NI, W) :- !, 
    loop_w(NI, 0, W).

loop_i(I, NI, W) :- 
    loop_w(I, 0, W),
    I1 is I + 1,
    loop_i(I1 , NI, W).

go_no_threads :- data_call(I,W), loop_i(1, I, W). 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                 MULTI-THREADING                          %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%





:- go. 
:- show_all_tables, halt.
