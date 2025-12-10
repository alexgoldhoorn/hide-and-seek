#!/usr/bin/env python

# learn the policy
# 	expbatch_pomcp "size:number1,number2,..;size2:num1,.." trial_name exp [r|s|l] params
# eg:
#	expbatch "10x10:1,2;20x20,1,2,3" i1 l
#
# map names are assumed to have the name: map#_#x# e.g.: map3_10x10
# 
# 3 subdirectories of pomdp dir are used: 
# 1. simplerew_fs
# 2. simplerew_fsc
# 3. newrew_fs_r3t1
#
# r=random hider; s=smart hider; l=random hider list, per map and iteration [default]
# v=very smart; ..
#
# for the online segmentation used: robot centered, and combined robot (+base) centered


import subprocess
import os
import sys
from math import ceil


def runCreateMapList(mapname):
	mapfile = "%s/%s.txt" % (mapdir,mapname)

	# create action list
	cmd = "./actiongen %s %s_actions_%s_%s.txt %s_actions_%s_%s" % (mapfile,mapname,exp,trial,mapname,exp,trial)
	print cmd
	subprocess.call(cmd, shell=True)


def runPomcp(mapname,reward,depth,numSim,numBel,explCount,expandCount):	
	mapactfile = ""
	user = "u_pomcp%s_R%s_d%d_ns%d_nb%d_x%d_e%d" % (exp,reward,depth,numSim,numBel,explCount,expandCount)
	logfile = "%s_%s.log" % (user,trial)

	if opponent=='l':
		mapactfile = "%s_actions_%s_%s.txt" % (mapname,exp,trial)

	print ""
	print "Map:        		"+mapname
	print "User: 			"+user
	print "Map act file:		"+mapactfile

	doRun = True

	if (doRun==True):
		cmd = "./hsmomdp -o%s %s -sc -u %s -m %s -R%s -d %d -ns %d -ni %d -x %d -e %d %s > expbatch_pomcp.log" % (opponent,mapactfile,user,mapname, reward, depth, numSim, numBel, explCount, expandCount, params) # ,logfile)
		print cmd
		subprocess.call(cmd, shell=True)

	return 0




def doMap(mapname,size):
	sz = size.split('x')
	rows = int(sz[0])
	cols = int(sz[1])
	cells = rows*cols
	maxAct = 2*(rows+cols)
	
	print "map: %s, size=%d x %d, maxAct=%d" % (mapname,rows,cols,maxAct)
	
	#params
	depth = [5,20,maxAct]
	numSim = [100]
	numBel = [ceil(0.05*cells),ceil(0.1*cells),ceil(0.2*cells)]
	expandC = [2,10]
	explorC_triangle = [cells,2*cells,3*cells]
	explorC_simple = [1,2,4]
	# 3x1x3x2x3	

	for d in depth:
		for ns in numSim:
			for ni in numBel:
				for e in expandC:
					for x in explorC_triangle:
						runPomcp(mapname,'t',d,ns,ni,x,e)
					for x in explorC_simple:
						runPomcp(mapname,'f',d,ns,ni,x,e)
	


#params
if (len(sys.argv) < 4):
	print "Expected at least 4 parameter:"
	print '''	"size:number1,number2,..;size2:num1,.." trial exp [r|s|l]'''
	print "	(r=random hider; s=smart hider; l=random hider list, per map and iteration [default])"
	sys.exit(-1)




trial = sys.argv[2]
exp = sys.argv[3]
opponent = sys.argv[4]
params = sys.argv[5]

if not opponent in ['l','r','s','v','a','A']:
	print "Unknown opponent, expecting: l, r, s, v, a, A"
	sys.exit(-1)

mapszlist = (sys.argv[1]).split(';')

print "Trial:			%s" % (trial)
#print "Segment parms:		%s" % (segmparams)
#print "Parameters:		%s" % (params)
print "Experiment:		%s" % (exp)
print "Opponent:		%s" % (opponent)
print "Params:			%s" % (params)

# loop all sizes and map ids
# ("size:number1,number2,..;size2:num1,..")
for a in mapszlist:
	z = a.split(':')
	sz = z[0]
	maplist = z[1].split(',')
	for b in maplist:
		mapname = "map%s_%s" % (b,sz)
		# create action list
		if opponent=='l':
			runCreateMapList(mapname)
		

		doMap(mapname,sz)
