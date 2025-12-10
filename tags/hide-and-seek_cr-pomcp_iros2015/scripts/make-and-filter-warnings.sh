#!/bin/sh
#cat makeout2.txt | grep -i -v appl | grep -i -v from | grep -i -v constexpr | grep -i -v intrusive_ptr | grep -i -v "static const" | less
make &> makeout.tmp
cat makeout.tmp | grep -i -v appl | grep -i -v from | grep -i -v constexpr | grep -i -v intrusive_ptr | grep -i -v "static const" | grep -i -v "\^" | grep -i -v "building" | less

