:- yap_flag(tabling_mode,[local,load_answers,local_trie]).

%:-['data_size_2000_max_random_1000.pl'].
%:-['data_size_5000_all_different.pl'].
%:-['data_size_3200_max_random_960.pl'].

:-['lcs_depth_first_rule.pl'].

run_query(T):- 
  data_call(I, K),
  statistics(walltime,[InitTime,_]),
  (time_query(I, K) ; true),
  statistics(walltime,[EndTime,_]),
  Time is EndTime - InitTime,
  writeln('Threads-Time:'-T-Time).
%  lcs(I,K,V),
%  writeln('result:'-V).
  
%:-run_query(32).
%:-halt.
  
