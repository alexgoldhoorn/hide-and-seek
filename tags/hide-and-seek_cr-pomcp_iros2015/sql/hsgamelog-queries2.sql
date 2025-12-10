
select * from ServerStart where id > 5000;
select * from GameListDetail where StartTime>='2012-08-30' and seeker <>'test';

select `g`.`id` AS `id`,`g`.`MapName` AS `MapName`,`g`.`MapWidth` AS `MapWidth`,
	`g`.`MapHeight` AS `MapHeight`,`g`.`MapNumObst` AS `MapNumObst`,`g`.`BaseRow` AS `BaseRow`,
	`g`.`BaseCol` AS `BaseCol`,`g`.`MaxActions` AS `MaxActions`,`g`.`StartTime` AS `StartTime`,
	`g`.`EndTime` AS `EndTime`, TIME_TO_SEC(TIMEDIFF(g.EndTime,g.StartTime)) AS 'Duration', 
	`g`.`SeekerUserID` AS `SeekerUserID`,`g`.`HiderUserID` AS `HiderUserID`,`gl`.`WinStatus` AS `WinStatus`,
	`gl`.`NumActions` AS `NumActions`,`us`.`Name` AS `Seeker`,`uh`.`Name` AS `Hider` 
from (((`Game` `g` join `GameList` `gl` on((`gl`.`GameID` = `g`.`id`))) 
	left join `User` `us` on((`us`.`id` = `g`.`SeekerUserID`))) 
	left join `User` `uh` on((`uh`.`id` = `g`.`HiderUserID`)))



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


select * from SimPOMCPSept2013p8 where mapsize='40x40'


select * from SimPOMCPSept2013p8 where mapsize='20x20'


-- use in group by delimiter $$

CREATE VIEW `SimPOMCPOct2013` AS 
select `gd`.`OnOffline` AS `OnOffline`,`gd`.`HiderCat` AS `HiderCat`,`gd`.`Hider` AS `Hider`,
`gd`.`MapName` AS `MapName`,`gd`.`MapSize` AS `MapSize`,`gd`.`WinState` AS `WinState`,`gd`.`MaxDepth` AS `MaxDepth`,
if((`gd`.`MapWidth` > `gd`.`MapHeight`),`gd`.`MapWidth`,`gd`.`MapHeight`) AS `MapDim`,
(`gd`.`MapWidth` * `gd`.`MapHeight`) AS `NumMapCells`,
((`gd`.`MapWidth` * `gd`.`MapHeight`) - `gd`.`MapNumObst`) AS `NumStates`,`gd`.`MapNumObst` AS `NumObst`,
`gd`.`MapWidth` AS `MapWidth`,`gd`.`MapHeight` AS `MapHeight`,`gd`.`DurationWithLoad` AS `DurationWithLoad`,
`gd`.`GameDuration` AS `GameDuration`,`gd`.`DurationPerAction` AS `DurationPerAction`,`gd`.`NumActions` AS `NumActions`,
if((`gd`.`WinState` like '%win%'),1,0) AS `IsWin`,if((`gd`.`WinState` like '%lose%'),1,0) AS `IsLose`,
if((`gd`.`WinState` like '%tie%'),1,0) AS `IsTie`,`gd`.`WinState_id` AS `WinState_id`,
if((`gd`.`HiderCat` = 'Random List'),'Random',`gd`.`HiderCat`) AS `HiderCat2`,
if((`gd`.`HiderCat` like '%Random%'),'random',if((`gd`.`HiderCat` = 'All Knowing Very Smart'),'smart ff',
	if((`gd`.`HiderCat` = 'Very Smart'),'smart','??'))) AS `HiderCat3`,
