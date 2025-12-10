
# queries to make graphs for H&S analysis #

delimiter $$




$$

# Table 1 / Fig 1a: # games won, online/off-, reward, smart/random, win%
# OnOffline,Hider,Reward,Win State,n
select OnOffline,HiderCat,Reward,WinState,count(*) n
from GameListDetailFiltered
group by OnOffline,HiderCat,Reward,WinState
into outfile '/tmp/anly_games_won.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Table 1 / Fig 1a: # games won, online/off-, reward, smart/random, win%
# OnOffline,MaxDepthFilter,Hider,Reward,Win State,n
select OnOffLine,MaxDepthFilter,HiderCat,Reward,WinState,count(*) n
from GameListDetailFiltered
group by OnOffLine , MaxDepthFilter,HiderCat,Reward,WinState
into outfile '/tmp/anly_games_won2.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Table 1 / Fig 1a: # games won, online/off-, reward, smart/random, win%
# OnOffline,MaxDepthFilter,Hider,Reward,Segmentation,Segmentation X,Win State,n
select OnOffLine,MaxDepthFilter,HiderCat,Reward,Segmentation,SegmentationX,WinState,count(*) n
from GameListDetailFiltered
group by OnOffLine , MaxDepthFilter,HiderCat,Reward,WinState,Segmentation,SegmentationX
into outfile '/tmp/anly_games_won3.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';



$$

# Table 1b: # games won, online/off-, reward, smart/random, win%
# Max Depth,Hider,Reward,Win State,n
select MaxDepthFilter,HiderCat,Reward,WinState,count(*) n
from GameListDetailFiltered
where OnOffline='online'
group by MaxDepthFilter,HiderCat,Reward,WinState
into outfile '/tmp/anly_gameswon_maxdfil_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 1b: count of game sims per size,reward,on/offline (,Hider)
# Hider,Map Size,Reward,OnOffline,Max Depth Filter,n
select HiderCat, MapSize, Reward, OnOffline, MaxDepthFilter, count(*) n
from GameListDetailFiltered
group by HiderCat, MapSize, Reward, OnOffline, MaxDepthFilter
into outfile '/tmp/anly_game_count.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'

$$

# ag130219
# OnOffline,Hider,Map Size,Win State,Duration (s),Duration per Action (s),Number of Actions
select OnOffline,HiderCat, MapSize, WinState, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where MaxDepthFilter='no'
into outfile '/tmp/anly_numact_dur.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';
$$

# ag130219
# OnOffline,Hider,Map Size,Win State,Reward,Segmentation,Segmentation X,Duration (s),Duration per Action (s),Number of Actions
select OnOffline,HiderCat, MapSize, WinState,Reward,Segmentation,SegmentationX, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where MaxDepthFilter='no'
into outfile '/tmp/anly_numact_dur_nomaxd.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 2a: game durations of offline
# Fig 3b/4: game # actions of offline
# Hider,Map Size,Win State,Duration (s),Duration per Action (s),Number of Actions
select HiderCat, MapSize, WinState, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where OnOffline='offline'
into outfile '/tmp/anly_game_dur_off.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 2b: game durations of online
# Fig 3b/4: game # actions of online
# Fig 6b: duration per action
# Max Depth,Hider,Map Size,Win State,Duration (s),Duration per Action (s),Number of Actions
select MaxDepthFilter, HiderCat, MapSize, WinState, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where OnOffline='online'
into outfile '/tmp/anly_game_dur_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Table 2: avg time per action (online) (median not in mysql, if needed get whole list and use other prog)
# Map Size,Duration Per Action Avg,Duration Per Action Std
select MapSize,avg(DurationPerAction),std(DurationPerAction)
from GameListDetailFiltered
where OnOffline='online' and MaxDepthFilter='no'
group by MapSize
into outfile '/tmp/anly_timeperaction_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'

$$

