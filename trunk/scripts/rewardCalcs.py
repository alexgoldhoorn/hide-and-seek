#!/usr/bin/env python

def maxTriangleReward(rows,cols):
	return 2*(rows+cols)


def maxTriangleReward(sizeStr):
	(rows,cols) = sizeStrToRowsCols(sizeStr)
	return maxTriangleReward(rows,cols)
	

def sizeStrToRowsCols(sizeStr):
	sz = size.split('x')
	rows = int(sz[0])
	cols = int(sz[1])
	return (rows,cols)

def calcExpRew(d,Rmax):
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

