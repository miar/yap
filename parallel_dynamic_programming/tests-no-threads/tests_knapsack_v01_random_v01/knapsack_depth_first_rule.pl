
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
time_query(Index, Value):-
    knapsack(Index, Value, _),
    fail.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

knapsack(0, I, K, V) :-
  I > 0,
  I1 is I - 1,
  knapsack(I1, K, V).

knapsack(1, I, K, V) :- 
  I > 0,
  item(I, F, Bi),
  K1 is K - F,
  K1 >= 0,
  I1 is I - 1,
  knapsack(I1, K1, V1),
  V is V1 + Bi.

%:- table knapsack(index, index, max).
:- table knapsack(dim(1650), dim(3500), max(integer)).
knapsack(_, W, 0):- 
  W >= 0.

knapsack(A1, A2, A3):- 
    random_hos(0, 1, N),
    call_knapsack(N, A1, A2, A3).

call_knapsack(0, A1, A2, A3):- 
    (knapsack(0, A1, A2, A3) ; 
     knapsack(1, A1, A2, A3)).

call_knapsack(1, A1, A2, A3):- 
    (knapsack(1, A1, A2, A3) ; 
     knapsack(0, A1, A2, A3)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


