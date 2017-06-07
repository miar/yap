#!/bin/bash

RESULTS_PATH="results"
NR_RUNS=1
PROLOG="/home/miguelareias/conferences/iclp2013/XSB/bin/xsb-mt --shared_predicates "
export LD_PRELOAD='/home/miguelareias/thesis-phd/tabmalloc/mem_alloc/tcmalloc.so'
THREAD_LAUNCHER="1 8 16 24 32"

TESTS=$(ls data_*)
for data_file in ${TESTS}; do
    echo "LAUNCHING -> " ${data_file}
    for thread_launcher in ${THREAD_LAUNCHER}; do
	echo ${thread_launcher}
	for ((i = 1; i <= ${NR_RUNS}; i++)) do
	${PROLOG} <<xxxQUERY_GOALSxxx   2>> /dev/null   >> ${RESULTS_PATH}/XSB_LCS2_${data_file}
             ['${data_file}'].
             ['XSB_test_parallel'].    
             run_query(5, ${thread_launcher}).
             halt.
xxxQUERY_GOALSxxx
	done
    done    
done
