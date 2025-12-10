/*
In order to move physically to other disk location:

$ sudo bash
# service mysql stop
# service apparmor stop

# cd /var/lib/mysql
# mv database /newlocation/
# ln -s /newlocation /origdbname
# chown -R mysql:mysql origdbname/
  # chown -R mysql:mysql /newlocation
# vim /etc/apparmor.d/usr.sbin.mysqld
 -> add the new data dir, with the same parameters as the original data dir

# service apparmor start
# service mysql start


Now see:
	http://stackoverflow.com/questions/67093/how-do-i-quickly-rename-a-mysql-database-change-schema-name
especially the answer:
	http://stackoverflow.com/a/2298602/1771479
*/

CREATE DATABASE IF NOT EXISTS hsgamelog2016;

-- give privilges
GRANT ALL PRIVILEGES ON hsgamelog2016.* TO hsserver@'%' IDENTIFIED BY 'hsserver_us3r_p@ss' WITH GRANT OPTION;

-- NOTE: if you want to move this database physically to another location, do that here

USE hsgamelog;


-- RENAME TABLE old_db.table TO new_db.table;

RENAME TABLE hsgamelog.Game TO hsgamelog2016.Game;
RENAME TABLE hsgamelog.GameLine TO hsgamelog2016.GameLine;
RENAME TABLE hsgamelog.GameUser TO hsgamelog2016.GameUser;
RENAME TABLE hsgamelog.GameUserLine TO hsgamelog2016.GameUserLine;
RENAME TABLE hsgamelog.MapName TO hsgamelog2016.MapName;
RENAME TABLE hsgamelog.RobotLog TO hsgamelog2016.RobotLog;
RENAME TABLE hsgamelog.RobotLogRow TO hsgamelog2016.RobotLogRow;
RENAME TABLE hsgamelog.ServerStart TO hsgamelog2016.ServerStart;
RENAME TABLE hsgamelog.SolverType TO hsgamelog2016.SolverType;
RENAME TABLE hsgamelog.tmpMultiFindStep1 TO hsgamelog2016.tmpMultiFindStep1;
RENAME TABLE hsgamelog.tmpSingleData1 TO hsgamelog2016.tmpSingleData1;
RENAME TABLE hsgamelog.tmpSingleFindStep1 TO hsgamelog2016.tmpSingleFindStep1;
RENAME TABLE hsgamelog.tmpSingleFindSteps2016_time1 TO hsgamelog2016.tmpSingleFindSteps2016_time1;
RENAME TABLE hsgamelog.tmpSingleTime1 TO hsgamelog2016.tmpSingleTime1;
RENAME TABLE hsgamelog.User TO hsgamelog2016.User;

-- now create the views, functions, and stored procedures
