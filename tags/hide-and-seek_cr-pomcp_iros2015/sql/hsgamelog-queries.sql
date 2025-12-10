
select * from ServerStart;

select * from Game;

CREATE VIEW GameList AS
SELECT GameID, max(status) WinStatus, max(ActionNum) NumActions
FROM GameLine
GROUP BY GameID;

CREATE VIEW GameListDetail AS
select g.*, gl.WinStatus, gl.NumActions, us.Name as Seeker, uh.name as Hider
from ((Game g inner join GameList gl on gl.GameID=g.id) 
    left join User us on us.id=g.SeekerUserID)
    left join User uh on uh.id=g.HiderUserID;
where id>=6;


select * from GameListDetail
where id>=85
6

-- count # wins
-- first tmp table
drop table WinLogTmp;
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
where id>356
GROUP BY MapName,WinStatus);

select * from WinLogTmp;

-- now count wins %
select COALESCE(sw.MapName,hw.MapName,t.MapName) as MapName, sw.n as nSeekerWin, hw.n as nHiderWin, t.n as Tie, 
        sw.n+hw.n+t.n as Total

select m.Type, m.MapName, coalesce(sw.n,0) as nSeekerWin, coalesce(hw.n,0) as nHiderWin, coalesce(tw.n,0) as nTie, 
    coalesce(sw.n,0)+coalesce(hw.n,0)+coalesce(tw.n,0) as nTotal
from (((select distinct MapName from WinLogTmp ) m 
    left join WinLogTmp as sw on m.MapName=sw.MapName AND sw.WinStatus=1 AND m.Type=sw.Type)
    left join WinLogTmp as hw on m.MapName=hw.MapName AND hw.WinStatus=2 AND m.Type=hw.Type)
    left join WinLogTmp as tw on m.MapName=tw.MapName AND tw.WinStatus=3 AND m.Type=tw.Type;




select *, 100.0*nSeekerWin/nTotal as SeekerWinPct, 100.0*nHiderWin/nTotal as HiderWinPct, 100.0*nTie/nTotal as TiePct 
from (
    select m.MapName, coalesce(sw.n,0) as nSeekerWin, coalesce(hw.n,0) as nHiderWin, coalesce(tw.n,0) as nTie, 
        coalesce(sw.n,0)+coalesce(hw.n,0)+coalesce(tw.n,0) as nTotal
    from (((select distinct MapName from WinLogTmp ) m 
        left join WinLogTmp as sw on m.MapName=sw.MapName AND sw.WinStatus=1)
        left join WinLogTmp as hw on m.MapName=hw.MapName AND hw.WinStatus=2)
    left join WinLogTmp as tw on m.MapName=tw.MapName AND tw.WinStatus=3
) a;



--  show duration of game
select *,timestampdiff(second,StartTime,EndTime) as td from GameListDetail where id>356;

-- duration of loading
select g2.id as id2, timestampdiff(second, g1.EndTime,g2.StartTime)/3600.0 as LoadTime 
from GameListDetail g1 left join GameListDetail g2 on g1.id+1=g2.id
where g1.id>356;



-- old


select *,
    case  
        when name like 'finstate_r3t3%' then 'newrew_fs_r3t1'   
        when name like 'nfs_r3t3%' then 'nfs_r3t3'
        when name like 'newrew_fs_r3t1%' then 'newrew_fs_r3t1'
    end as GroupName
from User;


, WinLogTmp hw, WinLogTmp t) on
    (m.MapName=sw.MapName AND hw.MapName=m.MapName AND t.MapName=m.MapName AND sw.WinStatus=1 AND hw.WinStatus=2 AND t.WinStatus=3)


join WinLogTmp as hw on sw.MapName=hw.MapName and sw.WinStatus=1 AND hw.WinStatus=2;
    join WinLogTmp t on (t.MapName=sw.MapName OR t.MapName=hw.MapName) and t.WinStatus=3;


select *
from WinLogTmp as sw, WinLogTmp as hw 
where  sw.MapName=hw.MapName;


 (trim(sw.MapName)=trim(hw.MapName))

select * 
from Game g1 left join Game g2 on g1.MapName=g2.MapName

 (sw.MapName=hw.MapName); and sw.WinStatus=1 AND hw.WinStatus=2);



SELECT *
FROM Game;

select * from User;

select * from ServerStart;
select StopGame(1);



----

select status, count(*) n
from GameLine
where GameID in (
select id from Game where SeekerUserID=3037)
-- and status>0
group by status;



---



select Name as Seeker,SeekerUserID,status, count(*) n
from GameLine gl inner join Game g on g.id=gl.GameID
    inner join User u on u.id=g.SeekerUserID
where GameID in (
select id from Game where SeekerUserID between 3041 and 3051)
 and status>0
group by status,SeekerUserID,Name
order by Name,status;
