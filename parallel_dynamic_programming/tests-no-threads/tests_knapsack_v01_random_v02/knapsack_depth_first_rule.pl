
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
time_query(Index, Value):-
    knapsack(Index, Value, _),
    fail.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

knapsack(0, I, K, V, WOffset) :-
  I > 0,
  I1 is I - WOffset,
  ensure_non_negative_I1(I1, K, V). % ensure non negative call of I1 -> due to dim mode
%  knapsack(I1, K, V).

knapsack(1, I, K, V, WOffset) :- 
  I > 0,
  item(I, F, Bi),
  K1 is K - F,
  K1 >= 0,
  I1 is I - WOffset,
  ensure_non_negative_I1(I1, K1, V1),  % ensure non negative call of I1 -> due to dim mode
%  knapsack(I1, K1, V1),
  V is V1 + Bi.

%:- table knapsack(index, index, max).
:- table knapsack(dim(1650), dim(3500), max(integer)).

ensure_non_negative_I1(I1, K, 0) :-
    I1 < 0, !.

ensure_non_negative_I1(I1, K, V) :-
    knapsack(I1, K, V).

knapsack(_, W, 0):- 
  W >= 0.

knapsack(A1, A2, A3):- 
    random_hos(2, 160, WOffset),
    N is WOffset mod 2,
    call_knapsack(N, A1, A2, A3, WOffset).

call_knapsack(0, A1, A2, A3, WOffset):- 
    ((knapsack(0, A1, A2, A3, WOffset) ;
      knapsack(0, A1, A2, A3, 1))
    ; 
     knapsack(1, A1, A2, A3, 1)).

call_knapsack(1, A1, A2, A3, WOffset):- 
    ((knapsack(1, A1, A2, A3, WOffset) ; 
      knapsack(1, A1, A2, A3, 1))
    ;
     knapsack(0, A1, A2, A3, 1)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