# Fig 5a: win % online
# Fig 7b: # actions avg online
# Hider,Reward,Segmentation,Win State,n,Number of Actions avg,Number of Actions std
select HiderCat,Reward,Segmentation,WinState,count(*) n, avg(NumActions), std(NumActions)
from GameListDetailFiltered
where OnOffline='online' and MaxDepthFilter='no'
group by HiderCat,Reward,WinState,Segmentation
into outfile '/tmp/anly_numact_win_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 5b: win % offline
# Fig 7a: avg num actions
select HiderCat,WinState,count(*) n, avg(NumActions), std(NumActions)
from GameListDetailFiltered
where OnOffline='offline' and MaxDepthFilter='no'
group by OnOffline,HiderCat,WinState
into outfile '/tmp/anly_numact_win_off.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 5c: win % online for max depth
# (5d: without seperation for reward,segmentation,..)
# Fig 7b: avg num actions online
# Hider,Reward,Segmentation,Max Depth,Win State,n,Number of Actions avg,Number of Actions std,Duration (s) avg,Duration (s) std,Duration per Action (s) avg,Duration per Action (s) std
select HiderCatShort,Reward,SegmentationShort,MaxDepth,WinState,count(*) n, avg(NumActions), std(NumActions),
	avg(Duration),std(Duration),avg(DurationPerAction),std(DurationPerAction)
from GameListDetailFiltered
where OnOffline='online'  and SegmentationX='none'
group by HiderCat,Reward,WinState,Segmentation,MaxDepth
into outfile '/tmp/anly_numact_win_maxd_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Hider,MapSize,Reward,Segmentation,Max Depth,Win State,Number of Actions,Duration (s),Duration per Action (s)
select HiderCatShort,MapSize,Reward,SegmentationShort,MaxDepth,WinState, NumActions, 
	Duration,DurationPerAction
from GameListDetailFiltered
where OnOffline='online' and SegmentationX='none'
-- group by HiderCat,Reward,WinState,Segmentation,MaxDepth
into outfile '/tmp/anly_numact_win_maxd_on_all.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

#ag130221: not time limited games
select TimeLimit,WinState,count(*) N from (
	select WinState,
		if(Seeker like '%_rh' or Seeker like '%sh_T','No TL','TL') as TimeLimit
		-- count(*) n
	from GameListDetailFiltered
	where /*Seeker like '%_rh' or Seeker like '%sh_T'
	and*/ onOffline='online' and MaxDepthFilter='no' and SegmentationX='none' and MapSize='10x10'
	-- group by HiderCatShort,MapSize,Reward,SegmentationShort,WinState
) t group by TimeLimit,WinState



/*select HiderCatShort,MapSize,Reward,SegmentationShort,WinState,
		if(Seeker like '%_rh' or Seeker like '%sh_T','No TL','TL') as TimeLimit,
		count(*) n
	from GameListDetailFiltered
	where onOffline='online' and MaxDepthFilter='no' and SegmentationX='none' and MapSize='10x10'
	group by HiderCatShort,MapSize,Reward,SegmentationShort,WinState
*/
$$

#ag130222: limit time of game
# Hider,MapSize,Reward,Segmentation,Win State,TL,Number of Actions,Duration (s),Duration per Action (s)
select HiderCatShort,MapSize,Reward,SegmentationShort,WinState,if(Seeker like '%_rh' or Seeker like '%sh_T','No TL','TL') as TimeLimit,
	NumActions,Duration,DurationPerAction
from GameListDetailFiltered
where /*Seeker like '%_rh' or Seeker like '%sh_T'
and*/ onOffline='online' and MaxDepthFilter='no' and SegmentationX='none' and MapSize='10x10'
into outfile '/tmp/anly_numact_win_tl_notl.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';



$$

create view SarsopLogTrialPerLineView as
select p.*,
        l.`Info` AS `Info`,
        if((l.`Info` = 'online_T300_grx'),
            'online_T300_grx',
            if((l.`Info` like 'online_T300%'),
                'online_T300',
                l.`Info`)) AS `Info2`,
        l.nX, l.nY, l.nO, l.nA,
        l.initBoundsT,        
        l.initTotalT,        
        l.InitPrecis,
        l.Duration,
        l.lbBackT,
        l.ubBackT,
        l.sampleT,
        l.pruneT,
        l.nIt,        
        l.elapsedT,
        l.nTrials,
        l.lowerBound,
        l.upperBound,
        l.precis,
        l.nAlphas,
        l.nBeliefs,        
        l.nBackups,
        l.nLbBack,
        l.nUbBack,
        ((((l.initTotalT + l.lbBackT) + l.ubBackT) + l.sampleT) + l.pruneT) AS subTotalT
