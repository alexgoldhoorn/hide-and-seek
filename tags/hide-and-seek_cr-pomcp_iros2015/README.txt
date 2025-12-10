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


Requires: g++ 4.8 or newer, iriutils, APPL 0.95 (in include), OpenCV 2.3 (in include)



CONSOLE SIMULATOR VERSION
-------------------------
To build the console command and server you need Qt 
(Ubuntu version or newest at: http://qt-project.org/downloads)
The Qt Creator Project files are:
- hsmomdp.pro: the console version to start the automated seeker
- hsserver.pro: the server for the hide&seek game
- actiongen.pro: random action generator (for automated random hider)i
In order to run the game in simulation (either against another player or an automated player) you need to install a MySQL server (http://www.mysql.com, or Ubuntu version; MySQL server 5.6 or newer (I used 5.6.14), note: by default Ubuntu (until 13.10) installs version 5.5 which does NOT support millisecond precision).
Then create the database with the SQL script sql/servergamelog.sql, use the command:
 mysql -u root -p < sql/servergamelog.sql)


REQUIREMENTS:
- g++ v4.8
- iriutils
- Qt 5 (not when building the library with cmake)
- MySQL 5.6 or newer
- Qt - MySQL Lib: libqt5-sql-mysql (use Synaptic Package Manager)
- OpenCV 2.3
- APPL 0.95 (in include/)
- Eigen v3

