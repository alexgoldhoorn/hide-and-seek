
select MapSize,WinState,SeekerType2,HiderCat3,Reward, OnOffline,Segmentation,SegmentationX,count(*) n
from SimJuly2013
group by MapSize,WinState,Reward, Segmentation,SegmentationX,SeekerType2,HiderCat3;