from SarsopLogTrialPerLine l left join SarsopLogFileParams p on l.FileName=p.FileName;

$$

create view SarsopLogTrialPerLineViewPerc as
select *, initTotalT/subTotalT initTotalTPerc, lbBackT/subTotalT lbBackTPerc, ubBackT/subTotalT ubBackTPerc, 
	sampleT/subTotalT sampleTPerc, pruneT/subTotalT pruneTPerc
from SarsopLogTrialPerLineView


$$

# queries with: 
# 1. total sarsop duration + params + ubounds/lbounds
# 2. relative time % of SARSOP
# mapSize,reward,segmentX,segmentY,maxDepthFilter,Info,elapsedT,lowerBound,upperBound,precis,Converged,subTotalT,initTotalTPerc,lbBackTPerc,ubBackTPerc,sampleTPerc,pruneTPerc,nTrials,nBackups,nBeliefs,nAlphas
select mapSize,reward,segmentX,segmentY,maxDepthFilter,Info2,elapsedT,lowerBound,upperBound,precis,
	if(precis<1e-3,'yes','no') as Converged,subTotalT,initTotalTPerc,lbBackTPerc,ubBackTPerc,sampleTPerc,pruneTPerc,
	nTrials,nBackups,nBeliefs,nAlphas
from SarsopLogTrialPerLineViewPerc
into outfile '/tmp/anly_sarsop.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';




$$

-- segmentX,segmentY
select mapSize,reward,Info2,
	avg(nTrials) as trials_avg,std(nTrials) as trials_std,
	avg(nBackups) nBackups_avg,std(nBackups) nBackups_std, 
	avg(nBeliefs) nBeliefs_avg,std(nBeliefs) nBeliefs_std,
	avg(nAlphas) nAlphas_avg, std(nAlphas) nAlphas_std
from SarsopLogTrialPerLineViewPerc
where maxDepthFilter='no'
group by mapSize,reward,Info2

/*
select Info2,MapSize,Reward,SegmentX,SegmentY, initTotalT/subTotalT as initTotalTperc, lbBackT/subTotalT as lbBackTperc,
ubBackT/subTotalT as ubBackTperc, sampleT/subTotalT as sampleTperc, pruneT/subTotalT as pruneTperc,n,
	elepsedT/n as elepsedT, elapsedSubTtotalDiffT/n as elapsedSubTtotalDiffT,
	lowerBound/n as lowerBound, upperBound/n as upperBound
from (
	select Info2,MapSize,Reward,SegmentX,SegmentY, sum(initTotalT) initTotalT, sum(lbBackT) lbBackT, sum(ubBackT) ubBackT, sum(sampleT) sampleT, sum(pruneT) pruneT, 
		sum(subTotalT) subTotalT, sum(nLines) as n, 
		sum(elepsedT) elepsedT, sum(elapsedSubTtotalDiffT) elapsedSubTtotalDiffT,
		sum(lowerBound) lowerBound, sum(upperBound) upperBound
	from (
		select  MapSize,Info2,Reward,SegmentX,SegmentY,
		nLines*initTotalT_avg initTotalT,
		lbBackT_avg*nLines as lbBackT,
		ubBackT_avg*nLines as ubBackT,
		sampleT_avg*nLines as sampleT,
		pruneT_avg*nLines as pruneT,
		subTotalT*nLines as subTotalT,
		elepsedT_avg*nLines as elepsedT,
		elapsedSubTtotalDiff*nLines as elapsedSubTtotalDiffT,
		lowerBound_avg*nLines as lowerBound,
		upperBound_avg*nLines as upperBound,
		nLines
		from SarsopLogSummView

	) t
	group by MapSize,Info2,Reward,SegmentX,SegmentY -- Info2
) t2



$$*/

