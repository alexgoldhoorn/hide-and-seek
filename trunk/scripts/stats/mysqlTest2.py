#!/usr/bin/python
# -*- coding: utf-8 -*-

import MySQLdb as mdb
import sys
import getpass
import testTablesFisher as t
import datetime
import MySQLdb.cursors

TABLE_WIDTH = 86

# checkTable, sql
# sql: should be select statement returning cols: name,x_count,total_count
#   where x_count is the number of successes and, total_count the total
def checkTable(con,sql):
	cur = con.cursor()
	n = cur.execute(sql)

	if n==0:
		print "(no data)"
		return

	rows = cur.fetchall()

	#if rows.rowcount==0:
	#    print "Warning: no data for query"
	#    return
	#if rows.colcount!=3:
	#    raise Exception("the number of columns should be 3")

	print "------------------"
	print "Name	Count	Total"
	table = []
	for row in rows:
		print "%s %d %d" % (row[0],row[1],row[2])
		table.append( [row[0],int(row[1]),int(row[2])] )

	print ""

	t.test_tables_fisher_tot(table)

	print "------------------"


# checkTableWTL
# sql: should be select statement returning cols: name,win_count,tie_count,lose_count,total_count
#   where x_count is the number of successes and, total_count the total
def checkTableWTL(con,sql):
	# get data
	cur = con.cursor()
	n = cur.execute(sql)

	if n==0:
		print "(no data)"
		return


	rows = cur.fetchall()

	# data checks
	#if rows.rowcount==0:
	#    print "Warning: no data for query"
	#    return
	#if rows.colcount!=3:
	#    raise Exception("the number of columns should be 3")

	# print header

	print '{:<26} {:>8} {:>6}% {:>8} {:>6}% {:>8} {:>6}% {:>8}'.format(*['Category','Win','','Lose','','Tie','','Total'])
	print '-'*TABLE_WIDTH

	# get data
	table = []
	nWinT = nLoseT = nTieT = nTotT = 0
	for row in rows:
		#print "%s %d %d" % (row[0],row[1],row[2])
		nWin = int(row[1])
		nLose = int(row[2])
		nTie = int(row[3])
		nTot = int(row[4])
		table.append( [row[0],nWin,nTot] )
		print '{:<26} {:>8} {:>6}% {:>8} {:>6}% {:>8} {:>6}% {:>8}'.format(*[row[0][:26],nWin,round(100.0*nWin/nTot,t.ROUND_NDIT_PERC),nLose,round(100.0*nLose/nTot,t.ROUND_NDIT_PERC),nTie,round(100.0*nTie/nTot,t.ROUND_NDIT_PERC),nTot])

		nWinT += nWin
		nLoseT += nLose
		nTieT += nTie
		nTotT += nTot

	# print total
	if nTotT==0:
		print "(no data)"
	else:
		print '-'*TABLE_WIDTH
		print '{:<26} {:>8} {:>6}% {:>8} {:>6}% {:>8} {:>6}% {:>8}'.format(*['TOTAL',nWinT,round(100.0*nWinT/nTotT,t.ROUND_NDIT_PERC),nLoseT,round(100.0*nLoseT/nTotT,t.ROUND_NDIT_PERC),nTieT,round(100.0*nTieT/nTotT,t.ROUND_NDIT_PERC),nTotT])
	print ""

	t.test_tables_fisher_tot(table)