if((`gd`.`Seeker` = 'SmartSeeker'),'none',`gd`.`Reward`) AS `Reward2`,
if(((`gd`.`OnOffline` = 'offline') or (`gd`.`Seeker` = 'SmartSeeker') or ((`gd`.`Seeker` like '%300s%') = '300s')),'no','yes') AS `HasTimeLimit`,
if((`gd`.`OnOffline` = 'offline'),'off-line',if((`gd`.`Seeker` like '%sim4%'),'smart seeker',if((`gd`.`Seeker` like '%sim3%'),'on-line (top rew.)',if((`gd`.`Seeker` like '%sim2%'),'on-line',if((`gd`.`Seeker` = 'SmartSeeker'),'smart seeker','other'))))) AS `SeekerType2`,
`gd`.`Seeker` AS `Seeker`,`p`.`name` AS `name`,`p`.`version` AS `version`,
if((`p`.`reward` = 't'),'triangle',if((`p`.`reward` = 'f'),'final',if((`p`.`reward` = 'c'),'final cross','unknown'))) AS `Reward`,
`p`.`depth` AS `depth`,`p`.`numSim` AS `numSim`,`p`.`numBelief` AS `numBelief`,
`p`.`explorationConst` AS `explorationConst`,`p`.`expandCount` AS `expandCount`, 
if(p.rewardAggregType='s','sum',if(p.rewardAggregType='n','normSimPOMCPOct2013','unknown')) as rewardAggregType
from (`GameListDetail` `gd` left join `varsPomcp` `p` on((`gd`.`Seeker` = `p`.`name`))) where ((not((`gd`.`Seeker` like '%test%'))) and (`gd`.`Seeker` <> 'autohsUser') and (not((`gd`.`Hider` like '%test%'))) and (`gd`.`Hider` <> 'autohsUser') and (`gd`.`WinState_id` > 0) and (`gd`.`StartTime` > '2013-09-01'))


version,reward,depth,numSim,numBelief,explorationCost,expandCount


select *
from  SimPOMCPOct2013 where rewardAggregtype='normalized'
varsPomcp

create view SimPOMCPOct2013p9 as
select * from SimPOMCPSept2013 where version='p9'

select max(StartTime)
from GameListDetail





create view SimPOMCPOct2013p10 as
select * from SimPOMCPOct2013 where version='p9' and rewardAggregType<>'unknown' or version='p10'







-- list 
select Seeker,Hider,MapName,WinState,count(*),avg(numactions) as AvgNumActions,avg(DurationPerAction) AvgDurPerAct
from GameListDetail
where (seeker like 'SmartSeeker_shtvn%' or seeker like 'SmartSeeker_Rand%') -- hider='VerySmartHider2' or seeker='SmartSeeker_rh1'
-- and mapname like 'map%_40x40.txt'
group by Seeker,Hider, MapName,WinState
order by Seeker,Hider,MapName,WinState

-- table of win %

select 
select Seeker,Hider,MapName,WinState,count(*)
from GameListDetail
group by Seeker,Hider, MapName,WinState

order by Seeker,Hider,MapName,WinState





-- list count per map,players,winstate
-- drop view GameListWinStateCounter 
create view GameListWinStateCounter as 
select gd.MapName, SeekerUserID,HiderUserID,WinStatus,count(*) as n,avg(GameDuration) as AvgGameDuration,
1.0*avg(GameDuration)/count(*) as AvgGameDurationPerStep, avg(NumActions) as AvgNumActions
from Game gd
inner join GameList gl on gd.id=gl.GameID
group by gd.MapName, SeekerUserID,HiderUserID,WinStatus


create view GameListMapAndPlayers as
select distinct MapName, SeekerUserID, HiderUserID
	from Game


-- list count of win,lose,tie per players and map
-- drop view GameListWinCounter
create view GameListWinCounter as 
select g.MapName, g.SeekerUserID,g.HiderUserID,coalesce(gcw.n,0) as NumWin, coalesce(gcl.n,0) as NumLose, 
	coalesce(gct.n,0) as NumTie, gcw.AvgGameDurationPerStep as AvgGameDurationPerStepWin, 
	gcl.AvgGameDurationPerStep as AvgGameDurationPerStepLose, gct.AvgGameDurationPerStep as AvgGameDurationPerStepTie,
	gcw.AvgNumActions as AvgNumActionsWin, gcl.AvgNumActions as AvgNumActionsLose, gct.AvgNumActions as AvgNumActionsTie
from GameListMapAndPlayers as g 
left join GameListWinStateCounter as gcw on (g.MapName=gcw.MapName and g.SeekerUserID=gcw.SeekerUserID
	and g.HiderUserID=gcw.HiderUserID and gcw.WinStatus=1)
left join GameListWinStateCounter as gcl on (g.MapName=gcl.MapName and g.SeekerUserID=gcl.SeekerUserID
	and g.HiderUserID=gcl.HiderUserID and gcl.WinStatus=2)
left join GameListWinStateCounter as gct on (g.MapName=gct.MapName and g.SeekerUserID=gct.SeekerUserID
	and g.HiderUserID=gct.HiderUserID and gct.WinStatus=3)



