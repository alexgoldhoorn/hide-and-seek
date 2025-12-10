#!/usr/bin/python
# -*- coding: utf-8 -*-

import MySQLdb as mdb
import sys
import getpass
import datetime
import MySQLdb.cursors

from mysqlBase import *;


TABLE = "SimPOMCPOct2013" 

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


#def doQryCompareSets(sqlcats, sql1, sql2):



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


def save2DVec(file,header,v):
    """Save the matrix v with header to the file as a csv.
    """
    f=open(file,'w')
    v.insert(0,header)
    for r in v:
        first=True
        for x in r:
            if first:
                first=False
            else:
                f.write(',')
            f.write(str(x))

        f.write('\n')
    f.close()

def doMyCompare(qid1,qid2):
    """Compare two data sets: QueryID 1 and 2, but match the same params (solver,map,seekers,..)
    """
    #sqlparams = """select distinct concat_ws(',',MapName,SolverTypeName,nSeekers,SimObsNoiseStd,AutoWalkerN)
    #    from GameExt where (FirstSeeker like 'multiR150913%' or FirstSeeker like 'multiN150831%') and not FirstSeeker like '%_nc'
    #    order by MapName,SimObsNoiseStd,AutoWalkerN,SolverTypeName,nSeekers;"""
    sqlparams = """select distinct Params
            from TempGameListCompare
            where QueryID in (%d,%d)
            order by Params;""" % (qid1,qid2)
    params=getList(con,sqlparams)

    # sql for raw values
    sql = """select <COL>
            from <DB>
            where QueryID=<QID> and Params='<PARAM>'"""
    # db name and col
    colsDb = [  ['TempGameListCompare','min_d_sh'],
                ['TempGameListCompare','max_visDyn_sh'],
                ['TempGameListCompare','max_SeekerBeliefScore'],
                ['TempGameListCompareFirst','FirstVisibStep'],
                ['TempGameListCompareFirst','FirstClose3Step'] ]
    # qid as string
    qids1 = str(qid1)
    qids2 = str(qid2)

    print " RESULTS @ %s " % (datetime.datetime.now())

    res=[]
    #raw data
    for c in colsDb:
        for p in params:            
            printHeader("%s-%s-%s" % (c[0],c[1],p))
            # get sqls
            sqlb = sql.replace("<DB>",c[0]).replace("<PARAM>",p)
            sql1 = sqlb.replace("<COL>",c[1]).replace("<QID>",qids1)
            sql2 = sqlb.replace("<COL>",c[1]).replace("<QID>",qids2)
            print(" sql1: %s\n sql2: %s" % (sql1,sql2))
            # compare
            (pw,pt,p1,p2,n1,n2) = listCheckSets(con,sql1,sql2)

            #get avg + std
            sqlb = sqlb.replace("<COL>","avg(%s),std(%s)"%(c[1],c[1]))
            sql1 = sqlb.replace("<QID>",qids1)
            sql2 = sqlb.replace("<QID>",qids2)
            [avg1,std1] = getList(con,sql1)
            [avg2,std2] = getList(con,sql2)

            res.append(["%s-%s"%(c[0],c[1]),p,pw,pt,p1,p2,n1,n2,avg1,std1,avg2,std2])

    save2DVec('doMyCompare1Res%s-%s.txt'%(qid1,qid2),['col','MapName,SimObsNoiseStd,AutoWalkerN,SolverTypeName,nSeekers,Comm','p Wilcox comp', \
                'p t-test comp','p1-Gauss','p2-Guass','n1','n2','avg1','std1','avg2','std2'],res)

    print " FINISHED @ %s " % (datetime.datetime.now())



def conDB():
    pw = getpass.getpass()
    serv = 'localhost' # '192.168.1.222'
    user = 'root' # 'remote'
    con = mdb.connect(serv, user, pw, 'hsgamelog') #, cursorclass=MySQLdb.cursors.DictCursor)
    return con

con = None
try:
        con = conDB()

        #doAllQrs()
	#doOneQry(qrs_win[-1])

	#print " --- LATEX TABLE: ----"
	#latexTable(con,qrs_win[12][1].replace("<TABLE>",TABLE))
	#latexTable(con,qry_mapidsize_summ.replace("<TABLE>",TABLE),["(win) Nr Actions","(win) Duration(s)/Action"])

        doMyCompare(1,2)

        doMyCompare(2,4)

        doMyCompare(1,4)

	


except mdb.Error, e:
  
	print "Error %d: %s" % (e.args[0], e.args[1])
	sys.exit(1)

finally:
    
	if con and con.open:
		con.close()


