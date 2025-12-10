#!/usr/bin/python
# -*- coding: utf-8 -*-

import MySQLdb as mdb
import sys
import getpass
import datetime
import MySQLdb.cursors


from mysqlBase import *;



# TABLE_WIDTH = 86
TABLE = "SimPOMCPOct2013" 



def doQryWins():
	#q = qrs_win[-1]
	for q in qrs_win:
		printHeader(q[0])
		checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<WHERE>",""))
		print '='*TABLE_WIDTH	

	maps = getList(con,qry_get_maps.replace("<TABLE>",TABLE))
	for q in qrs_win:
		for m in maps:
			printHeader("Map %s - %s" % (m,q[0]))
			whereStr = " AND MapName='%s'" % (m)
			checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH

	hiders = getList(con,qry_get_hiders.replace("<TABLE>",TABLE))
	for q in qrs_win:
		for h in hiders:
			printHeader("Hider %s - %s" % (h,q[0]))
			whereStr = " AND Hider='%s'" % (h)
			checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH


'''def doQryComp(col):
	#q = qrs_act[-1]
	for q in qrs_act:	
		printHeader(q[0])
		listCheck(con,q[1].replace("<TABLE>",TABLE).replace("<COL>",col))
		print '='*TABLE_WIDTH	

	maps = getList(con,qry_get_maps.replace("<TABLE>",TABLE))
	for m in maps:
		printHeader("results for map %s" % (m))
		listCheck(con,qry_act_maps.replace("<TABLE>",TABLE).replace("<MAPSIZE>",m).replace("<COL>",col))
		print '='*TABLE_WIDTH

	hiders = getList(con,qry_get_hiders.replace("<TABLE>",TABLE))
	for hider in hiders:
		printHeader("results for hider %s" % (hider))
		listCheck(con,qry_act_hiders.replace("<TABLE>",TABLE).replace("<HIDER>",hider).replace("<COL>",col))
		print '='*TABLE_WIDTH
'''

def doQryCompPerMap(col):
	maps = getList(con,qry_get_maps.replace("<TABLE>",TABLE))
	for q in qrs_col_permap:
		for m in maps:
			printHeader("Map %s - %s" % (m,q[0]))
			whereStr = " AND MapName='%s'" % (m)
			checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<COL>",col).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH

	hiders = getList(con,qry_get_hiders.replace("<TABLE>",TABLE))
	for q in qrs_col_permap:
		for h in hiders:
			printHeader("Hider %s - %s" % (h,q[0]))
			whereStr = " AND Hider='%s'" % (h)
			checkTableWTL(con,q[1].replace("<TABLE>",TABLE).replace("<COL>",col).replace("<WHERE>",whereStr))
			print '='*TABLE_WIDTH

# queries
# all off-online

# TESTS qryies
qry_tot = """select OnOffline, sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total 
		from <TABLE> 
		group by OnOffline"""


qry = """select OnOffline,NumActions
	from <TABLE>
	where IsWin=1 limit 0, 10000"""

qry_all = """select concat_ws(',',OnOffline, Reward3,Segmentation,SegmentationX,HiderCat3,MapSize,MaxTime2,SeekerType2,RobotCenteredCircles), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
	from <TABLE>
	group by OnOffline,Reward3,Segmentation,SegmentationX,HiderCat3,MapSize,MaxTime2,SeekerType2,RobotCenteredCircles
	"""


# --- general queries ---
qry_get_maps = "select distinct MapName from <TABLE> where version in ('po1','po2','po3','po7','po5');" # AG131118: was MapSize
qry_get_hiders = "select distinct Hider from <TABLE>;" # AG131118: changed HiderCat3 -> Hider

# --- win stat queries ---

qrs_win = [
["po1: expand count",
"""select concat_ws(',',Reward,expandCount), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version='po1' <WHERE>
group by Reward,expandCount
order by Reward,expandCount"""],

["po2: depth",
"""select concat_ws(',',Reward,depth ), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version='po2' <WHERE>
group by Reward,depth
order by Reward,depth"""],

["po3: exploration constant",
"""select concat_ws(',',Reward,explorationConst ), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version='po3' <WHERE>
group by Reward,explorationConst
order by Reward,explorationConst"""],

["po4 (po7): gamma (discount)",
"""select concat_ws(',',Reward,discount), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version='po7' <WHERE>
group by Reward,discount
order by Reward,discount"""],

["po5: number simulations",
"""select concat_ws(',',Reward,numSim), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from <TABLE>
where version='po5' <WHERE>
group by Reward,numSim
order by Reward,numSim"""]

]


qrs_col_permap = [
["po1: expand count",
"""select concat_ws(',',Reward,expandCount), <COL>
from <TABLE>
where version='po1' AND IsWin=1 <WHERE>"""],

["po2: depth",
"""select concat_ws(',',Reward,depth ), <COL>
from <TABLE>
where version='po2' AND IsWin=1 <WHERE>"""],

["po3: exploration constant",
"""select concat_ws(',',Reward,explorationConst ), <COL>
from <TABLE>
where version='po3' AND IsWin=1  <WHERE>"""],

["po4 (po7): gamma (discount)",
"""select concat_ws(',',Reward,discount), <COL>
from <TABLE>
where version='po7' AND IsWin=1  <WHERE>"""],

["po5: number simulations",
"""select concat_ws(',',Reward,numSim), <COL>
from <TABLE>
where version='po5' AND IsWin=1  <WHERE>"""]

]




qry_mapsize_summ = """select concat_ws(',',MapSize,SeekerType2) as N, 
sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total,
NumActions_avg,NumActions_std,DurationPerAction_avg,DurationPerAction_std
from <TABLE> as s1 left join
(select concat_ws(',',MapSize,SeekerType2) as N2, 
avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std,
 avg(DurationPerAction) as DurationPerAction_avg, std(DurationPerAction) as DurationPerAction_std
from <TABLE>
where HasTimeLimit='no' and IsWin=1
group by MapSize,SeekerType2
) as s2 on concat_ws(',',MapSize,SeekerType2) =s2.N2
where HasTimeLimit='no'
group by MapSize,SeekerType2
order by MapSize,SeekerType2"""


qry_mapidsize_summ = """select concat_ws(',',MapSize,SeekerType2,Reward3) as N, 
sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total,
NumActions_avg,NumActions_std,DurationPerAction_avg,DurationPerAction_std
from <TABLE> as s1 left join
(select concat_ws(',',MapSize,SeekerType2,Reward3) as N2, 
avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std,
 avg(DurationPerAction) as DurationPerAction_avg, std(DurationPerAction) as DurationPerAction_std
from <TABLE>
where HasTimeLimit='no' and IsWin=1
group by MapSize,SeekerType2,Reward3
) as s2 on concat_ws(',',MapSize,SeekerType2,Reward3) =s2.N2
where HasTimeLimit='no'
group by MapSize,SeekerType2,Reward3
order by MapSize,SeekerType2,Reward3"""


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


