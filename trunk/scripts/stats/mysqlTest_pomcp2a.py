#!/usr/bin/python
# -*- coding: utf-8 -*-

import MySQLdb as mdb
import sys
import getpass
import datetime
import MySQLdb.cursors

from mysqlBase import *;


TABLE = "SimPOMCPOct2013" 


# query win-lose-tie %
def doQryWins():
	# get maps
	maps = getList(con,qry_get_maps.replace("<TABLE>",TABLE))
	# get hiders
	hiders = getList(con,qry_get_hiders.replace("<TABLE>",TABLE))

	# loop all quieries
	for q in qrs_win:
		#check wins for all
		printHeader(q[0])
		checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<WHERE>",""))
		print '='*TABLE_WIDTH	

		# per map
		for m in maps:
			printHeader("Map %s - %s" % (m,q[0]))
			whereStr = " AND MapName='%s'" % (m)
			checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH

		# per hider
		for h in hiders:
			printHeader("Hider %s - %s" % (h,q[0]))
			whereStr = " AND Hider='%s'" % (h)
			checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH

		# per map and hider
		for m in maps:
			for h in hiders:
				printHeader("Map %s - Hider %s - %s" % (m,h,q[0]))
				whereStr = " AND MapName='%s' AND Hider='%s' " % (m,h)
				checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<WHERE>",whereStr))
				print '='*TABLE_WIDTH


# query value comparison (duration, number of actions, ..)
def doQryCompPerMap(col):
	# get maps and hiders
	maps = getList(con,qry_get_maps.replace("<TABLE>",TABLE))
	hiders = getList(con,qry_get_hiders.replace("<TABLE>",TABLE))

	# query all
	for q in qrs_col_permap:
		# per map
		for m in maps:
			printHeader("Map %s - %s" % (m,q[0]))
			whereStr = " AND MapName='%s'" % (m)
			listCheck(con,q[1].replace("<TABLE>",TABLE).replace("<COL>",col).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH

		# per hider
		for h in hiders:
			printHeader("Hider %s - %s" % (h,q[0]))
			whereStr = " AND Hider='%s'" % (h)
			listCheck(con,q[1].replace("<TABLE>",TABLE).replace("<COL>",col).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH

		# per map and hider
		for m in maps:
			for h in hiders:
				printHeader("Map %s - Hider %s - %s" % (m,h,q[0]))
				whereStr = " AND MapName='%s' AND Hider='%s' " % (m,h)
				listCheck(con,q[1].replace("<TABLE>",TABLE).replace("<COL>",col).replace("<WHERE>",whereStr))
				print '='*TABLE_WIDTH


# --- general queries ---
qry_get_maps = "select distinct MapName from <TABLE> where version in ('po1','po2','po3','po7','po5');" # AG131118: was MapSize
qry_get_hiders = "select distinct Hider from <TABLE> where version in ('po1','po2','po3','po7','po5');" # AG131118: changed HiderCat3 -> Hider

# --- win stat queries ---

qrs_win = [
["Reward",
"""select concat_ws(',',Reward), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version in ('po1','po2','po3','po7','po5') <WHERE>
group by Reward
order by Reward"""],

["Hider",
"""select concat_ws(',',Hider ), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version in ('po1','po2','po3','po7','po5') <WHERE>
group by Hider
order by Hider"""],

["Reward,hider,map",
"""select concat_ws(',',Reward,Hider,MapSize ), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version in ('po1','po2','po3','po7','po5') <WHERE>
group by Reward,Hider,MapSize
order by Reward,Hider,MapSize"""],

]


# --- queries per (value) data column ---
qrs_col_permap = [
["Reward",
"""select concat_ws(',',Reward), <COL>
from <TABLE>
where version in ('po1','po2','po3','po7','po5') AND IsWin=1 <WHERE>"""],

["po2: depth",
"""select concat_ws(',',Hider ), <COL>
from <TABLE>
where version in ('po1','po2','po3','po7','po5') AND IsWin=1 <WHERE>"""],

["po3: exploration constant",
"""select concat_ws(',',Reward,Reward,Hider,MapSize ), <COL>
from <TABLE>
where version in ('po1','po2','po3','po7','po5') AND IsWin=1  <WHERE>"""],

]




def doAllQrs():
	print " RESULTS @ %s " % (datetime.datetime.now())
	print "\nQUERY WINS\n"
	doQryWins()

	print "\n\nQUERY DurationPerAction (per map and only WINS!!)\n"
	doQryCompPerMap('DurationPerAction')

	print "\n\nQUERY NumActions (per map and only WINS!!)\n"
	doQryCompPerMap('NumActions')

def doOneQry(q):
	printHeader(q[0])
	checkTableWTL(con,q[1].replace("<TABLE>",TABLE))
	print '='*TABLE_WIDTH	

pw = getpass.getpass()
serv = 'localhost' # '192.168.1.222'
user = 'root' # 'remote'
con = None

try:
    
	con = mdb.connect(serv, user, pw, 'hsgamelog') #, cursorclass=MySQLdb.cursors.DictCursor)

	doAllQrs()
	#doOneQry(qrs_win[-1])

	#print " --- LATEX TABLE: ----"
	#latexTable(con,qrs_win[12][1].replace("<TABLE>",TABLE))
	#latexTable(con,qry_mapidsize_summ.replace("<TABLE>",TABLE),["(win) Nr Actions","(win) Duration(s)/Action"])


	


except mdb.Error, e:
  
	print "Error %d: %s" % (e.args[0], e.args[1])
	sys.exit(1)

finally:
    
	if con and con.open:
		con.close()


