
/* HEADER:

FileName,Info,nX avg,nX std,nY avg,nY std,nO avg,nO std,nA avg,nA std,initBoundsT avg,initBoundsT std,initTotalT avg,initTotalT std,InitPrecis avg,InitPrecis std,Duration avg,Duration std,lbBackT avg,lbBackT std,ubBackT avg,ubBackT std,sampleT avg,sampleT std,pruneT avg,pruneT std,nIt avg,nIt std,elepsedT avg,elepsedT std,nTrials avg,nTrials std,lowerBound avg,lowerBound std,upperBound avg,upperBound std,precis avg,precis std,nAlphas avg,nAlphas std,nBeliefs avg,nBeliefs std,nBackups avg,nBackups std,nLB backup avg,nLB backup std,nUB backup avg,nUB backup std,nTrialsPerFile

change:
* added: 
	- stdev
	- n trials per file
*/
/*


CREATE TABLE SarsopLogTrialPerLine (
SarsopLogID int, ImportedTime datetime, FileName varchar(255), Info varchar(255),
	
TrialID int, TrialStartTime datetime, nX int, nY int, nO int, nA int, initUBoundT float, initLBoundT float, 
		initBoundsT float, initSampleEngineT float, initSARSOPPruneT float,
		initBeliefTreeT float, initTotalT float, initNIt int, initElapsedT float, initNTrials int, initNBackups int, 
		initLowerBound float, initUpperBound float, initPrecis float, initNAlphas int, initNBeliefs int, totalT float,
		StartTime datetime, EndTime datetime, Duration  float,
		lbBackT float, ubBackT float, nLbBack int, nUbBack int, sampleT float, 
		pruneT float, nIt int, elapsedT  float, nTrials int, nBackups int, 
		lowerBound float, upperBound float,precis float,nAlphas int,nBeliefs int,stop int
);
*/

INSERT INTO SarsopLogTrialPerLine
select l.id as SarsopLogID, ImportedTime, FileName, Info,
	t.id as TrialID, t.StartTime as TrialStartTime, nX, nY, nO, nA, initUBoundT, initLBoundT, initBoundsT, initSampleEngineT, initSARSOPPruneT,
		initBeliefTreeT, initTotalT, t.nIt, t.elapsedT, t.nTrials, t.nBackups, t.lowerBound, t.upperBound, t.precis, t.nAlphas, 
		t.nBeliefs, t.totalT,
	
	tot.StartTime, tot.EndTime, tot.Duration,
		tot.lbBackT, tot.ubBackT, tot.nLbBack, tot.nUbBack, tot.sampleT, 
		tot.pruneT, tot.nIt, tot.elapsedT,tot.nTrials,tot.nBackups,tot.lowerBound,tot.upperBound,tot.precis,tot.nAlphas,tot.nBeliefs,tot.stop


from (

	select str.TrialID, min(Time) StartTime, max(Time) EndTime, time_to_sec(timediff(max(time),min(time))) as Duration,
		sum(lbBackT) as lbBackT, sum(ubBackT) as ubBackT, sum(nLbBack) as nLbBack, sum(nUbBack) as nUbBack, sum(sampleT) as sampleT, 
		sum(pruneT) as pruneT, g.nIt, g.elapsedT,g.nTrials,g.nBackups,g.lowerBound,g.upperBound,g.precis,g.nAlphas,g.nBeliefs,g.stop
	from SarsopLogTrialRow str
	left join (
		select str.TrialID, id, nIt, elapsedT,nTrials,nBackups,lowerBound,upperBound,precis,nAlphas,nBeliefs,stop
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
left join SarsopLog l on l.id=t.FileID 
left join (
	select FileId,count(*) nTrialsPerFile
	from SarsopLogTrial
	group by FileId
) trls on t.FileID=trls.FileID;

