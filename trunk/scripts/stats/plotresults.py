#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import datetime
from mysqlBase import *

import pickle;
import pandas as pd;

import numpy as np
import scipy.stats as s
import matplotlib.pyplot as plt
import matplotlib
import seaborn as sns

import MySQLdb as mdb


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

def dfinfo(df):
    print df.memory_usage()
    print df.describe()
    print df.head()
    print df.shape


def loadSQLData(con,q):
    dfq=pd.read_sql(q,con=con)
    dfq["MapPubName"] = dfq["MapPubName"].astype('category')
    dfq["AutoWalkerN"] = dfq["AutoWalkerN"].astype('category')
    dfq["SolverTypePubName"] = dfq["SolverTypePubName"].astype('category')
    return dfq

dffind = loadSQLData(con,qry_find)
dffollowsum = loadSQLData(con,qry_follow_sum)
dffollow = loadSQLData(con,qry_follow)
dfffscore = loadSQLData(con,qry_ff_score)

def plotBars(dfq, col='FirstVisibStep', col_label='First visible time (steps)', plot=sns.barplot, palette="muted", show=False):
                #sns.palplot(sns.color_palette("cubehelix", 8))
                grid = sns.FacetGrid(dfq, col="MapPubName", col_order=['FME','BRL','Tel. Square'])#,col_wrap=2)
                labels = ['CR-POMCP',  'HB-CR-POMCP', 'Ad. CR-POMCP Follower', 'Ad. HB-CR-POMCP Follower']
                #labels = ['D. Follower', 'CR-POMCP',  'HB-CR-POMCP', 'Ad. CR-POMCP Follower', 'Ad. HB-CR-POMCP Follower']
                bp = grid.map(plot,'AutoWalkerN',col,'SolverTypePubName', hue_order=labels, palette=palette)
                #g.add_legend(label_order=labels) #
                grid.axes[0][0].legend()
                grid.set_titles("{col_name}")
                grid.set_ylabels(col_label)
                grid.set_xlabels("Numb. of Dyn. Obst.")
                plt.savefig("Barplotmap-ALL-%s.png" % (palette))
                #g.savefig("Barplotmap-all-savefig.png")
                if show:
                  sns.plt.show()

plotBars(dffind,'FirstVisibStep','First visible time (steps)', sns.barplot)
plotBars(dffind,'FirstVisibStep','First visible time (steps)', sns.boxplot)
sns.plt.show()

def plotBars2(dfq, plot=sns.barplot, palette="muted"):
                #sns.palplot(sns.color_palette("cubehelix", 8))
                #grid = sns.FacetGrid(dfq, col="MapPubName", col_order=['FME','BRL','Tel. Square'])#,col_wrap=2)
                grid = sns.factorplot(x="AutoWalkerN", y="FirstVisibStep", hue="SolverTypePubName", col="MapPubName",
                    data=dfq, kind="bar", aspect=.7, legend_out=False)
                #labels = ['D. Follower', 'CR-POMCP',  'HB-CR-POMCP', 'Ad. CR-POMCP Follower', 'Ad. HB-CR-POMCP Follower']
                #bp = grid.map(plot,'AutoWalkerN','FirstVisibStep','SolverTypePubName', hue_order=labels, palette=palette)
                #g.add_legend(label_order=labels) #
                #grid.axes[0][0].legend()
                grid.set_titles("{col_name}")
                grid.set_ylabels("First visible time (steps)")
                grid.set_xlabels("Numb. of Dyn. Obst.")
                plt.savefig("Barplotmap-ALL-%s.png" % (palette))
                #g.savefig("Barplotmap-all-savefig.png")
                sns.plt.show()


