#!/usr/bin/python

import os
import sys




def calcTriExpRew(d,Rmax):
	g=1
	Rtot = 0
	for i in range(d):
		Rtot=Rtot+g * (Rmax - (d-i))
		g=g*0.95
	return Rtot


def calcExpRew2(d,Rmax):
	g=1
	Rtot = 0
	for i in range(d):
		Rtot=Rtot+g * (Rmax )
		g=g*0.95
	return Rtot


rows = 0
cols = 0

if len(sys.argv)<3:
	print "Expected two parameters: rows cols"
	sys.exit(-1)


rows = int(sys.argv[1])
cols = int(sys.argv[2])


maxSteps = 2*(rows+cols)
d = maxSteps
Rmax_tri = rows*cols


x_tri = calcTriExpRew(d,Rmax_tri)
x_simple = 2 # calcExpRew2(d,1)

print "For %dx%d map: " % (rows,cols)
print " depth (d) = %d" % (d)
print " exploration const (x): "
print "    triangle: %d" % (x_tri)
print "    simple: %d" % (x_simple)



