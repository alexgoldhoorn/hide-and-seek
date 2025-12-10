#!/usr/bin/env python

# importSARSOPLogToDB "SARSOP_log_files" "extra_info"
# e.g.:
# importSARSOPLogToDB "SARSOP*" "offline"

import csv
from glob import glob
import MySQLdb as mdb
import sys

#params
if (len(sys.argv) < 2):
	print "Expected at least 1 parameter:"
	print "	SARSOP_file_log(s) [extra info]"
	sys.exit(-1)


logfiles = sys.argv[1]
print "Log file(s): %s" % (logfiles)


extraInfo = ""

if (len(sys.argv)>2):
	extraInfo = sys.argv[2];

# db conn
con = None
csvfile = None


# connect to mysql db
try:
	con = mdb.connect('localhost', 'hsgameloguser', 'hsg@m3l0gus3r', 'hsgamelog')
except mdb.Error, e:  
	print "Error %d: %s" % (e.args[0],e.args[1])
	if con:
		con.rollback()
	sys.exit(1)		

# test
cur = con.cursor()
cur.execute("SELECT VERSION()")
data = cur.fetchone()    
print "Database version : %s " % data


def importCSV(logfile):
	newTrialQ = None
	newRowQ = None
	try:
		# csv file
		csvfile = open(logfile, 'r')
		reader = csv.reader(csvfile, delimiter=',')
	
		# insert log	
		cur.execute("INSERT INTO SarsopLog (ImportedTime,FileName,Info) VALUES (Now(),'%s','%s');" % (logfile,extraInfo) )
		cur.execute("SELECT MAX(id) FROM SarsopLog;")
		logID = cur.fetchone()
		print "Log ID: %s" % logID
		logIDS = "%d" % (logID)

		trialIDS = None

		ln = 1
		# read csv file
		for r in reader:
			#ignore header
			print "%d) '%s' '%s'" % (ln,r[0],r[1])

			if r[0]!='[time]':
				typeS = r[1].strip()
				if typeS=='OPEN LOG':
					#new trial					
					startT = r[0].replace('[','').replace(']','')
					print '** open log @ %s' % (startT)
					newTrialQ = "INSERT INTO SarsopLogTrial (FileID,StartTime,nX,nY,nO,nA,initUBoundT, initLBoundT,initBoundsT,initSampleEngineT,initSARSOPPruneT,initBeliefTreeT,initTotalT,nIt,elapsedT, nTrials,nBackups,lowerBound,upperBound,precis,nAlphas,nBeliefs,totalT) VALUES (%s,'%s'," % (logIDS,startT)
					#print newTrialQ
				elif typeS=='START':
					print '** start'
					newTrialQ += "%s,%s,%s,%s," % (r[2],r[3],r[4],r[5])
					#print newTrialQ
				elif typeS=='SOLVE INIT':
					print '** solve init'
					newTrialQ += "%s,%s,%s,%s,%s,%s,%s," % (r[2],r[3],r[4],r[5],r[6],r[7],r[8])
					#print newTrialQ
				elif typeS=='SOLVE START':
					print '** solve start'
					newTrialQ += "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)" % (r[2],r[3],r[4],r[5],r[6],r[7],r[8],r[9],r[10],r[17])
					#newTrialQ += r[2]+","+r[3]+","+r[4]+","+r[5]+","+r[6]+","+r[7]+","+r[8]+","+r[9]+","+r[10]+","+r[17]+")" #note: last time is in end of array, all between don't matter for start
					print newTrialQ
					cur.execute(newTrialQ)
					cur.execute("SELECT MAX(id) FROM SarsopLogTrial;")
					trialID = cur.fetchone()
					trialIDS = "%d" % (trialID)
					print "Trial ID: %s" % trialIDS
					newTrialQ = None 
				elif typeS=='SOLVE':
					tm = r[0].replace('[','').replace(']','')
					newRowQ = "INSERT INTO SarsopLogTrialRow (TrialID,Time,nIt,elapsedT,nTrials,nBackups,lowerBound,upperBound, precis,nAlphas,nBeliefs,lbBackT,ubBackT,nLbBack,nUbBack,sampleT,pruneT,stop,totalItT) VALUES (%s,'%s',%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)" % (trialIDS,tm,r[2],r[3],r[4],r[5],r[6],r[7],r[8],r[9],r[10],r[11],r[12],r[13],r[14],r[15],r[16],r[17],r[18])
					#print newRowQ
					cur.execute(newRowQ)
			ln+=1
		con.commit()
	except mdb.Error, e:  
		print "Error %d: %s" % (e.args[0],e.args[1])
		if con:
			con.rollback()
		print "Last queries:"
		print " new trial q: '%s'" % (newTrialQ)
		print " new row q :  '%s'" % (newRowQ)
		sys.exit(1)
		
	finally:
		if csvfile:
			csvfile.close()
	return 0




for file in glob(logfiles):
	#if file.find("SARSOP")!=-1:
	print file
	importCSV(file)




