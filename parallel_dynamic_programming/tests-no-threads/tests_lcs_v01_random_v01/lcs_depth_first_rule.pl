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
    lcs(Index1, Index2, _),
    fail.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

lcs(0, Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 == F2,
   Aux11 is Aux1 - 1,
   Aux21 is Aux2 - 1,
   ensure_non_negative_Aux(Aux11, Aux21, N1),  % ensure non negative Aux -> due to dim mode
%   lcs(Aux11, Aux21, N1),
   N is N1 + 1.

lcs(1, Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 =\= F2,
   Aux11 is Aux1 - 1,
   ensure_non_negative_Aux(Aux11, Aux2, N).
%   lcs(Aux11, Aux2, N).

lcs(2, Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 =\= F2,
   Aux21 is Aux2 - 1,
   ensure_non_negative_Aux(Aux1, Aux21, N).
%   lcs(Aux1, Aux21, N).

ensure_non_negative_Aux(Aux11, Aux21, 0) :-
    Aux11 < 0, !.
ensure_non_negative_Aux(Aux11, Aux21, 0) :-
    Aux21 < 0, !.
ensure_non_negative_Aux(Aux11, Aux21, V) :-
    lcs(Aux11, Aux21, V).

%:- table lcs(index, index, max).
:- table lcs(dim(3500), dim(3500), max(integer)).

lcs(_, 0, 0).
lcs(0, _, 0).

lcs(A1, A2, A3):-
    random_hos(0,5, N),
    call_lcs(N, A1, A2, A3).

call_lcs(0, A1, A2, A3):-
    (lcs(0, A1, A2, A3) ;
    (lcs(1, A1, A2, A3) ; 
     lcs(2, A1, A2, A3))).

call_lcs(1, A1, A2, A3):-
    (lcs(0, A1, A2, A3) ;
    (lcs(2, A1, A2, A3) ; 
     lcs(1, A1, A2, A3))).

call_lcs(2, A1, A2, A3):-
    (lcs(1, A1, A2, A3) ;
    (lcs(0, A1, A2, A3) ; 
     lcs(2, A1, A2, A3))).

call_lcs(3, A1, A2, A3):-
    (lcs(1, A1, A2, A3) ;
    (lcs(2, A1, A2, A3) ; 
     lcs(0, A1, A2, A3))).

call_lcs(4, A1, A2, A3):-
    (lcs(2, A1, A2, A3) ;
    (lcs(0, A1, A2, A3) ; 
     lcs(1, A1, A2, A3))).

call_lcs(5, A1, A2, A3):-
    (lcs(2, A1, A2, A3) ;
    (lcs(1, A1, A2, A3) ; 
     lcs(0, A1, A2, A3))).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

