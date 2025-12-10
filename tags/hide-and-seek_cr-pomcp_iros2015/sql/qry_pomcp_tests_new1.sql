select *
from GameListDetail
where seeker like 'Pomcp_I%'


select Seeker,MapName,MapSize,WinState_id,
	avg(NumActions) avgNumActions,
    std(NumActions) stdNumActions,
	avg(DurationPerAction) avgDurationPerAction,
	std(DurationPerAction) stdDurationPerAction,
	count(*) as n
from GameListDetail
where seeker like 'pomcpc%' or seeker like 'pomcpnc%'--  'Pomcp2_hsb_I%'
group by Seeker,MapName,MapSize,WinState_id
order by Seeker,MapName,MapSize,WinState_id


select * from 
 GameListWinPercMapSeekerAndHiderCat
-- GameListWinCounterMapSeekerAndHiderCat
-- GameListMapSeekerAndHiderCatWinCounter
where (seeker like 'NPomcp%_upc%_I%' or seeker like 'NSmartS%_upc%'
 or seeker like 'NPomcp%_fme%_I%' or seeker like 'NSmartS%_fme%') and not seeker like '%sRc_En%'
order by Seeker,MapName




select * from GameListDetail
-- where id=3752
order by ID desc
limit 0,3

select * from GameLine
where id=3752