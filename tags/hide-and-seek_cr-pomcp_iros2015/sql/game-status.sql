select * 
from GameListDetailFiltered
where EndTime is NULL and StartTime>='2013-02-06 15:50:00'
order by id desc;
select * 
from GameListDetailFiltered
where Seeker like '%grid%'
order by id desc limit 0,10;
select * 
from GameListDetailFiltered
where not Seeker like '%grid%'
order by id desc limit 0,10;


select Seeker,WinState,count(*) as n,avg(NumActions) as AvgNumActions,avg(Duration) as avgDuration
from GameListDetailFiltered
where StartTime>='2013-02-06 15:50:00'
and Hider='SmartHider'
group by Seeker,WinState

select MapName,WinState,count(*) as n,avg(NumActions) as AvgNumActions,avg(Duration) as avgDuration
from GameListDetailFiltered
where StartTime>='2013-02-06 15:50:00'
group by MapName,WinState



select * 
from GameListDetail
where Reward='triangle' and MapWidth=12



-- mysql meta info
select * from information_schema.sessions;

select * from information_schema.processlist;
