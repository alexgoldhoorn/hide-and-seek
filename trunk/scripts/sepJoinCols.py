#!/usr/bin/python
# -*- coding: utf-8 -*-

import MySQLdb as mdb
import sys
import getpass
#import testTablesFisher as t
import datetime
import MySQLdb.cursors


#def splitParams(s):
	

def splitPOMCPParams(s): 
	params = s.split('_')
	# assert sz=8
	#u_pomcpp4_Rf_d11_ns1000_nb2_x2_e20
	
	ver = params[1]
	ver = ver[5:len(ver)]

	rew = params[2]
	rew = rew[1:len(rew)]

	depth = params[3]
	depth = int(depth[1:len(depth)])

	ns = params[4]
	ns = int(ns[2:len(ns)])

	nb = params[5]
	nb = int(nb[2:len(nb)])

	x = params[6]
	x = float(x[1:len(x)])

	e = params[7]
	e = int(e[1:len(e)])

	rs = ''
	hs = 'r'
	hsp = 0 #'0%'
	g = 0.95
	if len(params)>8:
		rs = params[8]
		rs = rs[1:len(rs)]

		if len(params)>9:
			hs = params[9]
			hs = hs[2:len(hs)]
			hsp = params[10]
			print "hsp = '%s'" % (hsp)
			if len(hsp)<=3: #and hsp=='hsp':
				hsp = 0
			elif len(hsp)>1 and hsp[len(hsp)-1]=='%':
				hsp = float(hsp[3:(len(hsp)-1)]) # hsp[3:len(hsp)]
			else:
				hsp = float(hsp[3:len(hsp)])

			if len(params)>11:
				gtxt = params[11]
				if len(gtxt)>0 and gtxt[0]=='g':
					g = float(gtxt[1:len(gtxt)])
			
		

	return (s,ver,rew,depth,ns,nb,x,e,rs,hs,hsp,g)


def createQry(vals):
        return "INSERT INTO varsPomcp (name,version,reward,depth,numSim,numBelief,explorationConst,expandCount,rewardAggregType,hiderSimType,hiderSimRandomProb,discount) VALUES ('%s','%s','%s',%d,%d,%d,%f,%d,'%s','%s',%f,%f);" % vals



def dbQryTest(cursor,sql,colID):
        cursor2 = con.cursor()
	n = cursor.execute(sql)

	if n==0:
		print "(no data)"
		return

	rows = cursor.fetchall()


        for DbRow in rows: #cursor.execute(sql): # cursors are iterable
                item = DbRow[0]
                print item
                if item!=None:
			vals = splitPOMCPParams(item)
			print vals
			sql = createQry(vals)
			print sql
			lc = cursor2.execute(sql) #.rowcount
			print "line count: %d" % (lc)

                #TODO INSERT IN A TABLE, AND DOE SAME FORE OTHER COMLUMS..

        cursor2.close()



pw = getpass.getpass()
#pw='Piere'
serv = 'localhost' # '192.168.1.222'
user = 'root' # 'remote'
con = None

try:
    
	con = mdb.connect(serv, user, pw, 'hsgamelog') #, cursorclass=MySQLdb.cursors.DictCursor)

	cursor = con.cursor()

	dbQryTest(cursor,"select distinct seeker from GameListDetail where seeker like 'u_pomcpp%' and starttime>'2013-09-01';",1)
	#dbQryTest(cursor,"select 'u_pomcpp4_Rf_d11_ns1000_nb2_x2_e20' as a;",1)
	#dbQryTest(cursor,"SELECT * FROM tmp2",2)


	print 'commit? (y/n)'
	choice= raw_input().lower()
	if len(choice)>0 and choice[0]=='y':
		print 'commiting...'
		con.commit()
		print 'ok'
	else:
		print 'NOT commiting'
		
	cursor.close()


except mdb.Error, e:
  
	print "Error %d: %s" % (e.args[0], e.args[1])
	sys.exit(1)

finally:
    
	if con and con.open:
		con.close()