def latexTable(con,sql,xCols=[]):
	# get data
	cur = con.cursor()
	n = cur.execute(sql)

	if n==0:
		print "(no data)"
		return

	rows = cur.fetchall()

	#'{:<26} & {:>8} ({:>6}\%) & {:>8} ({:>6}\%) & {:>8} ({:>6}\%) & {:>8}\\\\'
	formatBar = '{:<26} & {:>8} ({:>6}\%) & {:>8} ({:>6}\%) & {:>8} ({:>6}\%) & {:>8}'
	header = ['Category','Win','','Lose','','Tie','','Total']
	for c in xCols:
		formatBar += ' & {:>8}+/-{:>8}'
		header.append(c + ' avg')
		header.append(c + ' std')

	formatBar += '\\\\'


	# print header

	print formatBar.format(*header)
	print '-'*TABLE_WIDTH

	# get data
	table = []
	nWinT = nLoseT = nTieT = nTotT = 0
	for row in rows:
		#print "\%s \%d \%d" \% (row[0],row[1],row[2])
		nWin = int(row[1])
		nLose = int(row[2])
		nTie = int(row[3])
		nTot = int(row[4])
		table.append( [row[0],nWin,nTot] )

		dataRow = [row[0][:26],nWin,round(100.0*nWin/nTot,t.ROUND_NDIT_PERC),nLose,round(100.0*nLose/nTot,t.ROUND_NDIT_PERC),nTie,round(100.0*nTie/nTot,t.ROUND_NDIT_PERC),nTot]
		i = 5
		for c in xCols:
			dataRow.append(round(row[i],t.ROUND_NDIT_VAL))
			i += 1
			dataRow.append(round(row[i],t.ROUND_NDIT_VAL))
			i += 1

		print formatBar.format(*dataRow)

		nWinT += nWin
		nLoseT += nLose
		nTieT += nTie
		nTotT += nTot
	# print total
	if nTotT==0:
		print "(no data)"
	else:
		print '-'*TABLE_WIDTH
		print '{:<26} & {:>8} ({:>6}\%) & {:>8} ({:>6}\%) & {:>8} ({:>6}\%) & {:>8}\\\\'.format(*['TOTAL',nWinT,round(100.0*nWinT/nTotT,t.ROUND_NDIT_PERC),nLoseT,round(100.0*nLoseT/nTotT,t.ROUND_NDIT_PERC),nTieT,round(100.0*nTieT/nTotT,t.ROUND_NDIT_PERC),nTotT])
	print ""



def listCheck(con,sql):
	# get data
	cur = con.cursor()
	cur.execute(sql)
	rows = cur.fetchall()

	# 'num_rows' needed to reshape the 1D NumPy array returend by 'fromiter' 
	# in other words, to restore original dimensions of the results set
	num_rows = int(cur.rowcount)

	if num_rows==0:
		print "(no data)"
		return

	# recast this nested tuple to a python list and flatten it so it's a proper iterable:
	x = map(list, list(rows))              # change the type
	x = sum(x, [])                            # flatten

	#cat:
	cat = x[0::2]
	data = x[1::2]

	#check if it is not Decimal (TODO should be able to handle..)
	if isinstance(data[0],(int,long,float))==False:
		#convert to float
		data = [float(x) for x in data]

	t.normal_check(cat,data)

	print ""

	# now do cross check: t-test and wilcoxon
	t.compare_test(cat,data)
	

def getList(con,qry):
	# get data
	cur = con.cursor()
	cur.execute(qry)
	rows = cur.fetchall()
	x = map(list, list(rows))
	x = sum(x, []) 
	return x

def printHeader(header):
	print '\n{:<17}|{:^50}|{:>17}\n'.format(*['='*17,header,'='*17])


def doQryWins():
	#q = qrs_win[-1]
	for q in qrs_win:	
		printHeader(q[0])
		checkTableWTL(con,q[1])
		print '='*TABLE_WIDTH	

	maps = getList(con,qry_get_maps)
	for m in maps:
		printHeader("results for map %s" % (m))
		checkTableWTL(con,qry_win_maps.replace("<MAPSIZE>",m))
		print '='*TABLE_WIDTH

	hiders = getList(con,qry_get_hiders)
	for hider in hiders:
		printHeader("results for hider %s" % (hider))
		checkTableWTL(con,qry_win_hiders.replace("<HIDER>",hider))
		print '='*TABLE_WIDTH


