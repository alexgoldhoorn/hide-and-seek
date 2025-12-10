

select OnOffline,HiderCat,Reward,WinState,Seeker,SegmentationShort,Segmentation,count(*) n,
		avg(Duration) as Duration_avg, std(Duration) as Duration_std,
		avg(DurationPerAction) DurationPerAction_avg,std(DurationPerAction) DurationPerAction_std,
		avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std
from GameListDetailFiltered
where seeker like '%_fme_w1%'
group by OnOffline,HiderCat,Reward,WinState,Seeker,SegmentationShort,Segmentation


select HiderCat,count(*) n
from GameListDetailFiltered
where seeker like '%fme7x9_w1%'
group by HiderCat


select OnOffline,HiderCat,MapName,WinState,Reward,Segmentation,SegmentationX,MaxDepth,count(*) n
from GameListDetailFiltered
where seeker like '%_fme_w1%' and SegmentationShort<>'rcc'  	
group by HiderCat,MapName,WinState,Segmentation,SegmentationX,MaxDepth




select OnOffline,HiderCat,MapName,WinState,Reward,Segmentation,SegmentationX,MaxDepth,count(*) n,
		avg(GameDuration) as Duration_avg, std(GameDuration) as Duration_std,
		avg(DurationPerAction) DurationPerAction_avg,std(DurationPerAction) DurationPerAction_std,
		avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std
from GameListDetailFiltered
where seeker like '%fme7x9_w1_fs2%' -- and SegmentationShort<>'rcc'
group by OnOffline,HiderCat,MapName,WinState,Reward,Segmentation,SegmentationX,MaxDepth

drop view SimMay2013;
create view SimMay2013 as 
--  OnOffline,HiderCat,Hider,MapName,MapSize,WinState,Reward,Segmentation,SegmentationX,MaxDepth,DurationWithLoad,GameDuration,DurationPerAction,NumActions,MaxTime,SeekerType,RobotCenteredCircles
select OnOffline,HiderCat,Hider,MapName,MapSize,WinState,Reward,
    if(OnOffline='offline','none',Segmentation) as Segmentation,
    if(OnOffline='offline','none',SegmentationX) as SegmentationX,
    MaxDepth, if(MapWidth>MapHeight,MapWidth,MapHeight) as MapDim,
    MapWidth*MapHeight as NumMapCells, MapWidth*MapHeight-MapNumObst as NumStates,
    MapNumObst as NumObst,MapWidth,MapHeight,
	DurationWithLoad,GameDuration,DurationPerAction,NumActions,
	if (Seeker like '%10s%', '10s',
		if (Seeker like '%2s%', '2s', 
			if (Seeker like '%100s%','100s','300s')
		)
	) as MaxTime,
	if(OnOffline='offline','offline',
        if (Seeker like '%sim4%','SmartSeeker',
                if (Seeker like '%sim3%', 'TopFSReward',
                    if(Seeker like '%sim2%', 'BottomPnRew', 
                        if(Seeker='SmartSeeker','SmartSeeker','other')
                    )
                )
        )
	) as SeekerType,
	if(OnOffline='offline','none',if (Seeker like '%2c%', 'online_2c','online_1c')) as RobotCenteredCircles,
	if(WinState like '%win%', 1, 0) as IsWin,
	if(WinState like '%lose%', 1, 0) as IsLose,
	if(WinState like '%tie%', 1, 0) as IsTie,WinState_id
from GameListDetail -- Filtered
where seeker like '%sim3%' or seeker like '%sim4%' or seeker like '%sim2%' or seeker='SmartSeeker' ;
-- and SegmentationShort<>'rcc'
into outfile '/tmp/anly_sim11.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

drop view SimMay2013h;