-- list win % with player names
create view GameListWinPerc as
select gl.*,us.Name as Seeker,uh.Name as Hider,(NumWin+NumLose+NumTie) as NumTotal,
	100.0*NumWin/(NumWin+NumLose+NumTie) as WinPerc, 100.0*NumLose/(NumWin+NumLose+NumTie) as LosePerc,
	100.0*NumTie/(NumWin+NumLose+NumTie) as TiePerc
from GameListWinCounter gl 
left join User as us on gl.SeekerUserID=us.id
left join User as uh on gl.HiderUserID=uh.id


select * from GameLine
where gameid in (
select id from GameListDetail where seeker='SmartSeeker_shtt1i'  and winstate_id=1
) and status=1


select Seeker,Hider,MapName,WinPerc,LosePerc,TiePerc,NumWin,NumLose,NumTie,NumTotal,AvgNumActionsWin,AvgNumActionsLose,AvgNumActionsTie
from GameListWinPerc
where (seeker like 'SmartSeeker_shtv2_%') -- or seeker like 'SmartSeeker_Rand%') 
order by Seeker,Hider,MapName

SmartSeeker_shtv2_1


select Seeker,Hider,MapName,WinPerc,LosePerc,TiePerc,NumWin,NumLose,NumTie,NumTotal,AvgNumActionsWin,AvgNumActionsLose,AvgNumActionsTie
from GameListWinPerc
where (seeker like 'u_pomcppn1%') 
order by Seeker,Hider,MapName


select Hider,MapName,MapSize,WinState,MaxDepth,NumMapCells,NumObst,DurationWithLoad,GameDuration,DurationPerAction,
NumActions,IsWin,IsLose,IsTie,WinState_id,
Seeker, version,Reward,depth,numSim,numBelief,explorationConst,expandCount,rewardAggregType,
HiderSimType,hiderSimRandomProb
from SimPOMCPOct2013 
where version = 'pn2'
into outfile '/tmp/anly_pomcp3.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';



select Seeker,Hider,MapName,WinPerc,LosePerc,TiePerc,NumWin,NumLose,NumTie,NumTotal,AvgNumActionsWin,AvgNumActionsLose,AvgNumActionsTie,
AvgGameDurationPerStepWin,AvgGameDurationPerStepLose,AvgGameDurationPerStepTie,
p.version,p.reward,p.depth,p.numSim,p.numBelief,p.explorationConst,p.expandCount,p.rewardAggregType,p.hiderSimType,p.hiderSimRandomProb,
p.discount
from GameListWinPerc as gl inner join varsPomcp as p on gl.seeker=p.name
where version in  ('po1','po2','po3','po7','po5') -- (seeker like 'u_pomcppo%') 
order by version,Seeker,Hider,MapName


select * from User order by id desc where length(Name)>50
select * from User where Name like 


select *
from Game
order by StartTime desc

select * from
(
select u1.Name,count(*) as n
from User u1 left join User u2 on u1.Name=u2.Name and u1.id<>u2.id
where u1.Name like 'u_pomcppo%'
group by u1.Name,u2.Name
) as q
where n>1


select numSim,avg(tm) as avg,std(tm) as std, count(*) as n from 
(select numSim,timediff(endtime,starttime) as tm from GameListDetail gd 
left join varsPomcp p on gd.seeker=p.name  where version like 'po_' and numSim>10000) as q group by numSim;


select version,MapName,Hider,max(StartTime)
from SimPOMCPOct2013
where version in  ('po1','po2','po3','po7','po5') and endtime is null
group by version,MapName,Hider





select Hider,Reward,MapName,WinState,count(*) as n, 
	avg(DurationPerAction) as avgDurationPerAction, std(DurationPerAction) as stdDurationPerAction,
	avg(NumActions) as avgNumActions, std(NumActions) as stdNumActions
from SimPOMCPOct2013
where version in  ('po1','po2','po3','po7','po5') 
group by Hider,reward,WinState,MapName


select max(DurationPerAction) as maxDurationPerAction, min(DurationPerAction) as minDurationPerAction,
	avg(DurationPerAction) as avgDurationPerAction, std(DurationPerAction) as stdDurationPerAction,
	avg(NumActions) as avgNumActions, std(NumActions) as stdNumActions
from SimPOMCPOct2013
where version in  ('po1','po2','po3','po7','po5') 


group by Hider,reward,WinState,MapName



select *
from SimPOMCPOct2013
where version in  ('po1','po2','po3','po7','po5') and winstate_id=2 and reward='triangle'

