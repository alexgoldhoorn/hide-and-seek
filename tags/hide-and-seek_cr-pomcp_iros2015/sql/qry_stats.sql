
select l.FileName, l.info, nX,nY,nO,nA,initBoundsT,initTotalT,t.precis as InitPrecis, tot.*

from (

	select str.TrialID, min(Time) StartTime, max(Time) EndTime, time_to_sec(timediff(max(time),min(time))) as Duration,
		sum(lbBackT) as lbBackT, sum(ubBackT) as ubBackT, sum(nLbBack) as nLbBack, sum(nUbBack) as nUbBack, sum(sampleT) as sampleT, 
		sum(pruneT) as pruneT, g.nIt, g.elapsedT,g.nTrials,g.lowerBound,g.upperBound,g.precis,g.nAlphas,g.nBeliefs,g.stop
	from SarsopLogTrialRow str
	left join (
		select str.TrialID, id, nIt, elapsedT,nTrials,lowerBound,upperBound,precis,nAlphas,nBeliefs,stop
		from SarsopLogTrialRow str
		inner join (
			select TrialID, max(id) maxID 
			from SarsopLogTrialRow
			group by TrialID
		) m on str.id=maxID
	) g on g.TrialID = str.TrialID
	group by str.TrialID
) tot
left join SarsopLogTrial t on tot.TrialID=t.id
left join SarsopLog l on l.id=t.FileID and l.info='offline'

into outfile '/tmp/outres.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'

