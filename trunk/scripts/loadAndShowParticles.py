#!/usr/bin/env python
# -*- coding: utf-8 -*-

import pandas as pd
import numpy as np
import io
import sys
import matplotlib.pyplot as plt

# get the csv files to show
if len(sys.argv)>1:
    files_csv = sys.argv[1:len(sys.argv)]
else:
    files_csv = ['particles_bef.csv','particles_pred.csv','particles_upd.csv']

# the colors to use in the scatter plot
colors = ['k', 'y', 'c', 'g', 'r', 'b']
# show the file
def showFileInfo(file_csv):
    print "Loading %s ..." % (file_csv)
    df = pd.read_csv(file_csv,header=None,names=['x','y','h'], delimiter=r"\s+")

    print df.describe()

    i=file_csv.find('.csv')
    if i<0:
        i=file_csv.find('.txt')
    if i>0:
        descr = file_csv[:i]
    else:
        descr = file_csv

    global colors
    plt.scatter(df['x'],df['y'],label=descr,color=colors.pop())


#show all file info
for f in files_csv:
    showFileInfo(f)

plt.xlabel('x')
plt.ylabel('y')
plt.title('particles')
plt.legend() #loc=legend_location)
plt.show()