def plotScatterFollow(df,plot=plt.scatter):
    grid = sns.FacetGrid(df, row="MapPubName", col="AutoWalkerN", hue="SolverTypePubName") #, col_wrap=5, size=1.5)
    grid.map(plot,"avgHidden","avgD")
    #ax=sns.regplot(x="avgHidden",y="avgD", data=dffollow)
    grid.set_titles("{row_name}-{col_name}")
    grid.set_ylabels("avg distance (m)")
    grid.set_xlabels("hidden")
    grid.axes[0][0].legend()


plotScatterFollow(dffollow, sns.regplot)
plotScatterFollow(dffollow, plt.scatter)
sns.plt.show()

def checkNormal(dfq,cols,compcol):
    if len(cols)==0:
        print "checkNormal ERROR: no columns to show"
        return
    #col
    col = cols[0]
    # get values of columns
    vals = dfq[col].unique()
    # cols to pass
    cols2 = cols[1:len(cols)]
    # results
    res = []
    # loop all
    for v in vals:
        mask = (dfq[col]==v)
        if len(cols2)==0:
            d =dfq[compcol][mask]
            # check normality
            (k2,p) = s.normaltest(d)
            res.append([v,p,np.mean(d),np.std(d),np.median(d),len(d), np.count_nonzero(~np.isnan(d))])
        else:
            retres = checkNormal(dfq[mask],cols2,compcol)
            for r in retres:
                r[0] = str(v)+'-'+r[0]
                res.append(r)
    return res

res = checkNormal(df,["MapPubName","AutoWalkerN","SolverTypePubName"],"FirstVisibStep")

def showRes(res):
    sig005 = []
    sig001 = []
    for r in res:
        p = r[1]
        if p<0.01:
             sig001.append(r)
        elif p<0.05:
             sig005.append(r)
        for i in range(len(r)):
            if isinstance(r[i],float):
                r[i] = round(r[i],3)
    n001,n005,n=len(sig001),len(sig005),len(res)
    nnotsign=n-n001-n005
    p001,p005,pnotsign=100.0*n001/n,100.0*n005/n,100.0*nnotsign/n
    print "%d (%.1f%%) highly significant (p<0.01), %d (%.1f%%) significant (p<0.05), and %d (%.1f) not significant of %d in total" % (n001,p001,n005,p005,nnotsign,pnotsign,n)
    #sig=sig001+sig005
    print "--- All Results ---"
    for (r) in res:
       print "{:<45}: p = {:>6}, mean = {:>6}, std = {:>6}, median = {:>6}, n = {:>6}, n_nonan = {:>6}".format(*r)
    if n001+n005>nnotsign:
        print "\n--- NOT sign ---"
        for r in res:
            if r[1]>=0.05:
                print "%s: %.2f" % (r[0],r[1])
    else:
        print "\n--- Sign ---"
        for r in sig001+sig005:
            print "%s: %.2f" % (r[0],r[1])

showRes(res)



TODO!!!!
 - WHY is D. Follower for tel 	sq na ????? -> CHECK normal check with the data, and check the data
 -x WHY different values than previous????? (results_time)????????????? --> CONSISTENT with mysql, check SQL of previous checks (results_time)
 -

def plotBarsT1(df,m):
    mask = df["MapPubName"]==m
    ax=sns.barplot(x="AutoWalkerN",y="FirstVisibStep",hue="SolverTypePubName",data=df[mask])
    #plt.savefig("Barplotmap-"+m+".png")
    sns.plt.show()

def testColorBars(dfq):
                pals=['deep', 'muted', 'pastel', 'bright', 'dark', 'colorblind']
                for p in pals:
                        plotBars(dfq,palette=p)

def plotFromQry(df):
    maps = df["MapPubName"].unique()

    for m in maps:
        mask = df["MapPubName"]==m
        ax=sns.barplot(x="AutoWalkerN",y="FirstVisibStep",hue="SolverTypePubName",data=df[mask])
        plt.savefig("Barplotmap-"+m+".png")
        sns.plt.show()  #TODO: subplot, change axes, add title (map) (+other queries)


