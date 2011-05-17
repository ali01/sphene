#!/bin/sh
SR=`dirname $0`/build/sr
GDB_COMMAND=`dirname $0`/sphene_1s_hello.gdb
gdb --tty=$TTY --batch --command=$GDB_COMMAND --args $SR $@