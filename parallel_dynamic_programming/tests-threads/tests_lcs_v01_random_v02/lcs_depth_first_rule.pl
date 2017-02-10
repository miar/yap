%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

time_query(Index1, Index2):-
    lcs(Index1, Index2, _),
    fail.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
lcs(0, Aux1, Aux2, N, WOffset) :-
    Aux11 is Aux1 - WOffset,
    ensure_non_negative_Aux(Aux11, Aux2, N).  % ensure non negative Aux -> due to dim mode
%    lcs(Aux11, Aux2, N).

lcs(1, Aux1, Aux2, N, WOffset) :-
    Aux21 is Aux2 - WOffset,
    ensure_non_negative_Aux(Aux1, Aux21, N).  % ensure non negative Aux -> due to dim mode
%    lcs(Aux1, Aux21, N).

%:- table lcs(index, index, max).
:- table lcs(dim(3500), dim(3500), max(integer)).

lcs(_, 0, 0).
lcs(0, _, 0).

lcs(Aux1, Aux2, N) :-
   Aux1 >= 0,
   Aux2 >= 0,
   item1(Aux1, F1),
   item2(Aux2, F2),
   F1 == F2,
   Aux11 is Aux1 - 1,
   Aux21 is Aux2 - 1,
   ensure_non_negative_Aux(Aux11, Aux21, N1),  % ensure non negative Aux -> due to dim mode
%   lcs(Aux11, Aux21, N1),
   N is N1 + 1.

ensure_non_negative_Aux(Aux11, Aux21, 0) :-
    Aux11 < 0, !.

ensure_non_negative_Aux(Aux11, Aux21, 0) :-
    Aux21 < 0, !.

ensure_non_negative_Aux(Aux11, Aux21, V) :-
    lcs(Aux11, Aux21, V).

lcs(A1, A2, A3):-
   A1 >= 0,
   A2 >= 0,
   item1(A1,F1),
   item2(A2,F2),
   F1 =\= F2,
   random_hos(2,320, WOffset),
   N is WOffset mod 2,
   call_lcs(N, A1, A2, A3, WOffset).

call_lcs(0, A1, A2, A3, WOffset):- 
    ((lcs(0, A1, A2, A3, WOffset) ;
      lcs(0, A1, A2, A3, 1))
    ; 
     lcs(1, A1, A2, A3, 1)).

call_lcs(1, A1, A2, A3, WOffset):- 
    ((lcs(1, A1, A2, A3, WOffset) ; 
      lcs(1, A1, A2, A3, 1))
    ;
     lcs(0, A1, A2, A3, 1)).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