def plotFromQry(con):
    maps = getList(con,qry_get_maps)

    for m in maps:
        q = qry.replace("<MAP>",m)
        dfq=pd.read_sql(q,con=con)
        ax=sns.barplot(x="AutoWalkerN",y="FirstVisibStep",hue="SolverTypeName",data=dfq)
        sns.plt.show()  #TODO: subplot, change axes, add title (map) (+other queries)


qry_get_maps = "select distinct MapName from GameExt where SeekerUserID=44 and not EndTime is null and not MapName like '%master29f%';"

#qry="""select SolverTypePubName as 'Model, AutoWalkerN as , FirstVisibStep
#    from GameExt g  left join FirstVisibStep f on g.id=f.GameID
#    where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0 and MapName='<MAP>'
#    order by MapPubName,SolverTypePubName,SimObsNoiseStd,SolverType,AutoWalkerN;"""
qry_find = """select MapPubName,SolverTypePubName, AutoWalkerN, FirstVisibStep
        from GameExt g  left join FirstVisibStep f on g.id=f.GameID
        where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0 and not MapName like '%master29f%' and not SolverTypePubName='D. Follower'
        order by MapPubName,SolverTypePubName,AutoWalkerN;"""
qry_follow_sum = """select MapPubName,SolverTypePubName, AutoWalkerN, avg(d_shEuc) avgD, std(d_shEuc) stdD, avg(IsHidden) avgHidden
from GameExt g  left join FirstClose3Step f on g.id=f.GameID left join GameLineExt l on g.id=l.GameID
where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0 and
not MapName like '%master29f%' -- and not SolverTypePubName='D. Follower'
and ActionNum>=FirstClose3Step
group by MapPubName,SolverTypePubName,AutoWalkerN
order by MapPubName,SolverTypePubName,AutoWalkerN; """
qry_follow = """select g.id as GameID, MapPubName,SolverTypePubName, AutoWalkerN, avg(d_shEuc) avgD, std(d_shEuc) stdD, avg(IsHidden) avgHidden
from GameExt g  left join FirstClose3Step f on g.id=f.GameID inner join GameLineExt l on g.id=l.GameID
where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0 and
not MapName like '%master29f%' -- and not SolverTypePubName='D. Follower'
and ActionNum>=FirstClose3Step
group by GameID,MapPubName,SolverTypePubName,AutoWalkerN
order by GameID,MapPubName,SolverTypePubName,AutoWalkerN;"""
qry_ff_score = """select MapPubName,SolverTypePubName, AutoWalkerN, First_d_sh, Close_d_sh, 1.0*Close_d_sh / First_d_sh as Score
from GameExt g  left join
        (select GameID, d_sh as First_d_sh
        from GameLine
        where ActionNum=1) as fS on g.id=fS.GameID
left join
        (select l.GameID, ActionNum as Close_d_sh
        from GameLine l left join FirstClose1Step f on l.GameID=f.GameID
        where ActionNum=f.FirstClose1Step) as fc on g.id=fc.GameID
where SeekerUserID=44 and not EndTime is null and SimObsNoiseStd=0
group by MapPubName,SolverTypePubName,AutoWalkerN
order by MapPubName,SolverTypePubName,AutoWalkerN;"""

#TODO: create queries to:
# follower: map, solver, autowalkers, firstVisibStep
#   [gl]    map, solver, autowalkers, seekerbeliefscore/distance
# approach [gl]:		    , seekerbelief/dist
# follow [gl]:			    , idem
#
#[gl]=gamelines



#-----TODO: create pandas dataframe from rows (?)
#----------SEEET HIS GET PANDAS DATA FRAME
#FROM: http://stackoverflow.com/a/11138275/1771479
#-------------

con = None
try:
    con = conDB('hsgamelog2014')
    plotFromQry(con)

except mdb.Error, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit(1)

finally:
        if con and con.open:
                con.close()






