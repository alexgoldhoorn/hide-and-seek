#!/usr/bin/env python

import sys
import numpy as np
import scipy.stats as s


#number of digits for %, p and value
ROUND_NDIT_PERC = 1
ROUND_NDIT_P = 4
ROUND_NDIT_VAL = 2

# max p value to show 
MAX_P_TO_SHOW = 0.25


#.fisher_exact 
#      y1  y2
#  x1  a   b
#  x2  c   d
#
#
# you can input several x lines as a matrix:
#
# [['x1', a1, b1],
#  ['x2', a2, b2],
#  ['x3', a3, b3],
#  ...
#  ]
#
# it will do a cross check between x1,x2,x3,..
#
def test_tables_fisher(table, alternative='two-sided'):
	res = []
	nSign = 0
	n = 0
	print "Fisher\'s exact test (%s) on (only p<%f shown):" % (alternative,MAX_P_TO_SHOW)

	for i in range(len(table)-1):
		for j in range(i+1,len(table)):	
			# AG fix 130716: when 
			if table[i][1]==0 and table[j][1]==0 or table[i][2]==0 and table[j][2]==0:
				o = -1
				p = 1
			else:
				(o,p) = s.fisher_exact([ [ table[i][1], table[i][2] ], [ table[j][1], table[j][2] ] ])
			p1 = 1.0*table[i][1] / (table[i][2]+table[i][1])
			p2 = 1.0*table[j][1] / (table[j][2]+table[j][1])
			res = [ res, [ table[i][0], table[j][0], p, o] ]
			signWarn = '  '
			signTxt = ''
			if p<0.05:
				signWarn='! '
				nSign += 1
				signTxt='<0.05'
			if p<0.01:
				signWarn='!!'
				signTxt='<0.01'
			n += 1
			
			if (p<MAX_P_TO_SHOW):
				print '%s%s (p=%f) - %s (p=%f): p = %f %s' % (signWarn,table[i][0],round(p1,ROUND_NDIT_P),table[j][0],round(p2,ROUND_NDIT_P),round(p,ROUND_NDIT_P),signTxt)

	if n==0:
		print "(no data)"
	else:
		print 'Result: %d of %d significant (p<0.05)' % (nSign,n)

	return res
			

#use this if the last column is the total
def test_tables_fisher_tot(table, alternative='two-sided'):
	for i in range(len(table)):
		table[i][2] = table[i][2] - table[i][1]
		#print table[i][2]
	return test_tables_fisher(table,alternative)


#timeWinMat = [['2s',63,122],['10s',75,130],['300s-1c',71,132],['300s-2c',66,114]]

#res = test_tables_fisher_tot(timeWinMat)

# print res



#uses D'Agostino and Pearson's 
#(http://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.normaltest.html)
def normal_check(cat,data):
	#to numpy array:
	catn = np.array(cat)
	datan = np.array(data)
	#the categories
	cats = np.unique(catn)


	# normality test for all lists
	print "Normality test (shown if p<%f):" % (MAX_P_TO_SHOW)
	
	if len(cats)==0:
		print "(no data)"
		return

	nSign = 0
	nErr = 0
	for cat in  cats:
		d = datan[catn==cat]
		if len(d)==0:
			continue
		try:
			(k2,p) = s.normaltest(d)

			signWarn = '  '
			signTxt = ''
			if p<0.05:
				signWarn = ' !'
				nSign += 1
				signTxt='<0.05'
			if p<0.01:
				signWarn = '!!'
				signTxt='<0.01'

			if p<MAX_P_TO_SHOW:
				print "{:<2}{:<26} (n={:> 5}): {:>6}+/-{:<6} ;p = {:>6} {:>8}".format(*[signWarn, cat[:26],len(d),round(np.mean(d),ROUND_NDIT_VAL),round(np.std(d),ROUND_NDIT_VAL),round(p,ROUND_NDIT_P),signTxt])
		except ValueError:
			print "??%s: ERROR: %s" % (cat, sys.exc_info()[1])
			nErr += 1
		except ZeroDivisionError:
			print "??%s: ERROR: %s" % (cat, sys.exc_info()[1])
			nErr += 1

	print "\n%d categories (potientially) normal, %d significantly different than normal, %d error(s) of %d categories in total" % (len(cats)-nSign-nErr,nSign,nErr,len(cats))


#then do for all combis a check of equality (t-test or wilcoxon..)
# uses t-test of independent samples with assumed same variance
#	(http://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.ttest_ind.html)
#and the Wilcoxon ranksum test
#	(http://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.ranksums.html)
def compare_test(cat,data):
	#to numpy array:
	catn = np.array(cat)
	datan = np.array(data)
	#the categories
	cats = np.unique(catn)


	# normality test for all lists
	print "Comparison tests (shown if p<%f):" % (MAX_P_TO_SHOW)
	

	nSignT = nErr = nSignW = 0
	n = 0

	for i in range(len(cats)-1):
		cat1 = cats[i]		
		d1 = datan[catn==cat1]

		for j in range(i+1,len(cats)):
			cat2 = cats[j]
			d2 = datan[catn==cat2]
			n += 1

			try:
				(t,pt) = s.ttest_ind(d1,d2)
				(w,pw) = s.ranksums(d1,d2)

				signWarn = '  '
				signTxt = ''
				if pt<0.05:
					nSignT += 1
				if pw<0.05:
					nSignW +=1
				if pt<0.05 or pw<0.05:
					signWarn = ' !'
					signTxt='<0.05'
				if pt<0.01 or pw<0.01:
					signWarn = '!!'
					signTxt='<0.01'

				if pt<MAX_P_TO_SHOW or pw<MAX_P_TO_SHOW:
					print "{:<2} {:<26} (n={:>6},v={:>6}+/-{:<6}) - {:<8} (n={:>6},v={:>6}+/-{:<6}): p(t-test) = {:>4}, p(wilcoxon) = {:>4} {:>6}".format(*[signWarn, cat1,len(d1),round(np.mean(d1),ROUND_NDIT_VAL),round(np.std(d1),ROUND_NDIT_VAL),cat2,len(d2),round(np.mean(d2),ROUND_NDIT_VAL),round(np.std(d2),ROUND_NDIT_VAL),round(pt,ROUND_NDIT_P),round(pw,ROUND_NDIT_P),signTxt])
			except ValueError:
				print "??%s - %s: ERROR: %s" % (cat1, cat2, sys.exc_info()[1])
				nErr += 1
	if n==0:
		print "(no data)"
	else:
		print "\n%d significantly different by t-test and %d by wilcoxon, %d error(s) of %d comparisons in total" % (nSignT,nSignW,nErr,n)



