#!/usr/bin/python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt


import numpy as np
import pylab as p



def plotHist(data,bins=100):
    #data=np.random.normal(1,2,100000) #np.array(np.random.rand(1000))
    y,binEdges=np.histogram(data,bins=bins)
    bincenters = 0.5*(binEdges[1:]+binEdges[:-1])
    p.plot(bincenters,y,'-')
    p.show()
