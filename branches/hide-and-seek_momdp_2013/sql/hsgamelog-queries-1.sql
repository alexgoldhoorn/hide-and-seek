Gamedrop table WinLogTmp;
-- TEMPORARY
create  table WinLogTmp (
    Type varchar(50),
    MapName varchar(50),
    WinStatus smallint,
    n int,
    avgNumActions float
);

truncate WinLogTmp;
insert into WinLogTmp
(select '', MapName,WinStatus, count(*) as n, avg(NumActions) as NumActionsAvg 
FROM GameListDetail
where Seeker like 'simplerew%' and Hider='RandomHider' -- id>356
GROUP BY MapName,WinStatus,Hider);



select *,
    case  
        when name like 'finstate_r3t3%' then 'newrew_fs_r3t1'   
        when name like 'nfs_r3t3%' then 'nfs_r3t3'
        when name like 'newrew_fs_r3t1%' then 'newrew_fs_r3t1'
    end as GroupName
from User;
finstate_r3t3
nfs_r3t3


newrew_fs_r3t1 -> human
newrew_fs_r3t1 ->

select *
from  ServerStart where id>=3590
GameListDetail
WinLogTmp

select *,timestampdiff(second,StartTime,EndTime) as td from GameListDetail where id>356;

select g2.id as id2, timestampdiff(second, g1.EndTime,g2.StartTime)/3600.0 as LoadTime 
from GameListDetail g1 left join GameListDetail g2 on g1.id+1=g2.id
where g1.id>356;

truncate WinLogTmp;
insert into WinLogTmp
(select 0,MapName,WinStatus, count(*) as n, avg(NumActions) as NumActionsAvg 
FROM GameListDetail
where SeekerUserID between 3041 and 3051 -- Seeker like 'map3_sr_d06%'
-- like 'nfs_r3t3'  -- 'finstate_r3t3%'  -- id>356
GROUP BY MapName,WinStatus);



select * from WinLogTmp;
    
select *, 100.0*nSeekerWin/nTotal as SeekerWinPct, 100.0*nHiderWin/nTotal as HiderWinPct, 100.0*nTie/nTotal as TiePct 
from (
    select  m.MapName, coalesce(sw.n,0) as nSeekerWin, coalesce(hw.n,0) as nHiderWin, coalesce(tw.n,0) as nTie, 
        coalesce(sw.n,0)+coalesce(hw.n,0)+coalesce(tw.n,0) as nTotal
    from (((select distinct MapName from WinLogTmp ) m 
        left join WinLogTmp as sw on m.MapName=sw.MapName AND sw.WinStatus=1)
        left join WinLogTmp as hw on m.MapName=hw.MapName AND hw.WinStatus=2)
    left join WinLogTmp as tw on m.MapName=tw.MapName AND tw.WinStatus=3
) a;




select *,timestampdiff(second,StartTime,EndTime) as td from GameListDetail where id>356;

-- duration of loading
select g2.id as id2, timestampdiff(second, g1.EndTime,g2.StartTime)/3600.0 as LoadTime 
from GameListDetail g1 left join GameListDetail g2 on g1.id+1=g2.id
where g1.id>356;






select Name as Seeker,SeekerUserID,status, count(*) n
from GameLine gl inner join Game g on g.id=gl.GameID
    inner join User u on u.id=g.SeekerUserID
where GameID in (
select id from Game where SeekerUserID between 3041 and 3051)
 and status>0
group by status,SeekerUserID,Name
order by Name,status;


select * from User where id between 3041 and 3051;



select onoffline,size,avg(Duration) Duration,std(Duration) Duration_std, sum(n) as n
from (
	select Seeker,if(Seeker like '%_off','offline','online') as onoffline,
		if (Seeker like '%15x15%','15x15','12x12') as size, avg(Duration) Duration,count(*) n
	from GameListDetail
	where id >=21286 and not Duration is NULL
	group by Seeker
) t
group by onoffline,size

select *
from GameListDetail
order by id desc
where id = (select max(id) from Game)



select * -- distinct Info
from SarsopLog
order by ImportedTime desc


select count(*) -- * 
from SarsopLog where info='online_T300_2c'
order by FileName



select max(importedTime)
from SarsopLog

delete from SarsopLogTrialRow where TrialID in (
	select id from SarsopLogTrial where FileID=136	)


delete from SarsopLogTrial where FileID=136;

delete from SarsopLog where id =136;


