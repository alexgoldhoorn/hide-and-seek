#!/usr/bin/python
# -*- coding: utf-8 -*-

import getpass
import sys
import MySQLdb as mdb
import MySQLdb.cursors
import pickle
import pandas as pd
import testTablesFisher as t


TABLE_WIDTH = 86


def conDB(db='hsgamelog'):
    if len(sys.argv)>1:
        pw = sys.argv[1]
    else:
        pw = getpass.getpass()
    serv = 'localhost' # '192.168.1.222'
    user = 'root' # 'remote'
    con = mdb.connect(serv, user, pw, db) #, cursorclass=MySQLdb.cursors.DictCursor)
    return con


def loadMyPickle(filename,cols=6):
    """
    Loads the pickle data, assuming a fixed number of columns
    Args:
        filename: file name with pickle data
        cols: (optional) number of columns
    Returns:
        a list if cols=1 otherwise a matrix
    """
    res=[]
    with open(filename, "rb") as f:
        while True:
            try:
                #yield pickle.load(f)
                if cols==1:
                    res.append(pickle.load(f))
                else:
                    subres = []
                    for _ in range(cols):
                        subres.append(pickle.load(f))
                    res.append(subres)
            except EOFError:
                break
    return res


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

def filterNone(tup):
    """
    returns true if 2nd item of tuple is not None
    :param tup: tuple of 2
    :return: True if  not tup[1] is None
    """
    return not tup[1] is None

def listCheck(con,sql):
    """
    Runs the SQL query which should have two columns: first the category, second the value.
    For each category the meta data is obtained: prob. indicating if it is normal, mean, std.dev., std.err.
    Then each category is compared to each other with the Wilcoxon Ranksum test and t-test

    Args:
        con: connection to db
        sql: sql query, ASSUMED to have two columns: first category, second value
    Returns:
        dfNormal: pandas dataframe with info about data sets with columns: [cat,p,meanval,stdval,stderrval]
        dfCompare: pandas dataframe with comparison results: [cat1,cat2,pt,pw]

    """
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

    #ag151103: filter none
    x = filter(filterNone, rows)

    # recast this nested tuple to a python list and flatten it so it's a proper iterable:
    x = map(list, list(x))              # change the type
    x = sum(x, [])                            # flatten

    #cat:
    cat = x[0::2]
    data = x[1::2]


    try:
        #check if it is not Decimal (TODO should be able to handle..)
        #if isinstance(data[0],(int,long,float))==False:
        #convert to float and remove null
        data = [float(x) for x in data if x is not None]
    except ValueError:
            print "??%s - %s: ERROR: %s" % (cat1, cat2, sys.exc_info()[1])
            return -1,-1,-1,-1,0,0

    n = len(data)

    if n==0:
        print "WARNING: no data after filtering"
        return -1,-1,-1,-1,0,0

    # do normal check
    nSign,resNormal = t.normal_check_cat(cat,data)
    # -> resNormal: [cat,p,meanval,stdval,stderrval]

    print ""    

    # now do cross check: t-test and wilcoxon
    nSignW,nSignT,n,resCompare = t.compare_test_cat(cat,data)
    # -> resCompare: [cat1,cat2,pt,pw]


    dfNormal = pd.DataFrame(resNormal,columns=['cat','p','meanval','stdval','stderrval'])
    dfCompare = pd.DataFrame(resCompare,columns=['cat1','cat2','pt','pw'])

    return dfNormal, dfCompare



def listCheckSets(con,sql1,sql2):
        # get data
        cur = con.cursor()
        cur.execute(sql1)
        rows1 = cur.fetchall()

        # 'num_rows' needed to reshape the 1D NumPy array returend by 'fromiter'
        # in other words, to restore original dimensions of the results set
        num_rows1 = int(cur.rowcount)

        if num_rows1==0:
                print "WARNING: no data in data set 1"
                return -1,-1,-1,-1,0,0

        cur.execute(sql2)
        rows2 = cur.fetchall()

        num_rows2 = int(cur.rowcount)

        if num_rows2==0:
                print "WARNING: no data in data set 2"
                return -1,-1,-1,-1,0,0

        # recast this nested tuple to a python list and flatten it so it's a proper iterable:
        x = map(list, list(rows1))              # change the type
        data1 = sum(x, [])                            # flatten

        #cat:
        x = map(list, list(rows2))
        data2 = sum(x, [])

        #check if it is not Decimal AND filter None
        try:
            #if isinstance(data1[0],(int,long,float))==False:
                    #convert to float
            data1 = [float(x) for x in data1 if x is not None]
            #if isinstance(data2[0],(int,long,float))==False:
                    #convert to float
            data2 = [float(x) for x in data2 if x is not None]
        except ValueError:
                print "??%s - %s: ERROR: %s" % (cat1, cat2, sys.exc_info()[1])
                return -1,-1,-1,-1,0,0


        n1 = len(data1)
        n2 = len(data2)


        if n1==0:
            print "WARNING: no data in data set 1 after filtering"
            return -1,-1,-1,-1,0,0
        if n2==0:
            print "WARNING: no data in data set 2 after filtering"
            return -1,-1,-1,-1,0,0

        p1 = t.normal_check(data1)
        p2 = t.normal_check(data2)

        print ""

        # now do cross check: t-test and wilcoxon
        (pw,pt)=t.compare_test(data1,data2)

        return pw,pt,p1,p2,n1,n2




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
