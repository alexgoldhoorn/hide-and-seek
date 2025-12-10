#!/usr/bin/env python

# importRobotLogToDB "log_files" "extra_info"
# e.g.:
# importRobotLogToDB "*.txt" "offline"

import csv
from glob import glob
import MySQLdb as mdb
import sys

#params
if (len(sys.argv) < 2):
	print "Expected at least 1 parameter:"
	print "	file_log(s) [extra info]"
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
	con = mdb.connect('localhost', 'hsserver', 'hsserver_us3r_p@ss', 'hsgamelog')
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

# null check
def nc(s,minOneIsNull):
	if (s=='_' or s=='NULL' or ( minOneIsNull and s=='-1' ) ):
		return 'NULL'
	else:
		return s

def importCSV(logfile):
	qry = None
	try:
		# csv file
		csvfile = open(logfile, 'r')
		reader = csv.reader(csvfile, delimiter=',')
	
		# insert log	
		cur.execute("INSERT INTO RobotLog (ImportedTime,FileName,Info) VALUES (Now(),'%s','%s');" % (logfile,extraInfo) )
		cur.execute("SELECT MAX(id) FROM RobotLog;")
		logID = cur.fetchone()
		print "Log ID: %s" % logID
		logIDS = "%d" % (logID)

		trialIDS = None

		#firstLine = True

		ln = 1
		# read csv file
		for r in reader:
			if (ln>1):
				if (len(r)<21):
					print "Error in logfile '%s' @ line %d, only has %d columns, instead of 21!" % (logfile,ln,len(r))
					return 0
				startT = r[0].replace('[','').replace(']','')				
				
				qry = "INSERT INTO RobotLogRow (LogID,Time,exp_name,seeker_x,seeker_y,seeker_orient,seeker_row,seeker_col,hider_visible,hider_x,hider_y,hider_row,hider_col,hider_id,distance,new_seeker_x,new_seeker_y,new_seeker_orient,new_seeker_row,new_seeker_col,win_state) VALUES (%s,'%s','%s',%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s);" % (logIDS,startT,r[2],r[3],r[4],nc(r[5],False),r[6],r[7],r[8],nc(r[9],True),nc(r[10],True),nc(r[11],True),nc(r[12],True),r[13],nc(r[14],False),nc(r[15],False),nc(r[16],False),nc(r[17],False),nc(r[18],False),nc(r[19],False),nc(r[20],False))

				cur.execute(qry)
			ln = ln + 1

		con.commit()
	except mdb.Error, e:  
		print "Error %d: %s" % (e.args[0],e.args[1])
		if con:
			con.rollback()
		print "Last query: '%s'" % (qry)
		print "Last line: %d" % (ln)
		sys.exit(1)
		
	finally:
		if csvfile:
			csvfile.close()
	return 0




for file in glob(logfiles):
	#if file.find("SARSOP")!=-1:
	print file
	importCSV(file)




