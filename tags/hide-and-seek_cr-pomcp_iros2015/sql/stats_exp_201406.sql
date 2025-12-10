
-- data per game
select g.*, gl.*
from GameExt g left join GameLineStats gl on g.id=gl.GameID
where SeekerUserID in (9,10,13)
	and NumActions=StopAfterNumSteps;

-- grouped data using already grouped data per line
select SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd, 
	count(*) as n, avg(avgDist) as avgDist, avg(avgDistEuc) as avgDistEuc,
	avg(avgSeekerBeliefScore) as avgSeekerBeliefScore,
	avg(avgSeekerDuration_ms) as avgSeekerDuration_ms,
	avg(DurationS) as avgDurationS
from GameExt g left join GameLineStats gl on g.id=gl.GameID
where SeekerUserID in (9,10,13)
	and NumActions=StopAfterNumSteps
group by SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd
order by SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd


CREATE TABLE MapStepSize (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	MapName varchar(255),
	Name varchar(64),
	CellSizeM float(14,6)
);


insert into MapStepSize (MapName,Name,CellSizeM) values ('bcn_lab/fme2014_map4.txt','FME',1);
insert into MapStepSize (MapName,Name,CellSizeM) values ('brl/brl29a.txt','BRL',1);
insert into MapStepSize (MapName,Name,CellSizeM) values ('brl/master29e.txt','Telecos',0.8);
insert into MapStepSize (MapName,Name,CellSizeM) values ('brl/master29f.txt','Master',0.4);

select * from MapStepSize

select distinct MapName
from Game



-- grouped data using already grouped data per line
select SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd, 
	count(*) as n, avg(avgDist) as avgDist, avg(avgDistEuc) as avgDistEuc,
	avg(avgSeekerBeliefScore) as avgSeekerBeliefScore,
	avg(avgSeekerDuration_ms) as avgSeekerDuration_ms,
	avg(DurationS) as avgDurationS
from GameExt g left join GameLineStats gl on g.id=gl.GameID
where SeekerUserID in (46) -- 9,10,13)
	and NumActions=StopAfterNumSteps
group by SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd
order by SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd

select SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd, StopAfterNumSteps,
	count(distinct g.id) as nGames,
	max(NumActions) AS avgNumActions,
	avg(d_sh) AS avgDist,
	std(d_sh) AS stdDist,
	avg(d_shEuc) AS avgDistEuc,
	std(d_shEuc) AS stdDistEuc,
	avg(SeekerBeliefScore) AS avgSeekerBeliefScore,
	std(SeekerBeliefScore) AS stdSeekerBeliefScore,
	avg(SeekerDuration_ms) AS avgSeekerDuration_ms,
	std(SeekerDuration_ms) AS stdSeekerDuration_ms,
	avg(HiderDuration_ms) AS avgHiderDuration_ms,
	std(HiderDuration_ms) AS stdHiderDuration_ms
from GameExt g  left join GameLineExt gl on g.id=gl.GameID
	 left join GameLineStats gls on g.id=gls.GameID
where SeekerUserID in (46) -- 9,10,13)
	 and NumActions=StopAfterNumSteps	
group by SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd
order by SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd
into outfile '/tmp/res140829.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'


