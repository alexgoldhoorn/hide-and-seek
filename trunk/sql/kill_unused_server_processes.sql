
select concat('KILL ',id,';') from information_schema.processlist where user='hsserver' and Id<>(select max(Id) from information_schema.processlist where user='hsserver') into outfile '/tmp/a.txt';


source /tmp/a.txt;

SHOW PROCESSLIST;
