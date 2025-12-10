#!/usr/bin/python
# -*- coding: utf-8 -*-

import MySQLdb as mdb
import sys
import getpass
import datetime
import MySQLdb.cursors

from mysqlBase import *;

import pickle;
import pandas as pd;


def loadPickle(filename):
    res=[]
    with open(filename, "rb") as f:
        while True:
            try:
                #yield pickle.load(f)
                res.append(pickle.load(f))
            except EOFError:
                break

    return res


def loadMyPickle(filename):
    res=[]
    with open(filename, "rb") as f:
        while True:
            try:
                #yield pickle.load(f)
                subres = []
                for _ in range(2):
                    subres.append(pickle.load(f))
                res.append(subres)
            except EOFError:
                break
    return res


# query value comparison (duration, number of actions, ..)
def doQryCompPerMap(pklFile):
    # get maps
    maps = getList(con,qry_get_maps)
    autoWN = [0,10,50]

    with open(pklFile,'w') as f:
        for qrytup in qrys:
            phase, var, name, qry = qrytup

            # per map
            for m in maps:
                # per param
                for aw in autoWN:
                    print "=== %s - Map %s - autowalkers=%d ===" % (name,m,aw)
                    q = qry.replace("<MAP>",m).replace("<AUTOW>",str(aw))
                    dfDesc,dfComp = listCheck(con,q)
                    print '='*TABLE_WIDTH

                    #ag151106: store
                    pickle.dump(phase,f)
                    pickle.dump(var,f)
                    pickle.dump(m,f)
                    pickle.dump(aw,f)
                    pickle.dump(dfDesc,f)
                    pickle.dump(dfComp,f)
        f.close()


# --- general queries ---
qry_get_maps = "select distinct MapName from GameExt where SeekerUserID=44 and not EndTime is null and not MapName like '%master29f%';"

# matrix with on each row: phase, variable, description, sql-query
qrys = [
["find", "steps",
"find - first visible step",
"""select concat_ws(',',left(MapName,10),left(SolverTypeName,14),AutoWalkerN) as Params, FirstVisibStep
from GameExt g  left join FirstVisibStep f on g.id=f.GameID
where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0 and MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by MapName,SolverTypeName,SimObsNoiseStd,SolverType,AutoWalkerN;""",
],
["find","beliefError",
"find - belief error",
"""select Params, SeekerBeliefScore
from tmpFindPhaseRes
where MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by Params;"""
],
["find","distance",
"find - distance (euc)",
"""select Params, d_shEuc
from tmpFindPhaseRes
where MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by Params;"""
],
["approach","steps",
"approach - close-seen steps",
"""select concat_ws(',',left(MapName,10),left(SolverTypeName,14),AutoWalkerN) as Params, FirstClose3Step-FirstVisibStep as ApproachSteps
from GameExt g  left join FirstClose3Step fc3 on g.id=fc3.GameID
        left join FirstVisibStep f on g.id=f.GameID
where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0 and MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by MapName,SolverTypeName,SimObsNoiseStd,SolverType,AutoWalkerN;"""
],
["approach","beliefError",
"approach - belief error",
"""select Params, SeekerBeliefScore
from tmpApproachPhaseRes
where MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by Params;"""
],
["approach","distance",
"approach - distance (euc)",
"""select Params, d_shEuc
from tmpApproachPhaseRes
where MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by Params;"""
],
["follow","beliefError",
"follow - belief error",
"""select Params, SeekerBeliefScore
from tmpFollowPhaseRes
where MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by Params;"""
],
["follow","distance",
"follow - distance (euc)",
"""select Params, d_shEuc
from tmpFollowPhaseRes
where MapName='<MAP>' and AutoWalkerN=<AUTOW>
order by Params;"""
]

]

#"""select concat_ws(',',left(MapName,10),left(SolverTypeName,14),AutoWalkerN) as Params, SeekerBeliefScore
#from GameExt g  left join FirstVisibStep f on g.id=f.GameID
# left join GameLineExt gl on (g.id=gl.GameID and gl.ActionNum<FirstVisibStep)
#where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0 and MapName='<MAP>' and AutoWalkerN=<AUTOW>
#order by MapName,SolverTypeName,SolverType,AutoWalkerN;""""


con = None
try:
    con = conDB('hsgamelog2014')
    doQryCompPerMap('out.pkl')
except mdb.Error, e:
	print "Error %d: %s" % (e.args[0], e.args[1])
	sys.exit(1)
finally:
	if con and con.open:
		con.close()


