#!/usr/bin/env python

# learn the policy
# 	learnOffline pomdp_root_dir pol_out_dir "size:number1,number2,..;size2:num1,.." "params"
# eg:
#	learnOffline ../pomdpx . "10x10:1,2;20x20,1,2,3" "--memory 2000"
#
# map names are assumed to have the name: map#_#x# e.g.: map3_10x10
# 
# 3 subdirectories of pomdp dir are used: 
# 1. simplerew_fs
# 2. simplerew_fsc
# 3. newrew_fs_r3t1

from glob import glob
import subprocess
import os
import sys


def stripFileName(name):
	print "stripfilename"
	print file
	ist = name.rfind("/")+1
	i1 = name.find("_",ist)
	i2 = name.find("_",i1+1)
	if (i2==-1):
		i2 = name.rfind(".")
	
	return name[ist:i2]


def runProg(name,typ,subdir):
	pomdpfile = "%s/%s/%s.pomdpx" % (pdir,subdir,name)
	polfile = "%s/%s_%s.pol" % (outdir,mapname,typ)
	logfile = "%s/%s_%s.log" % (outdir,mapname,typ)
	sarsoplogfile = "SARSOP_time_log_%s_%s.txt" % (mapname,typ)


	print "POMDP file: 	"+pomdpfile
	print "Policy file: 	"+polfile
	print "Log file: 	"+logfile
	print "SARSOP log:	"+sarsoplogfile
	doRun = True

	if os.path.exists(pomdpfile) == False:
		print " ERROR: pomdp file not found"
		doRun = False
	if (doRun==True):
		#wait for server
		#time.sleep(2)
		cmd = "appl sol_nola %s -o %s %s > %s" % (params,polfile,pomdpfile,logfile)
		print cmd
		subprocess.call(cmd, shell=True)

		#move file
		try:
			os.rename("SARSOP_time_log.txt",sarsoplogfile)
		except OSError, e:
			print "could not move SARSOP log file to %s" % ( sarsoplogfile )
	return 0


#params
if (len(sys.argv) < 4):
	print "Expected at least 1 parameter:"
	print '''	pomdp_root_dir pol_out_dir "size:number1,number2,..;size2:num1,.." "params"'''
	sys.exit(-1)


pdir = sys.argv[1]
print "POMDP dir: %s" % (pdir)
outdir = sys.argv[2]
print "Policy out dir: %s" % (outdir)
params = ''
if (len(sys.argv) > 4):
	params = sys.argv[4]
mapszlist = (sys.argv[3]).split(';')



# loop all sizes and map ids
# ("size:number1,number2,..;size2:num1,..")
for a in mapszlist:
	z = a.split(':')
	sz = z[0]
	maplist = z[1].split(',')
	for b in maplist:
		mapname = "map%s_%s" % (b,sz)
		
		runProg(mapname,"srw_fs","simplerew_fs")
		runProg(mapname,"srw_fsc","simplerew_fsc")
		runProg(mapname,"newrew","newrew_fs_r3t1")




