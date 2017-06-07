:- yap_flag(tabling_mode,[local,load_answers,local_trie]).

%:- ['data_size_1600_max_random_160_k_3200.pl'].
%:-['data_size_2000_k_1000_inc.pl'].

:-['matrix_knapsack.pl'].

:- set_value(qsize, 1).

compute(_, W, WLast) :- 
    W > WLast, !.

compute(ILast, WFirst, WLast) :- 
%    writeln(ILast-WFirst-WLast),
    loop_i(1, ILast, WFirst, WLast).

worker(IL, WL, WO) :-  
    get_value(qsize, W),
    (W =< WL -> 
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
%    knap(IL, WL, V),
%    writeln('Result'-V).

%:- run_query(5,32).

%:- data_call(IL, WL),loop_i(1,IL,1,WL),knap(IL,WL,V), writeln(result-V).
%:- halt.




