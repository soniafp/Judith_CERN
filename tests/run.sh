#!/bin/bash

# Script compiles all the tests, finds their binaries and runs them

if [[ `basename $PWD` != tests ]]; then
    cd tests
    if [[ $? != 0 ]]; then
        echo "ERROR: tests directory not found"
        exit -1
    fi
fi

echo "Compiling tests..."
make clean > /dev/null
make > /dev/null 2>&1
if [[ $? != 0 ]]; then
    echo "ERROR: compilation failed"
    exit -1
fi

tests=`ls bin/`
ntests=`echo $tests | wc -w`

echo "Running $ntests tests:"

for t in $tests; do
    echo -e "\n=== $t ==="
    bin/$t

    if [[ $? == 0 ]]; then
        retstat="success"
    else
        retstat="failure"
    fi

    echo -e "=== $t $retstat ==="
done

