import sys
import re

#prints the values as: (id, 'value')
# used to insert into the database

# usage:
# hsmomdp - A ? | python solverMapsToMySQL.py


r="^\s*(\d+)\) (.*)\s\["

for line in sys.stdin:
	m = re.match(r,line)
	if not m is None:
		print "(%s,'%s')," % (m.group(1),m.group(2))