# ----- TEST ------
# from my question: http://stackoverflow.com/questions/33631163/how-to-put-the-legend-on-first-subplot-of-seaborn-facetgrid

def createTestData(dbq):
                dfqT = dfq
                dfqT["MapPubName"]
                m=dfqT["MapPubName"]=='FME'
                dfqT["MapPubName"][m] = 'F1'
                m=dfqT["MapPubName"]=='BRL'
                dfqT["MapPubName"][m] = 'F2'
                m=dfqT["MapPubName"]=='Tel. Square'
                dfqT["MapPubName"][m] = 'F3'
                labels = ['D. Follower', 'CR-POMCP',  'HB-CR-POMCP', 'Ad. CR-POMCP Follower', 'Ad. HB-CR-POMCP Follower']
                i=1
                for l in labels:
                                m=dfqT["SolverTypePubName"]==l
                                dfqT["SolverTypePubName"][m] = "So"+str(i)
                                i+=1

                dfqT["Method"]=dfqT["SolverTypePubName"]
                dfqT["Param"]=dfqT["AutoWalkerN"]
                dfqT["Time"]=dfqT["FirstVisibStep"]
                dfqT["Area"]=dfqT["MapPubName"]


def barplotall(df):
                grid = sns.FacetGrid(df, col="Area", col_order=['F1','F2','F3'])
                bp = grid.map(sns.barplot,'Param','Time','Method')
                bp.add_legend()
                bp.set_titles("{col_name}")
                bp.set_ylabels("Time (s)")
                bp.set_xlabels("Number")
                sns.plt.show()

def barplot1(df):
    mask = df['Area']=='F3'
    ax=sns.barplot(x='Param',y='Time',hue='Method',data=df[mask])
    sns.plt.show()


def barplotalltest1(df):
    grid = sns.FacetGrid(df, col="Area", col_order=['F1','F2','F3'])
    bp = grid.map(sns.barplot,'Param','Time','Method')
    Ax = bp.axes[0]
    Boxes = [item for item in Ax.get_children()
         if isinstance(item, matplotlib.patches.Rectangle)][:-1]
    legend_labels  = ['So1', 'So2', 'So3', 'So4', 'So5']
    # Create the legend patches
    legend_patches = [matplotlib.patches.Patch(color=C, label=L) for
                  C, L in zip([item.get_facecolor() for item in Boxes],
                              legend_labels)]
    # Plot the legend
    plt.legend(legend_patches)
    sns.plt.show()



sns.factorplot(x="sex", y="total_bill", hue="smoker", col="day",
                   data=tips, kind="bar", aspect=.7, legend_out=False)
sns.factorplot(x="Param", y="Time", hue="Method", col="Area",
                   data=dfqT, kind="bar", aspect=.7, legend_out=False)



def barplotalltest2(df):
    grid = sns.FacetGrid(df, col="Area", col_order=['F1','F2','F3'],col_wrap=2)
    bp = grid.map(sns.barplot,'Param','Time','Method')
    bp.add_legend()
    bp.set_titles("{col_name}")
    bp.set_ylabels("Time (s)")
    bp.set_xlabels("Number")
    sns.plt.show()

