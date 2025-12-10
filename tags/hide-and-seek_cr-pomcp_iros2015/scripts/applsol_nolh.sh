#!/bin/sh
echo param1: $1
echo param2: $2
appl sol $1 -o $2  --lookahead false 
