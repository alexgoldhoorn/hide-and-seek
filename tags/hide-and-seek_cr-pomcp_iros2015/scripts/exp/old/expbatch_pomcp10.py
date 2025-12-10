#!/usr/bin/env python

# learn the policy
# 	expbatch_pomcp "size:number1,number2,..;size2:num1,.." trial_name exp opp_id server host params
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
from time import sleep


def runCreateMapList(mapname):
	mapfile = "%s/%s.txt" % (mapdir,mapname)

	# create action list
	cmd = "./actiongen %s %s_actions_%s_%s.txt %s_actions_%s_%s" % (mapfile,mapname,exp,trial,mapname,exp,trial)
	print cmd
	subprocess.call(cmd, shell=True)


def runPomcp(mapname,reward,depth,numSim,numBel,explCount,expandCount,expectedRewCount,hiderSim,hiderSimRandomP,gamma):	
	mapactfile = ""
	user = "u_pomcp%s_R%s_d%d_ns%d_nb%d_x%d_e%d_E%s_hs%s_hsp%d%s_g%f" % (exp,reward,depth,numSim,numBel,explCount,expandCount,expectedRewCount,hiderSim,int(round(100*hiderSimRandomP)),'%',gamma)
	logfile = "%s_%s.log" % (user,trial)

	if opponent=='l':
		mapactfile = "%s_actions_%s_%s.txt" % (mapname,exp,trial)

	print ""
	print "Map:        		"+mapname
	print "User: 			"+user
	print "Map act file:		"+mapactfile

	doRun = True

	if (doRun==True):
		cmd = "./hsmomdp -o%s %s -sc -u %s -m %s -R%s -d %d -ns %d -ni %d -x %d -e %d -E%s -hs%s %f -D %f %s > expbatch_pomcp.log" % (opponent,mapactfile,user,mapname, reward, depth, numSim, numBel, explCount, expandCount, expectedRewCount, hiderSim, hiderSimRandomP,gamma, params) # ,logfile)
		print cmd
		subprocess.call(cmd, shell=True)

		#run seeker
		cmdArr = cmd.split(" ")

	return 0




def doMap(mapname,size):
	sz = size.split('x')
	rows = int(sz[0])
	cols = int(sz[1])
	cells = rows*cols
	maxAct = cells # 2*(rows+cols)
	maxRowCol = max([rows,cols])

	rewardDiff = cells + maxRowCol; #
	
	print "map: %s, size=%d x %d, maxAct=%d" % (mapname,rows,cols,maxAct)
	
	#params
	depth = [maxAct] #5,10
	numSim = [10000]
	numBel = [0] # [ceil(0.05*cells),ceil(0.1*cells),ceil(0.2*cells)]
	expandC = [2]
	explorC_triangle = [rewardDiff] # [ceil(0.5*rewardDiff),rewardDiff,ceil(1.5*rewardDiff)]
	explorC_simple = [2]
	hiderSims = ['b'] # ,'r']
	hiderSimRandProb = [0.1] #0,0.1,0.5]
	gamma = [0,0.1,0.5,0.9,0.95]
	# 3x1x3x2x3	

	Rmax = rows*cols
	for g in gamma:
		for d in depth:
			for ns in numSim:
				for ni in numBel:
					for e in expandC:
						for hs in hiderSims:
							if hs=='r':
								hsimRandProb = [0]
							else:
								hsimRandProb = hiderSimRandProb
							for hsp in hsimRandProb:
								for x in explorC_simple:
									runPomcp(mapname,'f',d,ns,ni,x,e,'s',hs,hsp,g)
									#runPomcp(mapname,'c',d,ns,ni,x,e,'s',hs,hsp,g)
								for x in explorC_triangle:
									#runPomcp(mapname,'t',d,ns,ni,x,e,'s',hs,hsp,g)
									runPomcp(mapname,'t',d,ns,ni,x,e,'n',hs,hsp,g)
	


#params
if (len(sys.argv) < 4):
	print "Expected at least 4 parameter:"
	print '''	"size:number1,number2,..;size2:num1,.." trial exp [r|s|l]'''
	print "	(r=random hider; s=smart hider; l=random hider list, per map and iteration [default])"
	sys.exit(-1)




trial = sys.argv[2]
exp = sys.argv[3]
opp_id = sys.argv[4]
server = sys.argv[5]
port = sys.argv[6]
params = sys.argv[7]
mapdir = "maps"


mapszlist = (sys.argv[1]).split(';')

print "Server:			%s:%s" % (server,port)
print "Trial:			%s" % (trial)
#print "Segment parms:		%s" % (segmparams)
#print "Parameters:		%s" % (params)
print "Experiment:		%s" % (exp)
print "Opponent:		%s" % (opp_id)
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
