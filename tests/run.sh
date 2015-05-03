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

sources=`find . -type f -name 'test_*.cxx'`

cc="g++"
cflags="`root-config --cflags` -g -O3 -Wall"
lib="-L../lib -ljudstorage -ljudmechanics `root-config --ldflags --glibs` -O1"
inc="-I../include"

rm -rf bin/
mkdir bin/

for s in $sources; do
    name=`basename $s | head -c-5`
    $cc -o bin/$name $cflags $inc $s $lib
    if [[ $? != 0 ]]; then
        echo "ERROR: compilation failed"
        exit -1
    fi
done

tests=`ls bin/`
ntests=`echo $tests | wc -w`

echo "Running $ntests tests:"

for t in $tests; do
    echo -e "\n=== $t starting ==="
    bin/$t

    if [[ $? == 0 ]]; then
        retstat="success"
    else
        retstat="failure"
    fi

    echo -e "=== $t $retstat ==="
done

