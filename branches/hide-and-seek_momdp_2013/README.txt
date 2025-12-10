README HIDE&SEEK
================

First compile APPL-0.95:
- go to include/appl-0.95
- do: make

LIBRARY VERION
--------------
To build the library:
cd build
cmake ..
make


Only requires: iriutils, APPL 0.95 (in include), OpenCV 2.3



CONSOLE SIMULATOR VERSION
-------------------------
To build the console command and server you need Qt 
(Ubuntu version or newest at: http://qt-project.org/downloads)
The Qt Creator Project files are:
- hsmomdp.pro: the console version to start the automated seeker
- hsserver.pro: the server for the hide&seek game
- actiongen.pro: random action generator (for automated random hider)i
In order to run the game in simulation (either against another player or an automated player) you need to install a MySQL server (http://www.mysql.com, or Ubuntu version; MySQL version 5.5, I used version 5.5.13).
Then create the database with the SQL script sql/servergamelog.sql, use the command:
 mysql -u root -p < sql/servergamelog.sql)


REQUIREMENTS:
- iriutils
- Qt 4.8 (not when building the library with cmake)
- MySQL
- Qt - MySQL Lib: libqt4-sql-mysql (use Synaptic Package Manager)
- OpenCV 2.3
- APPL 0.95 (in include/)
	(WARNING: APPL 0.95 does not work for g++/gcc 4.7 or newer [Ubuntu 12.10/13.04]
	 	  can be 'fixed' by changing the CC/CXX/Linker to gcc-4.6 / g++-4.6 )


