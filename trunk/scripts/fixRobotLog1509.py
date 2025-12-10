#!/usr/bin/env python

# importRobotLogToDB "log_files" "extra_info"
# e.g.:
# importRobotLogToDB "*.txt" "offline"

import csv
from glob import glob
import sys

#params
if (len(sys.argv) < 2):
	print "Expected at least 1 parameter:"
        print "	file_log(s) "
	sys.exit(-1)


logfiles = sys.argv[1]
print "Log file(s): %s" % (logfiles)

csvfile = None



# null check
def nc(s,minOneIsNull):
	if (s=='_' or s=='NULL' or ( minOneIsNull and s=='-1' ) ):
		return 'NULL'
	else:
		return s

def fixCSV(logfile,outfile):
	qry = None
	try:
		# csv file
		csvfile = open(logfile, 'r')
		reader = csv.reader(csvfile, delimiter=',')

                #out file
                outf = open(outfile,'w')
	
		ln = 1
                numCols = 0
		# read csv file
		for r in reader:
                        if (ln==1):
                            # header
                            numCols=len(r) # -2 because the first two fields (timestamps) are not repeated since this is only done on newline
                            #print(r)
                            isFirst=True
                            for x in r:
                                if isFirst:
                                    isFirst=False
                                else:
                                    outf.write(",")
                                outf.write(x)
                            outf.write("\n")
                        else:
                            i=0
                            # print whole line and split
                            while i<len(r):
                                #print r[i],
                                outf.write(r[i])  #TODO FIX THIS!!!
                                if i==numCols-1 or i>=numCols and (i-numCols+1)%(numCols-2)==0: # -2: since line starts with timestmaps field
                                    #print ""
                                    outf.write("\n")
                                    if len(r)>i+1: # more left
                                        #print "[-],-,", # time field
                                        outf.write("[-],-,")
                                else:
                                    #print ",",
                                    outf.write(",")
                                i += 1

                        ln += 1


        except ValueError as e:
                print "ValueError %d: %s" % (e.args[0],e.args[1])
		sys.exit(1)
        except IOError as ioe:
                print "IOError %d: %s" % (e.args[0],e.args[1])
                sys.exit(1)
	finally:
		if csvfile:
                    csvfile.close()
                if outf:
                    outf.close()
	return 0



print("Fixing log files:")
c = 0
for file in glob(logfiles):
        newFile=file.replace('.txt','.fix.txt')
        print("  - %s -> %s" % (file,newFile))
        fixCSV(file,newFile)
        c += 1


print("\nDone, fixed %d files" % (c))


