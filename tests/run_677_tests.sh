#!/bin/sh
if [[ x"$BINARY" == x ]]; then
  echo "Specify a binary path in BINARY environment variable"
  exit 1
fi

TESTS_DIR=`dirname $0`
REF=$TESTS_DIR/../sr_ref


function run_tests() {
  echo "Running 677 tests:"
  echo "  router 0: $BINARY0"
  echo "  router 1: $BINARY1"
  echo "  router 2: $BINARY2"
  echo "  router 3: $BINARY3"
  echo "  router 4: $BINARY4"
  echo "  router 5: $BINARY5"

  time nosetests -sv $TESTS_DIR/topo677_test.py
  echo
  echo
}

export BINARY0=$BINARY
export BINARY1=$REF
export BINARY2=$BINARY
export BINARY3=$REF
export BINARY4=$BINARY
export BINARY5=$REF

run_tests;

export BINARY0=$REF
export BINARY1=$BINARY
export BINARY2=$REF
export BINARY3=$BINARY
export BINARY4=$REF
export BINARY5=$BINARY

run_tests;