delimiter ;

-- ------------------  TEST ----------------- --
/*

concat(cast(MapWidth as char(2)),'x',cast(MapHeight as char(2))) as MapSize
TODO: time difference!!! but not working
select EndTime-StartTime,StartTime,EndTime,timestampdiff(second,StartTime,EndTime)
from GameListDetail




select concat(convert(3,  char(8) ),'x','yyy')

drop view GameListDetailFiltered

delimiter $$

$$

CREATE VIEW GameListDetailFiltered AS
select *
from GameListDetail
where not HiderCat='unknown'
	and not Seeker like '%test%' and not Hider like '%test%'
	and StartTime>=(select min(StartTime) from SarsopLogTrial where StartTime>0) 
$$
delimiter ;


select MapName,MaxActions,Seeker,WinStatus as WinStatusID,avg(Duration) as Duration_avg,std(Duration) as Duration_std,
	avg(NumActions) as NumActions_avg,std(NumActions) as NumActions_std, count(*) n,
	if (Seeker like '%off%','offline','online') as OnOffline,
	if (Seeker like '%newrew%', 'newrew', 
		if (Seeker like '%sr_fsc%','sr_fsc',
			if (Seeker like '%sr_fs%', 'sr_fs','unknown')
		)
	) as Reward,
	if (Seeker like '%gr' or Seeker like '%gr_%','gr',
		if (Seeker like '%gc' or Seeker like '%gc_%','gc','none')
	) as Segmentation,
	if (Seeker like '%grx%','grx','none') as SegmentationX,
	if (Seeker like '%maxd%', substr	ng(Seeker,instr(Seeker,'maxd')+4,4), '') as MaxDepth,
	case WinStatus 
		when 0 then '0 - unfinished'
		when 1 then '1 - win'
		when 2 then '2 - loose'
		when 3 then '3 - tie'
		else 'unknown'
	end as WinStatus
from GameListDetail
where Seeker not like 'test%'
	and StartTime>=(select min(StartTime) from SarsopLogTrial where StartTime>0) 
	and EndTime<=(select max(StartTime) from SarsopLogTrial)
group by MapName,MaxActions,Seeker,WinStatus



select g.*, gl.WinStatus as WinStatus_id, gl.NumActions, us.Name as Seeker, uh.name as Hider,
	if (uh.Name='RandomHider', 'Random',
		if (uh.Name='SmartHider', 'Smart',
			if (uh.Name like 'ActionList%', 'Random List','unknown')
		)
	) as HiderCat,
	if (us.Name like '%off%','offline','online') as OnOffline,
	if (us.Name like '%newrew%', 'triangle', 
		if (us.Name like '%sr_fsc%','final cross',
			if (us.Name like '%sr_fs%', 'final','unknown')
		)
	) as Reward,
	if (us.Name like '%gr' or us.Name like '%gr_%','Robot Centred',
		if (us.Name like '%gc' or us.Name like '%gc_%','Combi Centred','none')
		
	) as Segmentation,
	if (us.Name like '%grx%','grx','none') as SegmentationX,
	if (us.Name like '%maxd%', substring(us.Name,instr(us.Name,'maxd')+4,4), 'none') as MaxDepth,
	case gl.WinStatus 
		when 0 then '0 - unfinished'
		when 1 then '1 - win'
		when 2 then '2 - loose'
		when 3 then '3 - tie'
		else 'unknown'
	end as WinStatus
	
from ((Game g inner join GameList gl on gl.GameID=g.id) 
    left join User us on us.id=g.SeekerUserID)
    left join User uh on uh.id=g.HiderUserID


select Hider from GameListDetail where Hider like '%List%' and not Hider like 'ActionList%'


select * 
from GameListDetail
where not Seeker like '%test%' and not HiderCat='unknown'
and StartTime>=(select min(StartTime) from SarsopLogTrial where StartTime>0) 


*/