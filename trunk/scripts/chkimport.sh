#!/bin/sh
grep "SARSOP_time" import.log | while read line; do echo "mv $line ok/ " ; done > tst.sh
gedit tst.sh