-- >> QUERY USED << --		REMEMBER TO SET SEEKERUSERID IN BOTH!!!! SUBQUERIES
select td.*,avgNumActions, stdNumActions,avgFirstStepClose,stdFirstStepClose,avgFirstStepVisibile,stdFirstStepVisibile
from (	
		select MapName, AutoWalkerN, SimObsNoiseStd,SolverTypeName,SolverType,
			count(distinct g.id) as nGames,
			-- max(NumActions) AS avgNumActions,
			avg(d_sh) AS avgDist,
			std(d_sh) AS stdDist,
			avg(d_shEuc) AS avgDistEuc,
			std(d_shEuc) AS stdDistEuc,
			avg(SeekerBeliefScore) AS avgSeekerBeliefScore,
			std(SeekerBeliefScore) AS stdSeekerBeliefScore,
			avg(SeekerReward) AS avgSeekerReward,
			std(SeekerReward) AS stdSeekerReward,
			avg(SeekerDuration_ms) AS avgSeekerDuration_ms,
			std(SeekerDuration_ms) AS stdSeekerDuration_ms,
			avg(HiderDuration_ms) AS avgHiderDuration_ms,
			std(HiderDuration_ms) AS stdHiderDuration_ms,
			count(*) as nLines,
			sum(IsHidden) nHidden,
			100*sum(IsHidden)/count(*) PercHidden,
			100*sum(IsClose)/count(*) PercIsClose, -- within 1.2
			100*sum(IsClose2)/count(*) PercIsClose2, -- within 1.2
			100*sum(IsClose5)/count(*) PercIsClose5 -- within 1.2
			-- avg(FirstStepClose, -- first action to be close (1.2)
			-- avg(NumActions) as avgNumActions
		from GameExt g  left join GameLineExt gl on g.id=gl.GameID
			-- left join GameLineStats gls on g.id=gls.GameID
			-- left join GameFirstStepClose f on g.id=f.GameID
		where SeekerUserID in (46) -- (9,10,13)
			-- and NumActions=StopAfterNumSteps
			-- and (
			/*(ActionNum<=20)
			OR 
				(ActionNum<=50 AND MapHeight>20)
			)*/
		group by MapName, AutoWalkerN, SimObsNoiseStd,SolverTypeName,SolverType
	) as td left join (
		select MapName, AutoWalkerN, SimObsNoiseStd,SolverType, 
			avg(NumActions) as avgNumActions, std(NumActions) as stdNumActions
		from GameLineStats gs inner join Game g on gs.GameID=g.id
		where SeekerUserID in (46) 
		group by MapName, AutoWalkerN, SimObsNoiseStd,SolverType
	) sd on td.MapName=sd.MapName and td.AutoWalkerN=sd.AutoWalkerN and td.SimObsNoiseStd=sd.SimObsNoiseStd
		and td.SolverType=sd.SolverType
	left join (
		select MapName, AutoWalkerN, SimObsNoiseStd,SolverType, 
			avg(FirstStepClose) as avgFirstStepClose, std(FirstStepClose) stdFirstStepClose
		from GameFirstStepClose gf inner join Game g on gf.GameID=g.id
		where SeekerUserID in (46) 
		group by MapName, AutoWalkerN, SimObsNoiseStd,SolverType
	) fd on td.MapName=fd.MapName and td.AutoWalkerN=fd.AutoWalkerN and td.SimObsNoiseStd=fd.SimObsNoiseStd
		and td.SolverType=fd.SolverType
	left join (
		select MapName, AutoWalkerN, SimObsNoiseStd,SolverType, 
			avg(FirstStepVisibile) as avgFirstStepVisibile, std(FirstStepVisibile) stdFirstStepVisibile
		from GameFirstStepVisibile gf inner join Game g on gf.GameID=g.id
		where SeekerUserID in (46) 
		group by MapName, AutoWalkerN, SimObsNoiseStd,SolverType
	) vd on td.MapName=vd.MapName and td.AutoWalkerN=vd.AutoWalkerN and td.SimObsNoiseStd=vd.SimObsNoiseStd
		and td.SolverType=vd.SolverType

order by td.MapName, td.AutoWalkerN, td.SimObsNoiseStd,td.SolverTypeName,avgDist
into outfile '/tmp/resfil140626_1050.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';
-- <<


-- to get all step data:
select MapName, AutoWalkerN, SimObsNoiseStd,SolverTypeName,g.id as GameID, gl.id as GameLineID,ActionNum,
	d_sh AS Dist,
	d_shEuc AS DistEuc,
	SeekerBeliefScore,
	SeekerReward,
	SeekerDuration_ms,
	HiderDuration_ms,
	IsHidden, IsClose, IsClose2, IsClose5 
	-- FirstStepVisibile
from GameExt g  left join GameLineExt gl on g.id=gl.GameID
	 -- left join GameLineStats gls on g.id=gls.GameID
where SeekerUserID in (44)
	-- and NumActions=StopAfterNumSteps
	/*and (
		(ActionNum<=20)
	OR 
		(ActionNum<=50 AND MapHeight>20)*/
 and ActionNum>0
order by MapName, AutoWalkerN, SimObsNoiseStd,SolverTypeName, g.id, ActionNum
into outfile '/tmp/resall140618_1122.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';



select MapName, AutoWalkerN, SimObsNoiseStd,SolverType, 
	avg(NumActions) as avgNumActions, std(NumActions) as stdNumActions,
	avg(FirstStepClose) as avgFirstStepClose, std(FirstStepClose) stdFirstStepClose
	,count(distinct g.id) nGames
from GameLineStats gs inner join Game g on gs.GameID=g.id
group by MapName, AutoWalkerN, SimObsNoiseStd,SolverType

-- alter table GameLine add SeekerReward double AFTER SeekerBeliefScore;


select distinct SeekerUserID,SolverTypeName,StopAfterNumSteps
from GameExt g  

-- check how many complete runs
select MapName, HiderComments,count(*)
from GameExt
where SeekerUserID in (9,10,13)
group by MapName, HiderComments



select *
from GameExt
order by id desc


select count(*) from GameLineExt
where HiderRowContWNoise=-1


-- check if same hider actions
select g1.ActionNum,g1.HiderRowCont,g1.HiderColCont,
	g2.HiderRowCont,g2.HiderColCont 
from GameLine g1 left join GameLine g2 on g1.ActionNum=g2.ActionNum 
where g2.gameid=9931 and g1.gameid=9934
	and g1.ActionNum>0
	and (g1.HiderRowCont<>g2.HiderRowCont OR g1.HiderColCont<>g2.HiderColCont)

-- list of current running experiments with details
select g.*,gs.*,fs.FirstStepClose, le.IsHidden as IsHiddenAtStart
from GameExt g left join GameLineStats gs on g.id=gs.GameID
	left join GameFirstStepClose fs on g.id=fs.GameID
	left join
		(Select GameID,max(IsHidden) IsHidden
		from GameLineExt where ActionNum=0 group by GameID) le on g.id=le.GameID 
where seekeruserId=44


-- expected time left
select n, totalTimeDone_h, avgTimePerGame_s, avgTimePerGame_s*120/3600 as estTimePerRound_h,
	(120*100-n)*avgTimePerGame_s/3600 as estTimeLeftFor100Rounds_h,
	(120*30-n)*avgTimePerGame_s/3600 as estTimeLeftFor20Rounds_h
from (
select count(*) as n, sum(DurationS)/3600 as totalTimeDone_h, avg(DurationS) avgTimePerGame_s
from GameExt
where seekeruserId=44
) t1




-- grouped data using already grouped data per line and map name and sizes in m
select SolverTypeName, t.MapName,m.Name, AutoWalkerN, SimObsNoiseStd, 
	avgDist*CellSizeM as avgDistM, avgDistEuc*CellSizeM as  avgDistEucM,
	avgSeekerBeliefScore*CellSizeM as avgSeekerBeliefScoreM,
	avgSeekerDuration_ms,avgDurationS
(
	select SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd, 
		count(*) as n, avg(avgDist) as avgDist, avg(avgDistEuc) as avgDistEuc,
		avg(avgSeekerBeliefScore) as avgSeekerBeliefScore,
		avg(avgSeekerDuration_ms) as avgSeekerDuration_ms,
		avg(DurationS) as avgDurationS
	from GameExt g left join GameLineStats gl on g.id=gl.GameID	
	where SeekerUserID in (46) -- 9,10,13)
		and NumActions=StopAfterNumSteps
	group by SolverTypeName, MapName, AutoWalkerN, SimObsNoiseStd
) t
left join MapStepSize m on t.MapName=m.MapName
order by SolverTypeName, m.Name, AutoWalkerN, SimObsNoiseStd





select t.MapName, m.Name, AutoWalkerN, SimObsNoiseStd,SolverTypeName,SolverType,
avgDist*CellSizeM as avgDistM, stdDist*CellSizeM as stdDistM,
avgDistEuc*CellSizeM as avgDistEucM, stdDistEuc*CellSizeM as stdDistEucM,
avgSeekerBeliefScore*CellSizeM as avgSeekerBeliefScoreM,stdSeekerBeliefScore*CellSizeM as stdSeekerBeliefScoreM,
avgSeekerReward,stdSeekerReward,avgSeekerDuration_ms,stdSeekerDuration_ms,avgHiderDuration_ms,
nLines,nHidden,PercHidden,PercIsClose,PercIsClose2,PercIsClose5
from (
		select MapName, AutoWalkerN, SimObsNoiseStd,SolverTypeName,SolverType,
			count(distinct g.id) as nGames,
			-- max(NumActions) AS avgNumActions,
			avg(d_sh) AS avgDist,
			std(d_sh) AS stdDist,
			avg(d_shEuc) AS avgDistEuc,
			std(d_shEuc) AS stdDistEuc,
			avg(SeekerBeliefScore) AS avgSeekerBeliefScore,
			std(SeekerBeliefScore) AS stdSeekerBeliefScore,
			avg(SeekerReward) AS avgSeekerReward,
			std(SeekerReward) AS stdSeekerReward,
			avg(SeekerDuration_ms) AS avgSeekerDuration_ms,
			std(SeekerDuration_ms) AS stdSeekerDuration_ms,
			avg(HiderDuration_ms) AS avgHiderDuration_ms,
			std(HiderDuration_ms) AS stdHiderDuration_ms,
			count(*) as nLines,
			sum(IsHidden) nHidden,
			100*sum(IsHidden)/count(*) PercHidden,
			100*sum(IsClose)/count(*) PercIsClose, -- within 1.2
			100*sum(IsClose2)/count(*) PercIsClose2, -- within 1.2
			100*sum(IsClose5)/count(*) PercIsClose5 -- within 1.2
			-- avg(FirstStepClose, -- first action to be close (1.2)
			-- avg(NumActions) as avgNumActions
		from GameExt g  left join GameLineExt gl on g.id=gl.GameID
			-- left join GameLineStats gls on g.id=gls.GameID
			-- left join GameFirstStepClose f on g.id=f.GameID
		where SeekerUserID in (44) -- (9,10,13)
			-- and NumActions=StopAfterNumSteps
			-- and (
			/*(ActionNum<=20)
			OR 
				(ActionNum<=50 AND MapHeight>20)
			)*/
		group by MapName, AutoWalkerN, SimObsNoiseStd,SolverTypeName,SolverType
) t left join MapStepSize m on t.MapName=m.MapName
into outfile '/tmp/res140901_1.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';




d_shEuc
SeekerBeliefScore
SeekerReward
SeekerDuration_ms
HiderDuration_ms

select MapName, AutoWalkerN, SimObsNoiseStd,SolverTypeName,m.Name as Map,
	d_sh		
		from GameExt g  left join GameLineExt gl on g.id=gl.GameID
			left join MapStepSize m on g.MapName=m.MapName
		where SeekerUserID in (44)

