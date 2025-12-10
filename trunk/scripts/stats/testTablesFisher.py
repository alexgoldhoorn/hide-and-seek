#!/usr/bin/env python

#TODO: refactor this code, and rename to own stats tool

import sys
import numpy as np
import scipy.stats as s
#import matplotlib.pyplot as plt

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
                print '%s%s (p=%f) - %s (p=%f): p = %f %s' % (signWarn,table[i][0],round(p1,ROUND_NDIT_P), \
                                table[j][0],round(p2,ROUND_NDIT_P),round(p,ROUND_NDIT_P),signTxt)

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

def normal_check(d,name=''):
    """Check for normality of the data
        uses D'Agostino and Pearson's
        (http://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.normaltest.html)

     Args:
         d: a list of data
         name: (optional) name of the data
     Returns:
         p: the probability of being normal.
         mean: mean of data set
         stdev: standard dev.
         stderr: standard error (stdev/sqrt(n-1))

     """
    try:
        # do normal check
        (k2,p) = s.normaltest(d)

        signWarn = '  '
        signTxt = ''
        if p<0.05:
                signWarn = ' !'
                signTxt='<0.05'
        if p<0.01:
                signWarn = '!!'
                signTxt='<0.01'

        #ag151105: added stat values
        meanval = np.mean(d)
        stdval = np.std(d)
        stderrval = s.sem(d)

        if p<MAX_P_TO_SHOW:
                print "{:<2}{:<26} (n={:> 5}): {:>6}+/-{:<6} ;p = {:>6} {:>8}".format(*[signWarn, name[:26],len(d), \
                            round(meanval,ROUND_NDIT_VAL),round(stdval,ROUND_NDIT_VAL),round(p,ROUND_NDIT_P),signTxt])

        return p,meanval,stdval,stderrval

    except ValueError:
            print "??: ERROR: %s" % (sys.exc_info()[1])
    except ZeroDivisionError:
            print "??: ERROR: %s" % (sys.exc_info()[1])

    #return None if failed
    return None,None,None,None


def normal_check_cat(cat,data):
    """Check for normality of the data per category.

     Args:
         cat: category of each data point
         data: data
     Returns:
         numberNormal: the number of Normal categories (i.e. categories with p>0.05)
         res: matrix with columns: cat,p,mean,stdev,stderr
     """

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

    #AG151105: list of p
    res = []

    for cat in cats:
            # filter data for the category
            d = datan[catn==cat]

            if len(d)==0:
                continue

            p,meanval,stdval,stderrval = normal_check(d,cat)

            res.append([cat,p,meanval,stdval,stderrval])

            if p<0:
                nErr += 1
            elif p<0.05:
                nSign += 1

    print "\n%d categories (potientially) normal, %d significantly different than normal, %d error(s) of %d categories in total" % \
                                (len(cats)-nSign-nErr,nSign,nErr,len(cats))

    return len(cats)-nSign,res


def compare_test(d1,d2,name1='set1',name2='set2'):
    """Compare data sets d1 and d2 with t-test and Wilcoxon ranksum test.
        uses t-test of independent samples with assumed same variance
            (http://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.ttest_ind.html)
        and the Wilcoxon ranksum test
            (http://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.ranksums.html)

     Args:
         d1: data set 1
         d2: data set 2
         name1: (optional) name of data set 1
         name2: (optional) name of data set 2
     Returns:
         the probability of different using the Wilcoxon ranksum test, t-test
     """

    try:
            (t,pt) = s.ttest_ind(d1,d2)
            (w,pw) = s.ranksums(d1,d2)

            signWarn = '  '
            signTxt = ''
            if pt<0.05 or pw<0.05:
                    signWarn = ' !'
                    signTxt='<0.05'
            if pt<0.01 or pw<0.01:
                    signWarn = '!!'
                    signTxt='<0.01'

            if pt<MAX_P_TO_SHOW or pw<MAX_P_TO_SHOW:
                    print "{:<2} {:<26} (n={:>6},v={:>6}+/-{:<6}) - {:<8} (n={:>6},v={:>6}+/-{:<6}): p(t-test) = {:>4}, p(wilcoxon) = {:>4} {:>6}".format( \
                                    *[signWarn, name1,len(d1),round(np.mean(d1),ROUND_NDIT_VAL),round(np.std(d1),ROUND_NDIT_VAL),name2,len(d2), \
                                    round(np.mean(d2),ROUND_NDIT_VAL),round(np.std(d2),ROUND_NDIT_VAL),round(pt,ROUND_NDIT_P),round(pw,ROUND_NDIT_P),signTxt])

            return pw,pt
    except ValueError:
            print "??%s - %s: ERROR: %s" % (name1, name2, sys.exc_info()[1])
            nErr += 1

    #return None if failed
    return None,None



def compare_test_cat(cat,data):
    """Compare data set per category.

     Args:
         cat: category (per data point)
         data: data set
     Returns:
         n_signif_wilcox: the number of significantly different sets for the Wilcoxon test
         n_signif_t-test: number significantly diff. t-test
         n: total number of tests done
         res: matrix with columns: cat1, cat2, p_wilcox, p_t_test
     """     

    #to numpy array:
    catn = np.array(cat)
    datan = np.array(data)
    #the categories
    cats = np.unique(catn)

    # normality test for all lists
    print "Comparison tests (shown if p<%f):" % (MAX_P_TO_SHOW)


    nSignT = nErr = nSignW = 0
    n = 0
    #AG151105: added res
    res = []

    for i in range(len(cats)-1):
            cat1 = cats[i]
            d1 = datan[catn==cat1]

            for j in range(i+1,len(cats)):
                    cat2 = cats[j]
                    d2 = datan[catn==cat2]
                    n += 1

                    (pw,pt) = compare_test(d1,d2,cat1,cat2)

                    if pw<0:
                        nErr += 1
                    else:
                        if pt<0.05:
                                nSignT += 1
                        if pw<0.05:
                                nSignW +=1

                    #if pt<MAX_P_TO_SHOW or pw<MAX_P_TO_SHOW:
                    #    print "{:<2} {:<26} (n={:>6},v={:>6}+/-{:<6}) - {:<8} (n={:>6},v={:>6}+/-{:<6}): p(t-test) = {:>4}, p(wilcoxon) = {:>4} {:>6}".format(*[signWarn, cat1,len(d1),round(np.mean(d1),ROUND_NDIT_VAL),round(np.std(d1),ROUND_NDIT_VAL),cat2,len(d2),round(np.mean(d2),ROUND_NDIT_VAL),round(np.std(d2),ROUND_NDIT_VAL),round(pt,ROUND_NDIT_P),round(pw,ROUND_NDIT_P),signTxt])
                    res.append([cat1,cat2,pt,pw])

    if n==0:
            print "(no data)"
    else:
            print "\n%d significantly different by t-test and %d by wilcoxon, %d error(s) of %d comparisons in total" % (nSignT,nSignW,nErr,n)

    return nSignW,nSignT,n,res
