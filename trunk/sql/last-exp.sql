
select *
from SimMay2013ss
where MapSize='9x12'


select MapSize,WinState,SeekerType2,Reward, OnOffline,Segmentation,SegmentationX,count(*) n
from SimJuly2013
group by MapSize,WinState,Reward, Segmentation,SegmentationX,SeekerType2

select * from GameListDetail
where 	 StartTime>'2013-07-07'
and seeker = 'SmartSeeker'
# where StartTime> '2013-07-07'