%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% File:     test_lcs_2000.pl
%% Added by: João Santos and Ricardo Rocha
%% Program:  longest common subsequence. Taken from "Simplifying 
%%           Dynamic Programming via Mode-directed Tabling" Software 
%%           Practice and Experience, 38(1): 75-94, 2008, 
%%           by Hai-Feng Guo, Gopal Gupta
%%          
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

time_query(Index1, Index2):-
    writeln('call = '-Index1-Index2),
    lcs(Index1, Index2, R),
    fail.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

top_query(NT) :-
    data_call(I1, I2),
    (
     time_query_mt(NT, I1, I2)
    ; writeln(top_query_call),
     time_query(I1, I2)
    ).

time_query_mt(1, I1, I2) :- !,
    time_query(I1, I2).


time_query_mt(NT, I1, I2) :-
    I1 >= 0,
    I2 >= 0,
    item1(I1, F1),
    item2(I2, F2),
    F1 == F2, !,
    I11 is I1 - 1,
    I21 is I2 - 1,
    time_query_mt(NT, I11, I21).


time_query_mt(NT, I1, I2) :-
    I1 >= 0,
    I2 >= 0,
    item1(I1, F1),
    item2(I2, F2),
    NT1 is NT // 2,
    I11 is I1 - NT * 100,
    I21 is I2 - NT * 200,
    thread_create(time_query_mt(NT1, I1, I2), Tid1),
    thread_create(time_query_mt(NT1, I1, I21), Tid2),
    thread_join(Tid1,_),
    thread_join(Tid2,_),
    fail.









%% end all ops

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

:- table lcs(index,index,max).

lcs(_, 0, 0).
lcs(0, _, 0).
lcs(Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 == F2,
   Aux11 is Aux1 - 1,
   Aux21 is Aux2 - 1,
   lcs(Aux11, Aux21, N1),
   N is N1 + 1.

lcs(Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 =\= F2,
   Aux11 is Aux1 - 1,
   lcs(Aux11, Aux2, N).

lcs(Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 =\= F2,
   Aux21 is Aux2 - 1,
   lcs(Aux1, Aux21, N).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