def doQryComp(col):
	#q = qrs_act[-1]
	for q in qrs_act:	
		printHeader(q[0])
		listCheck(con,q[1].replace("<COL>",col))
		print '='*TABLE_WIDTH	

	maps = getList(con,qry_get_maps)
	for m in maps:
		printHeader("results for map %s" % (m))
		listCheck(con,qry_act_maps.replace("<MAPSIZE>",m).replace("<COL>",col))
		print '='*TABLE_WIDTH

	hiders = getList(con,qry_get_hiders)
	for hider in hiders:
		printHeader("results for hider %s" % (hider))
		listCheck(con,qry_act_hiders.replace("<HIDER>",hider).replace("<COL>",col))
		print '='*TABLE_WIDTH


def doQryCompPerMap(col):
	maps = getList(con,qry_get_maps)
	#q = qrs_act[-1]
	for q in qrs_col_permap:
		for m in maps:	
			printHeader(col + ' - ' + q[0] + ' - ' + m)
			listCheck(con,q[1].replace("<COL>",col).replace("<MAPSIZE>",m))
			print '='*TABLE_WIDTH

	hiders = getList(con,qry_get_hiders)
	for hider in hiders:
		for m in maps:	
			printHeader("%s - Hider: %s - map %s" % (col,hider,m))
			listCheck(con,qry_col_permap_hiders.replace("<HIDER>",hider).replace("<COL>",col).replace("<MAPSIZE>",m))
			print '='*TABLE_WIDTH



# queries
# all off-online

# TESTS qryies
qry_tot = """select OnOffline, sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total 
		from SimMay2013h 
		group by OnOffline"""


qry = """select OnOffline,NumActions
	from SimMay2013h
	where IsWin=1 limit 0, 10000"""

qry_all = """select concat_ws(',',OnOffline, Reward2,Segmentation,SegmentationX,HiderCat3,MapSize,MaxTime2,SeekerType2,RobotCenteredCircles), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
	from SimMay2013h
	group by OnOffline,Reward2,Segmentation,SegmentationX,HiderCat3,MapSize,MaxTime2,SeekerType2,RobotCenteredCircles
	"""


# --- general queries ---
qry_get_maps = "select distinct MapSize from SimMay2013h"
qry_get_hiders = "select distinct HiderCat3 from SimMay2013h"

# --- win stat queries ---

qrs_win = [
["tab1: on/off seek type",
"""select concat_ws(',',OnOffline, SeekerType2), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40' and HasTimeLimit='no'
group by OnOffline,SeekerType2"""],

["tab1b: on/off seek type, hider type",
"""select concat_ws(',',SeekerType2,Reward2,HiderCat3 ), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40' and HasTimeLimit='no'
group by SeekerType2,HiderCat3,Reward2
order by SeekerType2,Reward2,HiderCat3"""],

["tab2: off Reward2s",
"""select Reward2, sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where OnOffline='offline' and HasTimeLimit='no'
group by Reward2"""],

["tab2b: off Reward2s, HiderCat3",
"""select concat_ws(',',Reward2,HiderCat3), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where OnOffline='offline' and HasTimeLimit='no'
group by Reward2,HiderCat3"""],

["tab5: seg(x), 1/2 circ",
"""select concat_ws(',',right(RobotCenteredCircles,2),right(SegmentationX,5), SeekerType2,Segmentation), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40' and HasTimeLimit='no'
and OnOffline='online' and SeekerType<>'SmartSeeker'
group by RobotCenteredCircles, SeekerType2,Segmentation,SegmentationX"""],

["tab5b: seg(x)",
"""select concat_ws(',',right(RobotCenteredCircles,2),right(SegmentationX,5),Segmentation), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40' and HasTimeLimit='no'
and OnOffline='online' and SeekerType<>'SmartSeeker'
group by RobotCenteredCircles, Segmentation,SegmentationX"""],

["tab6a: max time, online (BottomToTopRewardsAndProbs)",
"""select concat_ws(',',MaxTime2), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40'
and OnOffline='online' and SeekerType='BottomPnRew'
group by MaxTime2,SeekerType2"""],

["tab6a: max time, online (TopFinalAndReward)",
"""select concat_ws(',',MaxTime2,SeekerType2), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40'
and OnOffline='online' and SeekerType='TopFSReward'
group by MaxTime2,SeekerType2"""],

["tab7: hider type",
"""select concat_ws(',',HiderCat3), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40' and HasTimeLimit='no'
group by HiderCat3"""],

["tab7b: hider, seeker type",
"""select concat_ws(',',HiderCat3,SeekerType2), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize<>'40x40' and HasTimeLimit='no'
group by HiderCat3,SeekerType2"""],

["tab8: MapSize, seeker type",
"""select concat_ws(',',MapSize,SeekerType2), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where HasTimeLimit='no'
group by MapSize,SeekerType2"""],

["tab8b: MapSize",
"""select concat_ws(',',MapSize), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where HasTimeLimit='no'
group by MapSize"""]
]


