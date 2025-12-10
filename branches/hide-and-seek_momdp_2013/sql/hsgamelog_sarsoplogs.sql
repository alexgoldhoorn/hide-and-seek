select * from GameListDetail
order by id desc


CREATE USER 'hsgameloguser'@'localhost' IDENTIFIED BY 'hsg@m3l0gus3r';

-- give privilges
GRANT ALL PRIVILEGES ON hsgamelog.* TO hsgameloguser@localhost IDENTIFIED BY 'hsg@m3l0gus3r' WITH GRANT OPTION;



drop TABLE if exists SarsopLog;
CREATE TABLE SarsopLog (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),    
    ImportedTime DATETIME,
	FileName VARCHAR(255),
	Info VARCHAR(255)
);
drop table if exists SarsopLogTrial;
CREATE TABLE SarsopLogTrial (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),    
	FileID INT NOT NULL, 
    StartTime DATETIME,
	nX INT, nY INT, nO INT, nA INT,
	initUBoundT FLOAT, initLBoundT FLOAT,
	initBoundsT FLOAT, initSampleEngineT FLOAT,
	initSARSOPPruneT FLOAT, initBeliefTreeT FLOAT,
	initTotalT FLOAT, 
	nIt INT, elapsedT FLOAT, nTrials INT, nBackups INT,
	lowerBound FLOAT, upperBound FLOAT, precis FLOAT,
	nAlphas INT, nBeliefs INT, totalT FLOAT
);

/*[time],SOLVE,#iteration,elapsed time,#trials,#backups,

lower bound,upper bound,precision,#alphas,#beliefs,
low bound backup time,up bound backup time,
#low bound backups,#up bound backups,sample time,
prune time,stop,total time*/
drop table if exists SarsopLogTrialRow;
CREATE TABLE SarsopLogTrialRow (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),    
	TrialID INT NOT NULL, 
    Time DATETIME, nIt INT, elapsedT FLOAT,
	nTrials INT, nBackups INT,
	lowerBound FLOAT, upperBound FLOAT, precis FLOAT,
	nAlphas INT, nBeliefs INT,
	lbBackT FLOAT, ubBackT FLOAT,
	nLbBack INT, nUbBack INT, 
	sampleT FLOAT, pruneT FLOAT,
	stop INT, totalItT FLOAT
);


select Now();
SELECT VERSION();

INSERT INTO SarsopLog (ImportedTime,FileName) VALUES ('2012/11/21 18:25:48','test');

select * from SarsopLog;

SELECT MAX(id) FROM SarsopLog;
