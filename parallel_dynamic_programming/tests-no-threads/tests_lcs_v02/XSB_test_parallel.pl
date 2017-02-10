:-import conget/2, conset/2 from machine.
:-import between/3 from basics.
set_value(X,Y) :- conset(X,Y).
get_value(X,Y) :- conget(X,Y).

%:-['../lcs_v01/data_size_3000_max_random_1500.pl'].
%:-['../lcs_v01/data_size_2000_all_different.pl'].

%:-['../lcs_v01/data_size_1000_max_random_1000.pl'].

:-['matrix_lcs_smith.pl'].

:- set_value(qsize, 1).

compute(_, W, WLast) :- 
    W > WLast, !.

compute(ILast, WFirst, WLast) :- 
    loop_i(1, ILast, WFirst, WLast).

worker(IL, WL, WO) :-  
    get_value(qsize, W),
    (W =< WL,
      W1 is W + WO,
      set_value(qsize, W1),
      W2 is W1 - 1,
      compute(IL, W, W2),
      worker(IL, WL, WO); 
     fail).

run_query(WorkerOffset, T):- 
    data_call(IL, WL),
    statistics(walltime,[InitTime,_]),
    (worker(IL, WL, WorkerOffset) ; true),
    statistics(walltime,[EndTime,_]),
    Time is EndTime - InitTime,
    writeln('Threads-Time:'-T-Time).
 %   lcs(IL, WL, V),
 %   writeln('Result'-V).

%:- run_query(5, 1).
%:- halt.

% /home/miguelareias/conferences/iclp2013/XSB/bin/xsb-mt --shared_predicates -e "['test_parallel_XSB.pl']."                
