%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

time_query(Index1, Index2):-
    lcs(Index1, Index2, _),
    fail.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%:- table lcs(index, index, max).
:- table lcs(dim(3500), dim(3500), max(integer)).

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
   ensure_non_negative_Aux(Aux11, Aux21, N1),
   %lcs(Aux11, Aux21, N1),
   N is N1 + 1.

lcs(Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1,F1),
   item2(Aux2,F2),
   F1 =\= F2,
   Aux11 is Aux1 - 1,
   ensure_non_negative_Aux(Aux11, Aux2, N).
%   lcs(Aux11, Aux2, N).

lcs(Aux1, Aux2, N) :-
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


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

