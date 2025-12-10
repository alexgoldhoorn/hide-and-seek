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
	print " NOTE: if using jokers for the file name(s) (*,?), then put them between quotes (e.g. \"*.txt\""
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
		print " -> Log ID: %s" % logID
		logIDS = "%d" % (logID)

		trialIDS = None

		#firstLine = True

		ln = 1
		# read csv file
		for r in reader:
			if (ln>1 and len(r)==53):
				if (len(r)<21):
					print "Error in logfile '%s' @ line %d, only has %d columns, instead of 21!" % (logfile,ln,len(r))
					return 0
				startT = r[0].replace('[','').replace(']','')				
				
                                qry = "INSERT INTO RobotLogRow (LogID,Time,exp_name,seeker_x,seeker_y,seeker_orient,seeker_row,seeker_col,hider_visible,hider_x,hider_y,hider_row,hider_col,hider_id,seeker2_x,seeker2_y,seeker2_row,seeker2_col,hidero2_x,hidero2_y,hidero2_row,hidero2_col,d_sh,d_sh2,d_s2h,d_s2h2,d_ss2,win_state,new_seeker1_froms1_x,new_seeker1_froms1_y,new_seeker1_froms1_row,new_seeker1_froms1_col,new_seeker2_froms1_x,new_seeker2_froms1_y,new_seeker2_froms1_row,new_seeker2_froms1_col,new_seeker1_froms1_belief,new_seeker2_froms1_belief,new_seeker1_froms2_x,new_seeker1_froms2_y,new_seeker1_froms2_row,new_seeker1_froms2_col,new_seeker2_froms2_x,new_seeker2_froms2_y,new_seeker2_froms2_row,new_seeker2_froms2_col,new_seeker1_froms2_belief,new_seeker2_froms2_belief,new_seeker1_goal_x,new_seeker1_goal_y,new_seeker1_goal_orient,new_seeker1_goal_row,new_seeker1_goal_col) VALUES (%s,'%s','%s',%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s);" % (logIDS,startT,r[2],r[3],r[4],nc(r[5],False),r[6],r[7],r[8],nc(r[9],True),nc(r[10],True),nc(r[11],True),nc(r[12],True),nc(r[13],True),nc(r[14],True),nc(r[15],True),nc(r[16],True),nc(r[17],True),nc(r[18],True),nc(r[19],True),nc(r[20],True),nc(r[21],True),nc(r[22],False),nc(r[23],False),nc(r[24],False),nc(r[25],False),nc(r[26],False),nc(r[27],False),nc(r[28],True),nc(r[29],True),nc(r[30],True),nc(r[31],True),nc(r[32],True),nc(r[33],True),nc(r[34],True),nc(r[35],True),nc(r[36],True),nc(r[37],True),nc(r[38],True),nc(r[39],True),nc(r[40],True),nc(r[41],True),nc(r[42],True),nc(r[43],True),nc(r[44],True),nc(r[45],True),nc(r[46],True),nc(r[47],True),nc(r[48],False),nc(r[49],False),nc(r[50],False),nc(r[51],False),nc(r[52],False))

				cur.execute(qry)
			else:
				if (ln>1):
					print "  warning: skipping line %d, because it only has %d columns, instead of %d." % (ln,len(r),53)
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
	print "Importing %s ..." % (file)
	importCSV(file)




