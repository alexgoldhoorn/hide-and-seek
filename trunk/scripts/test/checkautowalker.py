#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
This script has as input a file with locations:

1,2;3,4;5,6
...

Each column is separated by a ; and the coordinates with ,
"""

import pandas as pd
import sys
import numpy as np

def read_file(fname):
    return pd.read_csv(fname,header=None,delimiter=r'[;,]',engine='python')

def check_coords_df(df):
    (rows,cols) = df.shape
    # res
    i = 0
    expCnt = (rows-1)*cols/2
    dArr = np.empty(expCnt)
    # check all data
    for r in range(0,rows-1):
        for c in range(0,cols,2):
            d1 = df.iloc[r,c:(c+2)].as_matrix() - df.iloc[r+1,c:(c+2)].as_matrix()
            d = np.sqrt( np.sum(d1*d1) )
            dArr[i] = d
            i+=1

    assert(i==expCnt)
    return dArr


if len(sys.argv)<=1:
    print("Parameter required: file")
    sys.exit()

df = read_file(sys.argv[1])
res = check_coords_df(df)
resdf = pd.DataFrame(res)

print("Resulting distribution:")
print(resdf.describe())





