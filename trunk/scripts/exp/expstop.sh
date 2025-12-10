#!/bin/sh

if [ $# -eq 0 ]; then
    mv EXP.RUN EXP.RUN.off
else
    mv EXP.RUN.off EXP.RUN
fi
