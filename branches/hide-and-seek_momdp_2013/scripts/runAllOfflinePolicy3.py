#!/usr/bin/env python
#print "Batch script to learn POMDP policies with APPL"

import subprocess
import os
import sys
import time


def stripFileName(name):
	i = name.rfind(".")
	return name[0:i]

def runProg(pomdpfile,policyfile,logfile,mapID,user):
	print "POMDP:  "+pomdpfile
	print "Policy: "+policyfile
	print "Log:    "+logfile
	print "User:   "+user
	doRun = True

	if os.path.exists(policyfile) == False:
		print " ERROR: policy file not found"
		doRun = False
	if os.path.exists(pomdpfile) == False:
		print " ERROR: pomdp file not found"
		doRun = False

	if (doRun==True):
		#wait for server
		time.sleep(2)
		cmd = "./hsmomdp -so -m "+pomdpfile+" -p "+policyfile+" -A "+mapID+" -u "+user+" -L "+user+" "+params+ " >> "+logfile	
		print cmd
		subprocess.call(cmd, shell=True)
	return 0

#
#
fileList = [ "map1_6x5_0o.pomdpx", "map2_6x5_1o.pomdpx", "map3_6x5_2o.pomdpx", 
	"map4_6x5_2o.pomdpx", "map5_6x5_4o.pomdpx" #, 
#	"map3_10x10_3o.pomdpx"
#,	"map6_10x10.pomdpx", "map3_10x10_3o.pomdpx", "map4_10x10_4o.pomdpx","map5_10x10.pomdpx", "map7_10x10.pomdpx"
		]	

mapIDlist = [ 0,1,2,3,4] #,	14



if (len(sys.argv) < 4):
	print "Expected at least 3 parameters:"
	print "runAllOfflinePolicy.py pomdp_dir policy_dir log_dir reps [params]"
	sys.exit(-1)

#if (len(sys.argv) > 1):
pomdpdir = sys.argv[1]
poldir = sys.argv[2]
logdir = sys.argv[3]
userpre = sys.argv[4]
reps = int(sys.argv[5])

params = ""
for i in range(6,len(sys.argv)):
	#print sys.argv[i]
	params += " " + sys.argv[i]

print "pomdp dir:  " + pomdpdir
print "policy dir: " + poldir
print "log dir:    " + logdir
print "user pre:   " + userpre
print "# reps:     " + str(reps)
print "params:     " + params


#for subdir, dirs, files in os.walk(pomdpdir):
#    for file in files:
#	fileext = os.path.splitext(file)
#	if fileext[1]==".pomdpx":

for r in range(reps):
	for i in range(len(fileList)):
		fileext = os.path.splitext(fileList[i])
		pomdpfile = os.path.join(pomdpdir, fileList[i])
		policyfile = os.path.join(poldir, fileext[0]+".policy")
		mapID = str(mapIDlist[i])
		user = userpre+"_"+fileext[0]+"_i"+str(r)
		logfile = os.path.join(logdir, user+"_stream.log")
		runProg(pomdpfile,policyfile,logfile,mapID,user)




