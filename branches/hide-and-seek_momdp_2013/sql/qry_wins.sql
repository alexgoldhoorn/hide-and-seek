select MapName,MaxActions,Seeker,avg(Duration) as Duration_avg,std(Duration) as Duration_std,
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
	if (Seeker like '%maxd%', substring(Seeker,instr(Seeker,'maxd')+4,4), '') as MaxDepth

from GameListDetail
where Seeker not like 'test%'
	and StartTime>=(select min(StartTime) from SarsopLogTrial where StartTime>0) 
	and EndTime<=(select max(StartTime) from SarsopLogTrial)
group by MapName,MaxActions,Seeker,

into outfile '/tmp/outres_wins_all5.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'