def testseveral():
                grid = sns.FacetGrid(df, col="MapPubName", col_order=['F1','F2','F3'],col_wrap=2)
                bp = grid.map(sns.barplot,'AutoWalkerN','FirstVisibStep','SolverTypePubName')
                #g = g.add_legend()

                Ax = bp.axes[0]

                # Some how for a plot of 5 bars, there are 6 patches, what is the 6th one?
                Boxes = [item for item in Ax.get_children()
                                 if isinstance(item, matplotlib.patches.Rectangle)][:-1]

                # There is no labels, need to define the labels
                legend_labels  = ['So1', 'So2', 'So3', 'So4', 'So5']

                # Create the legend patches
                legend_patches = [matplotlib.patches.Patch(color=C, label=L) for
                                                  C, L in zip([item.get_facecolor() for item in Boxes],
                                                                          legend_labels)]

                # Plot the legend
                plt.legend(legend_patches)


                bp.set_titles("{col_name}")
                bp.set_ylabels("Time (s)")
                bp.set_xlabels("Number")
                sns.plt.show()

                mask = df["MapPubName"]=='F3'
                bp=sns.barplot(x="AutoWalkerN",y="FirstVisibStep",hue="SolverTypePubName",data=df[mask])

                bp.set_titles("{col_name}")
                bp.set_ylabels("Time (s)")
                bp.set_xlabels("Number")
                sns.plt.show()

                g = sns.factorplot(x="who", y="survived", col="class",
                                                        data=titanic, saturation=.5,
                                                        kind="bar", ci=None, aspect=.6)
                (g.set_axis_labels("", "Survival Rate")
                                .set_xticklabels(["Men", "Women", "Children"])
                                .set_titles("{col_name} {col_var}")
                                .set(ylim=(0, 1))
                                .despine(left=True))

# --- OLD ----

def plotbar():
    #from: http://stackoverflow.com/questions/22481854/plot-mean-and-standard-deviation
    #see also: http://matplotlib.org/examples/api/barchart_demo.html
    #seaborn: https://stanford.edu/~mwaskom/software/seaborn/generated/seaborn.barplot.html

    #!! standard error calc with bootstrapping as done by seaborn: http://www.dummies.com/how-to/content/the-bootstrap-method-for-standard-errors-and-confi.html

    means   = [26.82,26.4,61.17,61.55]         # Mean Data
    stds    = [4.59,4.39,4.37,4.38]            # Standard deviation Data
    peakval = ['26.82','26.4','61.17','61.55'] # String array of means

    ind = np.arange(len(means))
    width = 0.35
    colours = ['red','blue','green','yellow']
    plt.figure()
    plt.title('Average Age')
    for i in range(len(means)):
        plt.bar(ind[i],means[i],width,color=colours[i],align='center',yerr=stds[i],ecolor='k')
    plt.ylabel('Age (years)')
    plt.xticks(ind,('Young Male','Young Female','Elderly Male','Elderly Female'))

    def autolabel(bars,peakval):
        for ii,bar in enumerate(bars):
            height = bars[ii]
            plt.text(ind[ii], height-5, '%s'% (peakval[ii]), ha='ce')
            #nter', va='bottom')
    autolabel(means,peakval)


def plotSetDescr(name,df):
    #print "%s: mean=%f, stdev=%f, stderr=%f" % (name,
    pass


def test():
        # load data
        data = loadMyPickle('out.pkl')
        #expected size: 8x3x3=72

        # 6 cols: phase, var, map, autowalkers, dfDescr, dfComp

        # dfCompar: [cat1,cat2,pt,pw]
        # dfDescr:  [cat,p,meanval,stdval,stderrval]

        i = 0
        line = data[i]
        plotSetDescr(line[0]+"-"+line[1]+"-"+line[2], line[4])


def plotBarsT(dfq):
                #sns.palplot(sns.color_palette("cubehelix", 8))
                g = sns.FacetGrid(dfq, col="MapPubName", col_order=['F1','F2','F3'])
                #labels = ['D. Follower', 'CR-POMCP',  'HB-CR-POMCP', 'Ad. CR-POMCP Follower', 'Ad. HB-CR-POMCP Follower']
                g = g.map(sns.barplot,'AutoWalkerN','FirstVisibStep','SolverTypePubName')
                g = g.add_legend() #label_order=labels) #
                g = g.set_titles("{col_name}")
                g = g.set_ylabels("Time (s)")
                g = g.set_xlabels("Number")
                #plt.savefig("Barplotmap-ALL-%s.png" % (palette))
                #g.savefig("Barplotmap-all-savefig.png")
                sns.plt.show()

