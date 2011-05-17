#!/bin/sh
SR_REF=`dirname $0`/sr_ref
GDB_COMMAND=`dirname $0`/ref_1s_hello.gdb
TTY=`tty`
echo gdb --tty=$TTY --batch --command=$GDB_COMMAND --args $SR_REF $@
gdb --tty=$TTY --batch --command=$GDB_COMMAND --args $SR_REF $@