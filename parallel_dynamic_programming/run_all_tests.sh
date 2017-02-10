#!/bin/bash

TESTS_SET="tests-no-threads  tests-threads"
for test_set in ${TESTS_SET}; do
    cd ${test_set}    
    echo "LAUNCHING SET  -----------------> " ${test_set}
    sh run_tests.sh
    cd ..
done


