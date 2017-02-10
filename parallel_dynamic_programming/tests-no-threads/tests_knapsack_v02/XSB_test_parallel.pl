:- import conget/2, conset/2 from machine.
:- import between/3 from basics.
set_value(X,Y) :- conset(X,Y).
get_value(X,Y) :- conget(X,Y).

%:-['data_size_2000_max_random_1000_k_1000.pl'].

%:-['../knapsack_v01/data_size_2000_max_random_1000_k_1000.pl'].

%:-['data_size_2000_k_1000_inc.pl'].

:-['matrix_knapsack.pl'].

:- set_value(qsize, 1).

compute(_, W, WLast) :- 
    W > WLast, !. 

compute(ILast, WFirst, WLast) :- 
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
 %   knap(IL, WL, V),
 %   writeln('Result'-V).


%:- run_query(5,8).
%:-get_value(qlock,X), mutex_lock(X),write(ola),mutex_unlock(X).

%:- tabling_statistics.
%:- go_no_threads. 
%:- show_all_tables.
%:- halt.

% /home/miguelareias/conferences/iclp2013/XSB/bin/xsb-mt --shared_predicates -e "['test_parallel_XSB.pl']."                


