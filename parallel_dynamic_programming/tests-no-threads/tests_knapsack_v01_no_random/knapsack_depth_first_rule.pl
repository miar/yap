
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
time_query(Index, Value):-
    knapsack(Index, Value, _),
    fail.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%:- table knapsack(index, index, max).
:- table knapsack(dim(1650), dim(3500), max(integer)).
knapsack(_, W, 0):- W >= 0.

knapsack(I, K, V) :-
    I > 0,
    I1 is I - 1,
    knapsack(I1, K, V).

knapsack(I, K, V) :- 
    I > 0,
    item(I, F, Bi),
    K1 is K - F,
    K1 >= 0,
    I1 is I - 1,
    knapsack(I1, K1, V1),
    V is V1 + Bi.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


