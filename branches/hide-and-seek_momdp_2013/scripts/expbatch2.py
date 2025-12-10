#!/usr/bin/env python

# learn the policy
# 	expbatch pomdp_root_dir policy_dir map_dir "size:number1,number2,..;size2:num1,.." trial_name "segmenter_params" [r|s|l]
# eg:
#	expbatch ../pomdpx . ../maps "10x10:1,2;20x20,1,2,3" i1 "10 45 0" "-T 300" "expC2" l
#
# map names are assumed to have the name: map#_#x# e.g.: map3_10x10
# 
# 3 subdirectories of pomdp dir are used: 
# 1. simplerew_fs
# 2. simplerew_fsc
# 3. newrew_fs_r3t1
#
# r=random hider; s=smart hider; l=random hider list, per map and iteration [default]
#
# for the online segmentation used: robot centered, and combined robot (+base) centered


import subprocess
import os
import sys


def runCreateMapList(mapname):
	mapfile = "%s/%s.txt" % (mapdir,mapname)

	# create action list
	cmd = "./actiongen %s %s_actions_%s_%s.txt %s_actions_%s_%s" % (mapfile,mapname,exp,trial,mapname,exp,trial)
	print cmd
	subprocess.call(cmd, shell=True)


def runOffline(mapname,poltyp,typ2,subdir):
	pomdpfile = "%s/%s/%s.pomdpx" % (pdir,subdir,mapname)
	polfile = "%s/%s_%s.pol" % (poldir,mapname,poltyp)
	mapactfile = ""
	sarsoplogfile = "SARSOP_time_log_%s_%s_%s.txt" % (mapname,typ2,exp)
	user = "u_%s_%s_%s" % (mapname,typ2,exp)
	logfile = "%s_%s.log" % (user,trial)

	if opponent=='l':
		mapactfile = "%s_actions_%s_%s.txt" % (mapname,exp,trial)

	print ""
	print "POMDP file: 		"+pomdpfile
	print "Policy file:		"+polfile
	print "Log file: 		"+logfile
	print "SARSOP log:		"+sarsoplogfile
	print "User: 			"+user
	print "Map act file:		"+mapactfile
	doRun = True

	if os.path.exists(pomdpfile) == False:
		print " ERROR: pomdp file not found"
		doRun = False
	if (doRun==True):
		cmd = "./hsmomdp -o%s %s -so -u %s -m %s -p %s %s > %s" % (opponent,mapactfile,user,pomdpfile,polfile,params,logfile)
		print cmd
		subprocess.call(cmd, shell=True)

	return 0


def runOnline(mapname,poltyp,typ2,subdir,segmenter):
	pomdpfile = "%s/%s/%s.pomdpx" % (pdir,subdir,mapname)
	polfile = "%s/%s_%s.pol" % (poldir,mapname,poltyp)
	mapactfile = ""
	sarsoplogfile = "SARSOP_time_log_%s_%s_%s.txt" % (mapname,typ2,exp)
	user = "u_%s_%s_%s" % (mapname,typ2,exp)
	logfile = "%s_%s.log" % (user,trial)

	if opponent=='l':
		mapactfile = "%s_actions_%s_%s.txt" % (mapname,exp,trial)

	print ""
	print "POMDP file: 		"+pomdpfile
	print "Policy file:		"+polfile
	print "Log file: 		"+logfile
	print "SARSOP log:		"+sarsoplogfile
	print "User: 			"+user
	print "Map act file:		"+mapactfile
	doRun = True

	if os.path.exists(pomdpfile) == False:
		print " ERROR: pomdp file not found"
		doRun = False
	if (doRun==True):
		try:
			os.rename(sarsoplogfile,"SARSOP_time_log.txt")
		except OSError, e:
			print "could not move SARSOP log file from %s" % ( sarsoplogfile )

		cmd = "./hsmomdp -o%s %s -sl -u %s -m %s -p %s %s %s %s > %s" % (opponent,mapactfile,user,pomdpfile,polfile,segmenter,segmparams,params,logfile)
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
	print "Expected at least 4 parameter:"
	print '''	pomdp_root_dir policy_dir map_dir "size:number1,number2,..;size2:num1,.." trial segmentparams "params" experiment [r|s|l]'''
	print "	(r=random hider; s=smart hider; l=random hider list, per map and iteration [default])"
	sys.exit(-1)


pdir = sys.argv[1]
print "POMDP dir:	%s" % (pdir)
poldir = sys.argv[2]
print "Policy dir:	%s" % (poldir)
mapdir = sys.argv[3]
print "Map dir:		%s" % (mapdir)

trial = ''
segmparams = ''
params = ''
exp = ''
opponent = 'l'
if (len(sys.argv) > 5):
	trial = sys.argv[5]
	if (len(sys.argv) > 6):
		segmparams = sys.argv[6]
		if (len(sys.argv) > 7):
			params = sys.argv[7]
			if (len(sys.argv) > 8):
				exp = sys.argv[8]
				if (len(sys.argv) > 9):
					opponent = sys.argv[9]

print "Trial:			%s" % (trial)
print "Segment parms:		%s" % (segmparams)
print "Parameters:		%s" % (params)
print "Experiment:		%s" % (exp)
print "Opponent:		%s" % (opponent)

if not opponent in ['l','r','s','v','a','A']:
	print "Unknown opponent, expecting: l, r, s, v, a, A"
	sys.exit(-1)


mapszlist = (sys.argv[4]).split(';')



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
		
		runOffline(mapname,"srw_fs","sr_fs_off","simplerew_fs")
		runOnline(mapname,"srw_fs","sr_fs_on_gr","simplerew_fs","-gr")
		#runOnline(mapname,"srw_fs","sr_fs_on_gc","simplerew_fs","-gc")
		
		#runOffline(mapname,"srw_fsc","sr_fsc_off","simplerew_fsc")
		#runOnline(mapname,"srw_fsc","sr_fsc_on_gr","simplerew_fsc","-gr")
		#runOnline(mapname,"srw_fsc","sr_fsc_on_gc","simplerew_fsc","-gc")
		
		runOffline(mapname,"newrew" ,"newrew_off"  ,"newrew_fs_r3t1")
		#runOnline(mapname,"newrew"  ,"newrew_on_gr","newrew_fs_r3t1","-gr")
		#runOnline(mapname,"newrew"  ,"newrew_on_gc","newrew_fs_r3t1","-gc")

