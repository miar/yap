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
    ;
     time_query(I1, I2)
    ).

time_query_mt(1, I1, I2) :- !,
    time_query(I1, I2).


time_query_mt(2, I1, I2) :- !,
    thread_create(time_query_rule1(1, I1, I2), Tid1),
    (time_query_rule2(1, I1, I2) ; true),
    (time_query_rule3(1, I1, I2) ; true),
    thread_join(Tid1,_),
    fail.


time_query_mt(3, I1, I2) :- !,
    thread_create(time_query_rule1(1, I1, I2), Tid1),
    thread_create(time_query_rule2(1, I1, I2), Tid2),
    (time_query_rule3(1, I1, I2) ; true),
    thread_join(Tid1,_),
    thread_join(Tid2,_),
    fail.

%% op 1 (open 1st rule)
/*
time_query_mt(NT, I1, I2) :-
    NT1 is NT - 2,
    thread_create(time_query_rule1(NT1, I1, I2), Tid1),
    thread_create(time_query_rule2(1, I1, I2), Tid2),
    (time_query_rule3(1, I1, I2) ; true),
    thread_join(Tid1,_),
    thread_join(Tid2,_),
    fail.
*/


%% op 2 (open 2rd rule)


time_query_mt(NT, I1, I2) :-
    NT1 is NT - 2,
    thread_create(time_query_rule1(1, I1, I2), Tid1),
    thread_create(time_query_rule2(NT1, I1, I2), Tid2),
    (time_query_rule3(1, I1, I2) ; true),
    thread_join(Tid1,_),
    thread_join(Tid2,_),
    fail.


%% op 3 (open 3nd rule)

/*
time_query_mt(NT, I1, I2) :-
    NT1 is NT - 2,
    thread_create(time_query_rule1(1, I1, I2), Tid1),
    thread_create(time_query_rule2(1, I1, I2), Tid2),
    (time_query_rule3(NT1, I1, I2) ; true),
    thread_join(Tid1,_),
    thread_join(Tid2,_),
    fail.
*/



%% op 4 (open by three level- ideal 9 / 27 / 81 / ...) 

/*
time_query_mt(NT, I1, I2) :-
%    writeln(ola3),
    NT1 is NT // 3,
    thread_create(time_query_rule1(NT1, I1, I2), Tid1),
    NT2 is NT - NT1,
    NT3 is NT2 // 2,
    thread_create(time_query_rule2(NT3, I1, I2), Tid2),
    NT4 is NT2 - NT3,
    (time_query_rule3(NT4, I1, I2) ; true),
    thread_join(Tid1,_),
    thread_join(Tid2,_),
    fail.
*/



%% end all ops

time_query_rule1(NT, I1, I2) :-
    I1 >= 0,
    I2 >= 0,
    item1(I1, F1),
    item2(I2, F2),
    F1 == F2,
    I11 is I1 - 200,
    I21 is I2 - 200,
    time_query_mt(NT, I11, I21).

time_query_rule2(NT, I1, I2) :-
   I1 >= 0,
   I2 >= 0,
   item1(I1, F1),
   item2(I2, F2),
   F1 =\= F2,
   I11 is I1 - 200,
   time_query_mt(NT, I11, I2).

time_query_rule3(NT, I1, I2) :-
   I1 >= 0,
   I2 >= 0,
   item1(I1, F1),
   item2(I2, F2),
   F1 =\= F2,
   I21 is I2 - 200,
   time_query_mt(NT, I1, I21).

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
%   writeln(r1),
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
 %  writeln(r2),
   Aux11 is Aux1 - 1,
   lcs(Aux11, Aux2, N).

lcs(Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 =\= F2,
%   writeln(r3),
   Aux21 is Aux2 - 1,
   lcs(Aux1, Aux21, N).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/*
time_query_mt(0, Index1, Index2):- !,
time_query_mt(NT, Index1, Index2):- 
    Index11 is Index1 - 1,
    Index21 is Index2 - 1,
    item1(Index11, Value1),
    item2(Index21, Value2),
    Value1 =\= Value1, !
    thread_create(time_query(Index11, Index2),Tid),    
    thread_create(time_query(Index1, Index21),Tid).

   Aux11 is Aux1 - 1,
   lcs(Aux11, Aux2, N).


   Aux21 is Aux2 - 1,
   lcs(Aux1, Aux21, N).
*/
