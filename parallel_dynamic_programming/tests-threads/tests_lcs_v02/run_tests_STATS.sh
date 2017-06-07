#!/bin/bash

RESULTS_PATH="results"
#del ${RESULTS_PATH}/* 2> /dev/null
NR_RUNS=1
#PROLOG="/home/miguelareias/conferences/iclp2013/yap-bins/thesis-bins/NO_THREADS_RAND/yap"

PROLOG="/home/miguelareias/conferences/iclp2013/yap-bins/4AUSS_MD/yap"
#PROLOG="/home/miguelareias/conferences/iclp2013/yap-bins/4AUFS_MD/yap"

export LD_PRELOAD='/home/miguelareias/thesis-phd/tabmalloc/mem_alloc/tcmalloc.so'
#TESTS=$(ls data_*)
TESTS="data_size_3200_max_random_1600.pl"

THREAD_LAUNCHER="1 8 16 24 32"
for data_file in ${TESTS}; do
    echo "LAUNCHING -> " ${data_file}
    for thread_launcher in ${THREAD_LAUNCHER}; do
	echo ${thread_launcher}
	for ((i = 1; i <= ${NR_RUNS}; i++)) do
	${PROLOG} <<xxxQUERY_GOALSxxx   2>> /dev/null  # >>${RESULTS_PATH}/YAP_KNAP1_${data_file}
             ['${data_file}'].
             ['test_parallel'].    
             run_query(5, ${thread_launcher}).
             tabling_statistics.
             halt.
xxxQUERY_GOALSxxx
	done
    done    
done








