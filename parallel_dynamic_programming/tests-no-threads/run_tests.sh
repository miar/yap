#!/bin/bash

# # run yap
TESTS_SET="tests_knapsack_v01_random_v01  tests_knapsack_v02  tests_lcs_v01_random_v01  tests_lcs_v02 tests_knapsack_v01_no_random tests_lcs_v01_no_random tests_knapsack_v01_random_v02 tests_lcs_v01_random_v02"

echo "YAPTAB IN USE"
for test_set in ${TESTS_SET}; do
    cd ${test_set}    
    del results/*
    echo "LAUNCHING SET  -----------------> " ${test_set}
    sh run_tests.sh
    cd ..
done


