
select * from RobotLogExt;
select * from RobotLogStats;



select * 
from RobotLogRowDist

-- using rows
select map,sum(distDiff) as dist, sum(timeDiff)  as timeS, avg(distDiff/timeDiff) as avgSpeed, std(distDiff/timeDiff) as stdSpeed , count(*),
avg(timeDiff) avgTimeDiff, std(timeDiff) stdTimeDiff 
from 
(RobotLogRowDist r inner join RobotLog l on l.id=r.LogID)
inner join RobotLogMap m on l.Info=m.name_cat
-- left join MapStepSize ms on ms.map=m.map
where FileName not like '%test%'
and not FileName='experiment_gameLog.txt'
group by map;


-- freq of alg
select map,avg(timeDiff) avgTimeDiff, std(timeDiff) stdTimeDiff,
avg(1.0/timeDiff) avgF, std(1/timeDiff) stdF 
from 
(RobotLogRowDist r inner join RobotLog l on l.id=r.LogID)
inner join RobotLogMap m on l.Info=m.name_cat
-- left join MapStepSize ms on ms.map=m.map
where FileName not like '%test%'
and not FileName='experiment_gameLog.txt'
and timeDiff<10
group by map;

-- for person:
select  sum(distDiff) as dist, sum(timeDiff)  as timeS, avg(distDiff/timeDiff) as avgSpeed, std(distDiff/timeDiff) as stdSpeed , count(*) 
from 
(RobotLogRowDistPerson r inner join RobotLog l on l.id=r.LogID)
inner join RobotLogMap m on l.Info=m.name_cat
-- left join MapStepSize ms on ms.map=m.map
where FileName not like '%test%'
and not FileName='experiment_gameLog.txt'
group by map;

-- distance robot-person
select avg(distance*CellSizeM) avg_rp_dist, std(distance*CellSizeM) std_rp_dist ,avg(cellsizeM) avg_cellsize, count(*) n,
	sum(isHidden) isHidden, sum(isHidden)/count(*) hiddenPerc
from RobotLogRowExt s left join RobotLog l on s.LogID=l.id
left join RobotLogMap m on l.Info=m.name_cat
left join MapStepSize ms on m.map=ms.map
where FileName not like '%test%'
and not FileName='experiment_gameLog.txt'
group by m.map;



select * 
from RobotLog where FileName not like '%test%';


select * from MapStepSize;
select * from RobotLogMap;

alter table MapStepSize add map varchar(64);

update MapStepSize set CellSizeM=.4 where Name='BRL';



-- use summary

select * from RobotLogRowDistPerson

select * 
from RobotLog l left join RobotLogRow r on l.id=r.LogID
left join RobotLogMap m on l.Info=m.name_cat
where exp_name not like '%test%'
and map='FME';


select * from RobotLogMap






