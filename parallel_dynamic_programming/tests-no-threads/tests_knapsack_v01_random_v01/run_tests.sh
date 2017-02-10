#!/bin/bash

RESULTS_PATH="results"
del ${RESULTS_PATH}/* 2> /dev/null
NR_RUNS=3
PROLOG="/home/miguelareias/conferences/iclp2013/yap-bins/NO_THREADS_FS_NO_TRIE/yap"
export LD_PRELOAD='/home/miguelareias/gperftools-master/tcmalloc/lib/libtcmalloc.so'

TESTS=$(ls data_*)

THREAD_LAUNCHER="1"
for data_file in ${TESTS}; do
    echo "LAUNCHING -> " ${data_file}
    for thread_launcher in ${THREAD_LAUNCHER}; do
	echo ${thread_launcher}
	for ((i = 1; i <= ${NR_RUNS}; i++)) do
	${PROLOG} <<xxxQUERY_GOALSxxx   2>> /dev/null   >>${RESULTS_PATH}/YAP_KNAP1_${data_file}
             ['${data_file}'].
             ['test_parallel'].    
             run_query(${thread_launcher}).
             halt.
xxxQUERY_GOALSxxx
	done
    done    
done





