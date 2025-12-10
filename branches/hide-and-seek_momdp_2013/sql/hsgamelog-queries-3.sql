select *
from GameListDetail where StartTime>='2012-09-10' 
and Hider like 'ActionList%actions_2_i%'

select  * -- count(*),sum(SeekerAction>0)
from GameLine where GameID in (7039,7040,7042,7043)


select Seeker, count(*),sum(SeekerAction>0)
from GameLine gl left join GameListDetail dg on gl.GameID=dg.id
where gameid in (
select id
from GameListDetail where StartTime>='2012-09-10' 
and Hider like 'ActionList%actions_2_i%')  -- ActionList%i%
group by Seeker

select Seeker, n, nWin, nLoose, nTie, nNotFinished, 100.0*nWin/n as WinPerc, 100.0*nLoose/n as LoosePerc, 100.0*nTie/n as TiePerc
from (
select Seeker, count(*) n, sum(WinStatus=1) as nWin, sum(WinStatus=2) as nLoose, sum(WinStatus=3) as nTie,
    sum(WinStatus=0) as nNotFinished
from GameListDetail where StartTime>='2012-09-13' 
and Hider like 'ActionList%actions_i%' and Seeker like '%_src_%'
group by Seeker ) a



select Seeker, n, nWin, nLoose, nTie, nNotFinished, 100.0*nWin/n as WinPerc, 100.0*nLoose/n as LoosePerc, 100.0*nTie/n as TiePerc
from (
select Seeker, count(*) n, sum(WinStatus=1) as nWin, sum(WinStatus=2) as nLoose, sum(WinStatus=3) as nTie,
    sum(WinStatus=0) as nNotFinished
from GameListDetail where WinStatus<>0 and StartTime>='2012-08-20' 
and (Hider like 'ActionList%actions_i%' or Hider like 'RandomHider') -- and Seeker like '%_sr_%' 
group by Seeker ) a



select Seeker, WinStatus, count(*) n, avg(NumActions) AvgNumActions, std(NumActions) StdNumActions
from GameListDetail 
where StartTime>='2012-08-20' 
and Hider like 'ActionList%actions_i%' -- and Seeker like '%_sr_%'
group by Seeker,WinStatus




select * 
from GameListDetail 
where StartTime>='2012-09-13' 
    and WinStatus=0

select *
from GameLine 
where GameID in
(select id
from GameListDetail where StartTime>='2012-09-10' 
and Hider like 'ActionList%actions_i%'
-- and Seeker like 'offline%'
)


select * from (
select Hider,ActionNum,std(HiderAction) as hastd
from GameLine gl inner join GameListDetail dg on gl.GameID=dg.id
where dg.StartTime>='2012-09-10' 
and Hider like 'ActionList%actions_2_i%'
group by Hider,ActionNum;
) a where hastd<>0;

order by id



select concat('KILL ',id,';') from information_schema.processlist where user='hsserver' and Id<>(select max(Id) from information_schema.processlist where user='hsserver')


select max(id),max(StartTime)
from GameListDetail;

