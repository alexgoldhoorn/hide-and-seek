
--  To update varsPomcp table: --

truncate varsPomcp;

-- now run script: scripts/sepJoinCols.py


select distinct seeker from GameListDetail where seeker like 'u_pomcpp%' and starttime>'2013-09-01';


select MapSize,WinState,SeekerType2,HiderCat3,Reward, OnOffline,Segmentation,SegmentationX,count(*) n,
avg(DurationPerAction) DurationPerAction_avg, std(DurationPerAction) DurationPerAction_std,
avg(NumActions) NumActions_avg, std(NumActions) NumActions_std
from SimJuly2013
group by MapSize,WinState,SeekerType2,HiderCat3,Reward, OnOffline,Segmentation,SegmentationX;



select max(StartTime)
from GameListDetail


select distinct Seeker
from GameListDetail -- Filtered
where not seeker like '%test%' and not hider like '%test%' and
-- seeker='SmartSeeker' and (Hider='VerySmartHider' or Hider='AllKnowingVerySmartHider') and 
	WinState_id>0 and StartTime>'2013-07-07'


select MapSize,Seeker,WinState,HiderCat,Reward,count(*) n,
avg(DurationPerAction) DurationPerAction_avg, std(DurationPerAction) DurationPerAction_std,
avg(NumActions) NumActions_avg, std(NumActions) NumActions_std
from GameListDetail
where Seeker like '%pomcp%p4%'
group by MapSize,Seeker,WinState,HiderCat,Reward;



select Seeker,MapSize,WinState,HiderCat,Reward,DurationPerAction,NumActions
from GameListDetail
where Seeker like '%pomcp%p4%' and hidercat like '%smart%' and mapsize='20x20';


select hidercat,count(*)
from GameListDetail
where (Seeker like '%pomcpp4%' or Seeker like '%pomcpp5%') and hidercat like '%smart%' and winstate_id>0
group by hidercat



truncate varsPomcp
drop table varsPomcp;
create table varsPomcp (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	name varchar(255),
	version varchar(50),
	reward varchar(10),
	depth int,
	numSim int,
	numBelief int,
	explorationConst float,
	expandCount int,
	rewardAggregType varchar(10),
	hiderSimType varchar(10),
	hiderSimRandomProb float,
	discount float
)

SELECT * FROM hsgamelog.User where name like 'u_pomcppo%' and not like '%|%' escape '|';

select * from varsPomcp order by id desc;

