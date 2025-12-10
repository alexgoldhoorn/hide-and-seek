

select FileName, initTotalT,initLBoundT,initUBoundT, 
	st.elapsedT as initElapsedT, max(sr.elapsedT) maxElapsedT, st.nBackups as initNBackups, 
	max(sr.nBackups) maxNBackups, st.precis as initPrecis, min(sr.precis) minPrecis,
	sum(lbBackT) sumLbBackT, sum(ubBackT) sumUBBackT, 
	sum(nLbBack) as nLbBack, sum(nUbBack) as nUbBack, sum(sampleT) as sampleT,sum(pruneT) as pruneT
from SarsopLog s left join SarsopLogTrial st on s.id=st.FileID
	left join SarsopLogTrialRow sr on sr.TrialID = st.id
group by FileName, initTotalT,initLBoundT,initUBoundT,st.elapsedT


into outfile '/tmp/outsarsoplog_stats2.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'