create view SimMay2013h as 
select * , if(HiderCat='Random List','Random',HiderCat) as HiderCat2,
if (HiderCat like '%Random%','random',
    if(HiderCat = 'All Knowing Very Smart', 'smart ff',
        if (HiderCat ='Very Smart','smart','??')
    )
) as HiderCat3,
if (OnOffline='offline' or SeekerType='SmartSeeker','none', MaxTime) as MaxTime2,
if (SeekerType='SmartSeeker','none',Reward) as Reward2,
if (OnOffline='offline' or SeekerType='SmartSeeker' or MaxTime='300s','no', 'yes') as HasTimeLimit,
if(SeekerType='offline','off-line',
    if (SeekerType='SmartSeeker', 'smart seeker',
            if (SeekerType='TopFSReward', 'on-line (top rew.)',
                if(SeekerType='BottomPnRew', 'on-line', '??')
            )
    )
) as SeekerType2
from SimMay2013
where HiderCat in (
-- 'All Knowing Very Smart',
'Very Smart',
'Random List',
'Random') and WinState_id>0;

select distinct HiderCat,HiderCat3 from SimMay2013h

-- query 
select OnOffline, sum(IsWin) as Win, count(*) as n
from SimMay2013
group by OnOffline


select OnOffline,NumActions
from SimMay2013
where IsWin=1
group by OnOffline


select * from
SimMay2013h
where iswin=0 and islose=0 and istie=0

select Name from User order by id desc;

select max(StartTime)
from Game -- List -- Detail
order by id desc;

--
select concat_ws(',',MapSize,SeekerType) as N, 
sum(IsWin) as Win, sum(IsLose) as Lose, sum(IsTie) as Tie, count(*) as Total,
NumActions_avg,NumActions_std,DurationPerAction_avg,DurationPerAction_std
from SimMay2013h as s1 left join
(select concat_ws(',',MapSize,SeekerType) as N2, 
avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std,
 avg(DurationPerAction) as DurationPerAction_avg, std(DurationPerAction) as DurationPerAction_std
from SimMay2013h
where HasTimeLimit='no' and IsWin=1
group by MapSize,SeekerType
) as s2 on concat_ws(',',MapSize,SeekerType) =s2.N2
where HasTimeLimit='no'
group by MapSize,SeekerType

-- total
select 
avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std,
 avg(DurationPerAction) as DurationPerAction_avg, std(DurationPerAction) as DurationPerAction_std
from SimMay2013h
where HasTimeLimit='no' and IsWin=1


select distinct SeekerType,MaxTime,HasTimeLimit
from SimMay2013h

select *
from GameListDetail -- Filtered
where id = (select max(id) from Game);


select count(*),max(StartTime)
from GameListDetailFiltered
where seeker like '%sim3%' or seeker like '%sim4%' or seeker like '%sim2%' or seeker='SmartSeeker' 

 -- -----
select OnOffline,if(seeker like '%grx%','X segmentation',if(seeker like '%sim3%','sim3',
	if(seeker like '%sim4%','sim4-smarts',seeker) ) ) as Type, 
	count(*),max(StartTime)
from GameListDetailFiltered
where seeker like '%sim3%'
group by if(seeker like '%grx%','X segmentation',if(seeker like '%sim3%','sim3',
	if(seeker like '%sim4%','sim4-smarts',seeker) ) ) ,onoffline;


select OnOffline,HiderCat,MapName,MapSize,WinState,Reward,Segmentation,SegmentationX,MaxDepth,count(*) n,
		avg(DurationWithLoad) as DurationWithLoad_avg, std(DurationWithLoad) as DurationWithLoad_std,
		avg(DurationPerAction) DurationPerAction_avg,std(DurationPerAction) DurationPerAction_std,
		avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std,
		avg(GameDuration) as GameDuration_avg,
		std(GameDuration) as GameDuration_std,
		avg(DurationPerAction) as DurationPerAction_avg,
		std(DurationPerAction) as DurationPerAction_std
from GameListDetailFiltered
-- where seeker like '%sim3%' -- and SegmentationShort<>'rcc'
-- and mapsize='12x12'
where seeker like '%sim3%' or seeker like '%sim4%' or seeker like '%sim2%' or seeker='SmartSeeker' 
group by OnOffline,HiderCat,MapName,MapSize,WinState,Reward,Segmentation,SegmentationX,MaxDepth;


