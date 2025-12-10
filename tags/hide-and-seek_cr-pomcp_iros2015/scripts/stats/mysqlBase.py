#!/usr/bin/python
# -*- coding: utf-8 -*-


import MySQLdb as mdb
import MySQLdb.cursors

import testTablesFisher as t


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