qry_win_maps = """select concat_ws(',',OnOffline, SeekerType2,MapSize), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where MapSize='<MAPSIZE>'
group by OnOffline,SeekerType2,MapSize"""
qry_win_hiders = """select concat_ws(',',OnOffline, SeekerType2,HiderCat3), sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total
from SimMay2013h
where HiderCat3='<HIDER>'
and MapSize<>'40x40'
group by OnOffline,SeekerType2,HiderCat3"""



qrs_act = [
["tab1: on/off with diff online (momdp + smart)",
"""select concat_ws(',',OnOffline, SeekerType2), <COL>
from SimMay2013h
where IsWin=1 and MapSize<>'40x40'"""], 

["Table 2: Rewards offline",
"""select Reward2, <COL>
from SimMay2013h
where OnOffline='offline' and IsWin=1"""],

["SeekerType2 seg (x),1/2c,..",
"""select concat_ws(',',right(RobotCenteredCircles,2), SeekerType2,right(SegmentationX,5), Segmentation), <COL>
from SimMay2013h
where MapSize<>'40x40'
and OnOffline='online' and SeekerType<>'SmartSeeker'"""],

["seg (x),1/2c",
"""select concat_ws(',',right(RobotCenteredCircles,2),right(SegmentationX,5),  Segmentation), <COL>
from SimMay2013h
where MapSize<>'40x40'
and OnOffline='online' and SeekerType<>'SmartSeeker'"""],

["max time (BottomToTopRewardsAndProbs)",
"""select concat_ws(',',MaxTime2), <COL>
from SimMay2013h
where MapSize<>'40x40'
and OnOffline='online' and SeekerType='BottomPnRew'"""],

["Table 6b: max time (TopFinalAndReward)",
"""select concat_ws(',',MaxTime2,SeekerType2), <COL>
from SimMay2013h
where MapSize<>'40x40'
and OnOffline='online' and SeekerType='TopFSReward'"""],

["tab7: hider type",
"""select concat_ws(',',HiderCat3), <COL>
from SimMay2013h
where MapSize<>'40x40'"""],

["tab7b: hider, seeker type",
"""select concat_ws(',',HiderCat3,SeekerType2), <COL>
from SimMay2013h
where MapSize<>'40x40'"""]
]

qry_act_maps = """select concat_ws(',',OnOffline, SeekerType2), <COL>
from SimMay2013h
where MapSize='<MAPSIZE>'  and IsWin=1"""
qry_act_hiders= """select concat_ws(',',OnOffline, SeekerType2,HiderCat3), <COL>
from SimMay2013h
where HiderCat3='<HIDER>' 
and MapSize<>'40x40'"""


