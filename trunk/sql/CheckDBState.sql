-- shop process list
show processlist;
show full processlist;

-- to stop a process use:
-- kill x;
-- (x being the process number)


-- list all databases and their size
SELECT table_schema "Data Base Name", 
sum( data_length + index_length ) / 1024 / 
1024 "Data Base Size in MB", 
sum( data_free )/ 1024 / 1024 "Free Space in MB" 
FROM information_schema.TABLES 
GROUP BY table_schema ;

-- shows what db engine does
SHOW ENGINE INNODB STATUS;

-- loop all tables
select concat( 'select * from ', table_name, ';' )
from information_schema.tables
where table_schema='hsgamelog' and table_type='BASE TABLE';
