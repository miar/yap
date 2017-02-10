%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                         LCS                              %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% lcs (i, j, s[i,j]).
%:- table lcs/3.
:- table lcs(dim(3500), dim(3500), max(integer)).

lcs(0, _, 0).
lcs(_, 0, 0).

lcs(I, J, V):- 
    I > 0, J > 0,  
    I1 is I - 1,
    J1 is J - 1,
    item1(I, V1), % V1 = ai
    item2(J, V2), % V2 = bj
    ( (V1 == V2,
       lcs(I1, J1, V3),
       V is V3 + 1) ;
      (V1 =\= V2,
       lcs(I1, J, V3),
       lcs(I, J1, V4),
       max(V3, V4, V)
      )).

max(V1, V2, V1):- 
    V1 > V2, !.
max(_, V2, V2).

loop_w(I, WL, WL) :- !,               
    lcs(I, WL, _).

loop_w(I, W, WL) :- 
    lcs(I, W, _),
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
