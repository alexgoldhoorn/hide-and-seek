
select l.FileName, l.info, nX,nY,nO,nA,
avg(initBoundsT) initBoundsT,
avg(initTotalT) initTotalT,
avg(t.precis) as InitPrecis, 
avg(Duration) as Duration,
avg(lbBackT) as lbBackT, avg(ubBackT) as ubBackT,
avg(sampleT) as sampleT,
avg(pruneT) as pruneT, avg(g.nIt) as nIt,
avg(g.elapsedT) as elepsedT,
avg(g.nTrials) as nTrials,
avg(g.lowerBound) as lowerBound,
avg(g.upperBound) as upperBound,
avg(g.precis) as precis,
avg(g.nAlphas) as nAlphas,
avg(g.nBeliefs) as nBeliefs

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
left join SarsopLog l on l.id=t.FileID -- and l.info='offline'
group by l.FileName, l.info, nX,nY,nO,nA