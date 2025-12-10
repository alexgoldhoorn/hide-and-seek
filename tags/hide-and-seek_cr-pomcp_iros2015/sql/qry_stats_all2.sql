
/* HEADER:

FileName,Info,nX avg,nX std,nY avg,nY std,nO avg,nO std,nA avg,nA std,initBoundsT avg,initBoundsT std,initTotalT avg,initTotalT std,InitPrecis avg,InitPrecis std,Duration avg,Duration std,lbBackT avg,lbBackT std,ubBackT avg,ubBackT std,sampleT avg,sampleT std,pruneT avg,pruneT std,nIt avg,nIt std,elepsedT avg,elepsedT std,nTrials avg,nTrials std,lowerBound avg,lowerBound std,upperBound avg,upperBound std,precis avg,precis std,nAlphas avg,nAlphas std,nBeliefs avg,nBeliefs std,nBackups avg,nBackups std,nLB backup avg,nLB backup std,nUB backup avg,nUB backup std,nTrialsPerFile,Init Time %,LB back time %,UB back time %,sample time %,prune time %,missing time %


wanted to add game result details, but can't because in SARSOP files trials are run.. and that is one step


	-- ag130117: add game details [WARNING could cause incorrect associations or double associations doubling the data lines]
	left join GameListDetail g on g.StartTime<=tot.StartTime and g.EndTime>=tot.EndTime 
*/

select *, 1.0-(initTime_perc+lbBackTime_perc+ubBackTime_perc+sampleTime_perc+pruneTime_perc) as missingTime_perc

from (

select *,
		-- ag130117 new
		1.0*initTotalT_avg/elepsedT_avg as initTime_perc,
		1.0*lbBackT_avg/elepsedT_avg as lbBackTime_perc,
		1.0*ubBackT_avg/elepsedT_avg as ubBackTime_perc,
		1.0*sampleT/elepsedT_avg as sampleTime_perc,
		1.0*pruneT/elepsedT_avg as pruneTime_perc
from (

	select l.FileName, l.info, 
		avg(nX) as nX_avg,				std(nX) as nX_std,
		avg(nY) as nY_avg,				std(nY) as nY_std,
		avg(nO) as nO_avg,				std(nO) as nO_std,
		avg(nA) as nA_avg,				std(nA) as nA_std,
		avg(initBoundsT) initBoundsT_avg,std(initBoundsT) initBoundsT_std,
		avg(initTotalT) initTotalT_avg,	std(initTotalT) initTotalT_std,
		avg(t.precis) as InitPrecis_avg,std(t.precis) as InitPrecis_std, 
		avg(Duration) as Duration_avg,	std(Duration) as Duration_std,
		avg(lbBackT) as lbBackT_avg, 	std(lbBackT) as lbBackT_std, 
		avg(ubBackT) as ubBackT_avg,	std(ubBackT) as ubBackT_std,
		avg(sampleT) as sampleT_avg,	std(sampleT) as sampleT_std,
		avg(pruneT) as pruneT_avg, 		std(pruneT) as pruneT_std, 
		avg(tot.nIt) as nIt_avg,		std(tot.nIt) as nIt_std,
		avg(tot.elapsedT) as elepsedT_avg,std(tot.elapsedT) as elepsedT_std,
		avg(tot.nTrials) as nTrials_avg,std(tot.nTrials) as nTrials_std,
		avg(tot.lowerBound) as lowerBound_avg,std(tot.lowerBound) as lowerBound_std,
		avg(tot.upperBound) as upperBound_avg,std(tot.upperBound) as upperBound_std,
		avg(tot.precis) as precis_avg,	std(tot.precis) as precis_std,
		avg(tot.nAlphas) as nAlphas_avg,std(tot.nAlphas) as nAlphas_std,
		avg(tot.nBeliefs) as nBeliefs_avg, std(tot.nBeliefs) as nBeliefs_std,
		avg(tot.nBackups) as nBackups_avg,std(tot.nBackups) as nBackups_std,
		avg(nLbBack) as nLbBack_avg,	std(nLbBack) as nLbBack_std,
		avg(nUbBack) as nUbBack_avg,	std(nUbBack) as nUbBack_std,
		avg(nTrialsPerFile) as nTrialsPerFile_avg
		
	from (

		select str.TrialID, min(Time) StartTime, max(Time) EndTime, time_to_sec(timediff(max(time),min(time))) as Duration,
			sum(lbBackT) as lbBackT, sum(ubBackT) as ubBackT, sum(nLbBack) as nLbBack, sum(nUbBack) as nUbBack, sum(sampleT) as sampleT, 
			sum(pruneT) as pruneT, g.nIt, g.elapsedT,g.nTrials,g.nBackups,g.lowerBound,g.upperBound,g.precis,g.nAlphas,g.nBeliefs,g.stop
			-- ag130117 new
			
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
	left join SarsopLog l on l.id=t.FileID -- and l.info like '%grx'
	left join (
		select FileId,count(*) nTrialsPerFile
		from SarsopLogTrial
		group by FileId
	) trls on t.FileID=trls.FileID

	group by l.FileName, l.info -- , nX,nY,nO,nA

) as allData

) as allData2

into outfile '/tmp/outres_all2.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'

/*
select MapName,MaxActions,Seeker,WinStatus,avg(Duration) as Duration_avg,std(Duration) as Duration_std,
	avg(NumActions) as NumActions_avg, std(NumActions) as NumActions_std, count(*) n
from GameListDetail
where Seeker like '%grx_1c'
group by MapName,MaxActions,Seeker,WinStatus
*/
/*
select min(importedTime),max(importedTime) -- *
from SarsopLog
where Info like '%2c%'

select min(importedTime),max(importedTime) -- *
from SarsopLog
where Info like '%2c%'


select min(Time),max(Time)
from SarsopLog sl left join SarsopLogTrial st on sl.id=st.FileID
left join SarsopLogTrialRow sr on sr.TrialID=st.id
where Info like '%2c%'

*/


