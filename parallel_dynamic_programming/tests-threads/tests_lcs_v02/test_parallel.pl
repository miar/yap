:- yap_flag(tabling_mode,[local,load_answers,local_trie]).

%:-['../lcs_v01/data_size_3000_all_different.pl'].

%:-['../lcs_v01/data_size_1000_max_random_1000.pl'].

%:-['../lcs_v01/data_size_3000_max_random_1500.pl'].

%:-['data_size_3200_max_random_960.pl'].


:-['matrix_lcs_smith.pl'].

:- mutex_create(qlock).
:- set_value(qsize, 1).

compute(_, W, WLast) :- 
    W > WLast, !.

compute(ILast, WFirst, WLast) :- 
%    writeln(ILast-WFirst-WLast),
    loop_i(1, ILast, WFirst, WLast).

worker(IL, WL, WO) :-  
    mutex_lock(qlock),    
    get_value(qsize, W),
    (W =< WL ->
      W1 is W + WO,
      set_value(qsize, W1),
      mutex_unlock(qlock),
      W2 is W1 - 1,
      compute(IL, W, W2),
      worker(IL, WL, WO); 
     mutex_unlock(qlock)).

run_query(WorkerOffset, T):- 
    data_call(IL, WL),
    statistics(walltime,[InitTime,_]),
    forall(between(1,T,_),
	   thread_create(worker(IL, WL, WorkerOffset), _)),
    forall(between(1, T, Tid2), 
	   thread_join(Tid2,_)), 
    statistics(walltime,[EndTime,_]),
    Time is EndTime - InitTime,
    writeln('Threads-Time:'-T-Time).
%    lcs(IL, WL, V),
%    writeln('Result'-V).

%:- run_query(5, 32).

%:- data_call(IL, WL),loop_i(1,IL,1,WL),lcs(IL,WL,V), writeln(result-V).
%:- halt.


