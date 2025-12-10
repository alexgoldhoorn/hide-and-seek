
drop view SimMay2013ss ;


create view SimMay2013ss as 
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
	if(WinState like '%tie%', 1, 0) as IsTie,WinState_id,

if(HiderCat='Random List','Random',HiderCat) as HiderCat2,
if (HiderCat like '%Random%','random',
    if(HiderCat = 'All Knowing Very Smart', 'smart ff',
        if (HiderCat ='Very Smart','smart','??')
    )
) as HiderCat3,
if (OnOffline='offline' or Seeker='SmartSeeker','none', 
	if (Seeker like '%10s%', '10s',
		if (Seeker like '%2s%', '2s', 
			if (Seeker like '%100s%','100s','300s')
		)
	)
) as MaxTime2,
if (Seeker='SmartSeeker','none',Reward) as Reward2,
if (OnOffline='offline' or Seeker='SmartSeeker' or Seeker like '%300s%'='300s','no', 'yes') as HasTimeLimit,

	if(OnOffline='offline','off-line',
        if (Seeker like '%sim4%','smart seeker',
                if (Seeker like '%sim3%', 'on-line (top rew.)',
                    if(Seeker like '%sim2%', 'on-line', 
                        if(Seeker='SmartSeeker','smart seeker','other')
                    )
                )
        )
	) as SeekerType2


from GameListDetail -- Filtered
where seeker='SmartSeeker' and (Hider='VerySmartHider' or Hider='AllKnowingVerySmartHider')
 and WinState_id>0





create view SimMay2013ssh as 
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
from SimMay2013ss
where HiderCat in (
-- 'All Knowing Very Smart',
'Very Smart',
'Random List',
'Random') and WinState_id>0;






select Name from User where Name like '%hider%' and name like '%smart%';