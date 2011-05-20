#!/bin/sh
TESTS_DIR=`dirname $0`
for i in $TESTS_DIR/topo*_test.py; do
  echo ">> running $i"
  nosetests -sv $i
  echo
  echo
done