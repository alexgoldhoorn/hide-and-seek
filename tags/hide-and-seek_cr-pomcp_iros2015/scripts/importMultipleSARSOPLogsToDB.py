#!/usr/bin/env python

#import os
from glob import glob
import sys
import time


if (len(sys.argv) < 2):
	print "Expected at least 1 parameters:"
	print "logFiles"
	sys.exit(-1)

#if (len(sys.argv) > 1):
logfiledir = sys.argv[1]

extraInfo = ""

if (len(sys.argv)>2):
	extraInfo = sys.argv[2];

print "log file dir	: '%s'" % (logfiledir)
print "extra info	: '%s'" % (extraInfo)


#for subdir, dirs, files in os.walk(logfiledir):
for file in glob(logfiledir):
	#if file.find("SARSOP")!=-1:
	print file