qrs_col_permap = [
["tab1: on/off with diff online (momdp + smart)",
"""select concat_ws(',',OnOffline, SeekerType2), <COL>
from SimMay2013h
where IsWin=1 and MapSize='<MAPSIZE>' and HasTimeLimit='no'"""], 

["tab1b: on/off seek type, hider type",
"""select concat_ws(',',SeekerType2,Reward2,HiderCat3 ), <COL>
from SimMay2013h
where IsWin=1 and MapSize='<MAPSIZE>' and HasTimeLimit='no'"""],

["Table 2: Rewards offline",
"""select Reward2, <COL>
from SimMay2013h
where OnOffline='offline' and IsWin=1 and MapSize='<MAPSIZE>' and HasTimeLimit='no'"""],

["SeekerType2 seg (x),1/2c,..",
"""select concat_ws(',',right(RobotCenteredCircles,2), SeekerType2,right(SegmentationX,5), Segmentation), <COL>
from SimMay2013h
where MapSize<>'<MAPSIZE>'
and OnOffline='online' and SeekerType<>'SmartSeeker' and HasTimeLimit='no'"""],

["seg (x),1/2c",
"""select concat_ws(',',right(RobotCenteredCircles,2), right(SegmentationX,5), Segmentation), <COL>
from SimMay2013h
where IsWin=1 and MapSize='<MAPSIZE>'
and OnOffline='online' and SeekerType<>'SmartSeeker' and HasTimeLimit='no'"""],

["max time (BottomToTopRewardsAndProbs)",
"""select concat_ws(',',MaxTime2), <COL>
from SimMay2013h
where IsWin=1 and MapSize='<MAPSIZE>'
and OnOffline='online' and SeekerType='BottomPnRew'"""],

["Table 6b: max time (TopFinalAndReward)",
"""select concat_ws(',',MaxTime2,SeekerType2), <COL>
from SimMay2013h
where  IsWin=1 and MapSize='<MAPSIZE>'
and OnOffline='online' and SeekerType='TopFSReward'"""],

["tab7: hider type",
"""select concat_ws(',',HiderCat3), <COL>
from SimMay2013h
where  IsWin=1 and MapSize='<MAPSIZE>' and HasTimeLimit='no'"""],

["tab7b: hider, seeker type",
"""select concat_ws(',',HiderCat3,SeekerType2), <COL>
from SimMay2013h
where  IsWin=1 and MapSize='<MAPSIZE>' and HasTimeLimit='no'"""]
]

qry_col_permap_hiders= """select concat_ws(',',OnOffline, SeekerType2,HiderCat3), <COL>
from SimMay2013h
where HiderCat3='<HIDER>' 
and IsWin=1 and MapSize='<MAPSIZE>' and HasTimeLimit='no'"""

qry_mapsize_summ = """select concat_ws(',',MapSize,SeekerType2) as N, 
sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total,
NumActions_avg,NumActions_std,DurationPerAction_avg,DurationPerAction_std
from SimMay2013h as s1 left join
(select concat_ws(',',MapSize,SeekerType2) as N2, 
avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std,
 avg(DurationPerAction) as DurationPerAction_avg, std(DurationPerAction) as DurationPerAction_std
from SimMay2013h
where HasTimeLimit='no' and IsWin=1
group by MapSize,SeekerType2
) as s2 on concat_ws(',',MapSize,SeekerType2) =s2.N2
where HasTimeLimit='no'
group by MapSize,SeekerType2
order by MapSize,SeekerType2"""


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
	checkTableWTL(con,q[1])
	print '='*TABLE_WIDTH	

pw = getpass.getpass()
#pw='Piere'
serv = 'localhost' # '192.168.1.222'
user = 'root' # 'remote'
con = None

try:
    
	con = mdb.connect(serv, user, pw, 'hsgamelog') #, cursorclass=MySQLdb.cursors.DictCursor)

	doAllQrs()
	#doOneQry(qrs_win[-1])

	print " --- LATEX TABLE: ----"
	latexTable(con,qrs_win[1][1])
	latexTable(con,qry_mapsize_summ,["(win) Nr Actions","(win) Duration(s)/Action"])


	


except mdb.Error, e:
  
	print "Error %d: %s" % (e.args[0], e.args[1])
	sys.exit(1)

finally:
    
	if con and con.open:
		con.close()


