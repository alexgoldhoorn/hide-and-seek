
select * from ServerStart where id > 5000;
select * from GameListDetail where StartTime>='2012-08-30' and seeker <>'test';

select `g`.`id` AS `id`,`g`.`MapName` AS `MapName`,`g`.`MapWidth` AS `MapWidth`,`g`.`MapHeight` AS `MapHeight`,`g`.`MapNumObst` AS `MapNumObst`,`g`.`BaseRow` AS `BaseRow`,`g`.`BaseCol` AS `BaseCol`,`g`.`MaxActions` AS `MaxActions`,`g`.`StartTime` AS `StartTime`,`g`.`EndTime` AS `EndTime`, TIME_TO_SEC(TIMEDIFF(g.EndTime,g.StartTime)) AS 'Duration', `g`.`SeekerUserID` AS `SeekerUserID`,`g`.`HiderUserID` AS `HiderUserID`,`gl`.`WinStatus` AS `WinStatus`,`gl`.`NumActions` AS `NumActions`,`us`.`Name` AS `Seeker`,`uh`.`Name` AS `Hider` from (((`Game` `g` join `GameList` `gl` on((`gl`.`GameID` = `g`.`id`))) left join `User` `us` on((`us`.`id` = `g`.`SeekerUserID`))) left join `User` `uh` on((`uh`.`id` = `g`.`HiderUserID`)))

drop table WinListTmp;
create  table WinListTmp (
    Seeker varchar(50),
    WinStatus int,   
    AvgNumActions float,
    StdNumActions float,
    n int,
    Duration bigint(10)
);

truncate WinListTmp;
insert into WinListTmp 
select Seeker, WinStatus, avg(NumActions), std(NumActions), count(*) n, 1.0*Duration/Numactions as 
from GameListDetail where id>=7039 --  StartTime>='2012-09-05' -- '2012-08-31 16:30:00' -- '2012-08-30'
group by Seeker, WinStatus


select distinct w.Seeker, coalesce(w1.n,0) as SeekerWin, coalesce(w2.n,0) as HiderWin, coalesce(w3.n,0) as Tie, 
        coalesce(w1.n,0)+coalesce(w2.n,0)+coalesce(w3.n,0) as Total, w1.AvgNumActions as AvgNumActionsWin,
        w2.AvgNumActions as AvgNumActionsLoose,
from WinListTmp w
    left join WinListTmp w1 on w.Seeker=w1.Seeker and w1.WinStatus=1
    left join WinListTmp w2 on w.Seeker=w2.Seeker and w2.WinStatus=2
    left join WinListTmp w3 on w.Seeker=w3.Seeker and w3.WinStatus=3

select * from WinListTmp
where WinStatus=1
    

select *
from GameListDetail where StartTime>='2012-09-06' 
and not Seeker like '%_um_%'

'2012-08-31 16:30:00' and WinStatus=1

-- count(*) as n, sum(
select  * -- count(*),sum(SeekerAction>0)
from GameLine
where gameid in (
select id
from GameListDetail where StartTime>='2012-09-04'
and not Seeker like '%test' and Seeker like 'layerBase%')
and SeekerAction>0


select *
from User where name like 'layerRCd10a45_m%'

layerBase_map3_6x5_nr
layerK6_map3_6x5_sr_
layerK6_map3_6x5_nr_i
layerRC_map3_6x5_sr_i
layerRC_map3_6x5_nr_i
layerRCd10a45_map3_6x5_sr_i
layerRCd10a45_map3_6x5_nr_i



select * from GameListDetail where id=(select max(id) from Game);
select * from GameLine where GameId=(select max(id) from Game);
