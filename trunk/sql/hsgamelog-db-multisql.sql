-- AG150113: last update - add seeker2
-- AG150505: update for n agents

-- create user
CREATE USER 'hsserver'@'localhost' IDENTIFIED BY 'hsserver_us3r_p@ss';

-- create db
CREATE DATABASE IF NOT EXISTS hsgamelog;

-- give privilges
GRANT ALL PRIVILEGES ON hsgamelog.* TO hsserver@'%' IDENTIFIED BY 'hsserver_us3r_p@ss' WITH GRANT OPTION;


USE hsgamelog;


-- user db
-- DROP TABLE IF EXISTS User;
CREATE TABLE User (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	Name VARCHAR(50),
	Automated bool 
);

-- DROP TABLE IF EXISTS ServerStart;
CREATE TABLE ServerStart (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	StartTime DATETIME,
	StopTime DATETIME
);

-- DROP TABLE IF EXISTS Game;
CREATE TABLE `Game` (
	`id` int(11) NOT NULL AUTO_INCREMENT,
	`MapName` varchar(255) DEFAULT NULL,
	`MapWidth` int(11) DEFAULT NULL,
	`MapHeight` int(11) DEFAULT NULL,
	`MapNumObst` int(11) DEFAULT NULL,
	`BaseRow` int(11) DEFAULT NULL,
	`BaseCol` int(11) DEFAULT NULL,

	`StartTime` datetime(3) DEFAULT NULL,
	`EndTime` datetime(3) DEFAULT NULL,
/*	`SeekerUserID` int(11) NOT NULL,
	`HiderUserID` int(11) NOT NULL,
	`Seeker2UserID` int(11) DEFAULT NULL,*/

	`OpponentType` int(11) DEFAULT NULL,
	`OpponentPosFile` varchar(255) DEFAULT NULL,
	`AutoWalkerType` int(11) DEFAULT NULL,
	`AutoWalkerN` int(11) DEFAULT NULL,
	`AutoWalkerPosFile` varchar(255) DEFAULT NULL,

	`GameType` smallint(6) NOT NULL,
	`UseContinuous` bool NOT NULL,	
	`StopAfterNumSteps` int(11) NOT NULL,
	`StopAtWin` bool NOT NULL, 
	`MaxNumActions` int(11) DEFAULT NULL,
    `WinDist` float(14,6) NOT NULL,
    `AllowInconsistObs` bool NOT NULL,
    
    `SimObsNoiseStd` float(14,6) NOT NULL,
	`SimObsFalseNegProb` float(14,6) NOT NULL,
    `SimObsFalsePosProb` float(14,6) NOT NULL,
    `SeekerStepDistance` float(14,6) NOT NULL,
    `HiderStepDistance` float(14,6) NOT NULL,
	
  PRIMARY KEY (`id`)
);

-- DROP TABLE IF EXISTS GameUser;
CREATE TABLE `GameUser` (
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
	`GameID` int(11) NOT NULL,    
	UserID int(11) NOT NULL,
    `SolverType` smallint(6) NOT NULL,
	MetaInfo varchar(1023) DEFAULT NULL,
	Comments varchar(255) DEFAULT NULL,
    IsSeeker BOOL,    
	
  PRIMARY KEY (`id`)
);

-- DROP TABLE IF EXISTS GameLine;
CREATE TABLE `GameLine` (
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
	`GameID` int(11) NOT NULL,
	`ActionNum` int(11) DEFAULT NULL,
	`SentTimeStamp` datetime(3) DEFAULT NULL,
    `status` smallint(6) DEFAULT NULL,	
  PRIMARY KEY (`id`)
); 

-- DROP TABLE IF EXISTS GameLineUser;
CREATE TABLE `GameLineUser` (
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
	`GameLineID` bigint(20) NOT NULL,
    `GameUserID` bigint(20) NOT NULL,	
	`UserTimeStamp` datetime(3) DEFAULT NULL,	
	`Action` smallint(6) DEFAULT NULL,
	`Row` int(11) DEFAULT NULL,
	`Col` int(11) DEFAULT NULL,
	`RowCont` float(14,6) DEFAULT NULL,
	`ColCont` float(14,6) DEFAULT NULL,
    `d_sh` float(14,6) DEFAULT NULL,
	`d_sb` float(14,6) DEFAULT NULL,
	`d_shEuc` float(14,6) DEFAULT NULL,
    `vis_sh` bool DEFAULT NULL,
	`HiderRowContWNoise` float(14,6) DEFAULT NULL,
	`HiderColContWNoise` float(14,6) DEFAULT NULL,
	`SeekerBeliefScore` float(14,6) DEFAULT NULL,
	`SeekerReward` float(14,6) DEFAULT NULL,    
    `SeekerChosenGoalRow` float(14,6) DEFAULT NULL,
    `SeekerChosenGoalCol` float(14,6) DEFAULT NULL,
  PRIMARY KEY (`id`)
); 

-- DROP TABLE IF EXISTS 
CREATE TABLE SolverType (
	id INT NOT NULL, PRIMARY KEY(id),
	Name VARCHAR(50)
);

INSERT INTO SolverType VALUES (0, 'Not Set');
INSERT INTO SolverType VALUES (1, 'MOMDP Offline');
INSERT INTO SolverType VALUES (2, 'MOMDP Layered');
INSERT INTO SolverType VALUES (3, 'MOMDP Layered Compare');
INSERT INTO SolverType VALUES (4, 'MCVI Offline');
INSERT INTO SolverType VALUES (5, 'POMCP');
INSERT INTO SolverType VALUES (6, 'N/A');
INSERT INTO SolverType VALUES (7, 'Follower LastPos');
INSERT INTO SolverType VALUES (8, 'Follower');
INSERT INTO SolverType VALUES (9, 'Smart Seeker');
INSERT INTO SolverType VALUES (10, 'Combi: POMCP & Follower');
INSERT INTO SolverType VALUES (11, 'N/A');
INSERT INTO SolverType VALUES (12, 'Follower LastPos Exact');
INSERT INTO SolverType VALUES (13, 'Highest Belief Follower POMCP');
INSERT INTO SolverType VALUES (14, 'Combi: Highest Bel. POCMP & Follower');
INSERT INTO SolverType VALUES (15, 'Multi Seeker Highest Belief Explorer');


-- --------------------------------------------------------------------------------
-- Routine DDL
-- Note: comments before and after the routine body will not be stored by the server
-- --------------------------------------------------------------------------------
DELIMITER $$   
DROP FUNCTION IF EXISTS AddGameByName $$
CREATE FUNCTION `AddGameByName`(
	`MapName` varchar(255) ,
	`MapWidth` int(11),
	`MapHeight` int(11),
	`MapNumObst` int(11),
	`BaseRow` int(11),
	`BaseCol` int(11),

	`OpponentType` int(11),
	`OpponentPosFile` varchar(255),
	`AutoWalkerType` int(11),
	`AutoWalkerN` int(11),
	`AutoWalkerPosFile` varchar(255),

	`GameType` smallint(6),
	`UseContinuous` bool,
	`SolverType` smallint(6),
	`StopAfterNumSteps` int(11),
	`StopAtWin` bool, 
	`MaxNumActions` int(11),
    `WinDist` float(14,6),
    `AllowInconsistObs` bool,
    
    `SimObsNoiseStd` float(14,6),
	`SimObsFalseNegProb` float(14,6),
    `SimObsFalsePosProb` float(14,6),
    `SeekerStepDistance` float(14,6),
    `HiderStepDistance` float(14,6)
) RETURNS int(11)
BEGIN
    INSERT INTO Game (
		`MapName`,
		`MapWidth`,
		`MapHeight`,
		`MapNumObst`,
		`BaseRow`,
		`BaseCol`,

		`StartTime`,

		`OpponentType`,
		`OpponentPosFile`,
		`AutoWalkerType`,
		`AutoWalkerN`,
		`AutoWalkerPosFile`,

		`GameType`,
		`UseContinuous`,
		`SolverType`,
		`StopAfterNumSteps`,
		`StopAtWin`,
		`MaxNumActions`,
		`WinDist`,
		`AllowInconsistObs`,

		`SimObsNoiseStd`,
		`SimObsFalseNegProb`,
		`SimObsFalsePosProb`,
		`SeekerStepDistance`,
		`HiderStepDistance`
	) VALUES (
		`MapName`,
		`MapWidth`,
		`MapHeight`,
		`MapNumObst`,
		`BaseRow`,
		`BaseCol`,

		NOW(3),

		`OpponentType`,
		`OpponentPosFile`,
		`AutoWalkerType`,
		`AutoWalkerN`,
		`AutoWalkerPosFile`,

		`GameType`,
		`UseContinuous`,
		`SolverType`,
		`StopAfterNumSteps`,
		`StopAtWin`,
		`MaxNumActions`,
		`WinDist`,
		`AllowInconsistObs`,

		`SimObsNoiseStd`,
		`SimObsFalseNegProb`,
		`SimObsFalsePosProb`,
		`SeekerStepDistance`,
		`HiderStepDistance`
	);

    RETURN LAST_INSERT_ID(); 
END

$$

DROP FUNCTION IF EXISTS AddGameByName $$
CREATE FUNCTION `AddGameByName`(
	GameID INT(11),
	`Name` VARCHAR(255),
	`MetaInfo` varchar(1023),
	`Comments` varchar(255),
    IsSeeker BOOL
) RETURNS bigint(20)
BEGIN
    DECLARE UserID INT;        
    DECLARE checkExists INT;
    SET checkExists = 0;
    SET UserID = -1;
	
    SELECT count(*) INTO checkExists FROM User WHERE Name=`Name`;
    IF (checkExists <= 0) THEN
        INSERT INTO User (Name,Automated) VALUES (`Name`, 0);
    END IF;

	INSERT INTO GameUser (GameID, UserID, MetaInfo, Comments, IsSeeker)
		VALUES (GameID, UserID, MetaInfo, Comments, IsSeeker);    
    
    RETURN LAST_INSERT_ID(); 
END

$$

DROP FUNCTION IF EXISTS AddGameLine $$
CREATE FUNCTION `AddGameLine`(
	GameID INT,
	ActionNum INT,
	SentTimeStamp DATETIME(3),
	status SMALLINT
) RETURNS bigint(20)
BEGIN    
    INSERT INTO GameLine (
		GameID,
		ActionNum,
		SentTimeStamp,
		status
	) VALUES (
		GameID,
		ActionNum,
		SentTimeStamp,
		status
	);
    RETURN LAST_INSERT_ID(); 
END

$$

DROP FUNCTION IF EXISTS AddGameLineUser $$
CREATE FUNCTION AddGameLineUser(
	GameLineID bigint(20),
    GameUserID bigint(20),
	UserTimeStamp DATETIME(3),
	`Action` SMALLINT,
	`Row` INT,
	`Col` INT,	
	`RowCont` float(14,6),
	`ColCont` float(14,6),
	d_sh float(14,6),
	d_sb float(14,6),
	d_shEuc float(14,6),
    `vis_sh` bool,
    HiderRowContWNoise float(14,6),
	HiderColContWNoise float(14,6),
	SeekerBeliefScore float(14,6),
	SeekerReward float(14,6),
    `SeekerChosenGoalRow` float(14,6),
    `SeekerChosenGoalCol` float(14,6)
) RETURNS bigint(20)
BEGIN    
    INSERT INTO GameLine (
		GameLineID,
		ActionNum,
        GameUserID,
		UserTimeStamp,		
		Action,
		Row,
		Col,
		RowCont,
		ColCont,
		d_sh, 
		d_sb, 
		d_shEuc,
        vis_sh,
		HiderRowContWNoise,
		HiderColContWNoise,
		SeekerBeliefScore,
		SeekerReward,
		`SeekerChosenGoalRow`,
		`SeekerChosenGoalCol`
	) VALUES (
				GameLineID,
		ActionNum,
        GameUserID,
		UserTimeStamp,		
		Action,
		Row,
		Col,
		RowCont,
		ColCont,
		d_sh, 
		d_sb, 
		d_shEuc,
        vis_sh,
		HiderRowContWNoise,
		HiderColContWNoise,
		SeekerBeliefScore,
		SeekerReward,
		`SeekerChosenGoalRow`,
		`SeekerChosenGoalCol`
	);
    RETURN LAST_INSERT_ID(); 
END

$$

-- DROP FUNCTION IF EXISTS StopGame;
CREATE FUNCTION `StopGame`(
	GameID INT
) RETURNS int(11)
BEGIN
    UPDATE Game 
    SET EndTime=NOW()
    WHERE id=GameID;

    RETURN GameID;
END 

$$

-- DROP FUNCTION IF EXISTS StartServer;
CREATE FUNCTION `StartServer`(
) RETURNS int(11)
BEGIN
    INSERT INTO ServerStart (
		StartTime
	) VALUES (
		NOW()
	);
    RETURN LAST_INSERT_ID(); -- AG TODO: doe this always work, session dependend??
END 

$$

-- DROP FUNCTION IF EXISTS StopServer;
CREATE FUNCTION `StopServer`(
	sessionid INT
) RETURNS int(11)
BEGIN
    UPDATE ServerStart 
    SET StopTime=NOW(3)
    WHERE id=sessionid;

    RETURN sessionid;
END 

$$

-- TODO: VIEWS

-- ag131202: added since the line timestampdiff(microsecond, ..) did not work in a view
--  (see bug: http://bugs.mysql.com/bug.php?id=60628)

create function MicroTimestampDiff (
t1 datetime(6),
t2 datetime(6)
) returns bigint(20)
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2);

$$

create function DistanceEuc (
r1 float(14,6),
c1 float(14,6),
r2 float(14,6),
c2 float(14,6)
) returns float(14,6)
    DETERMINISTIC
return SQRT(POW(r1-r2,2)+POW(c1-c2,2));


$$

delimiter ;

-- drop view if exists GameExt;
create view GameExt as
select g.*, 
	MicroTimestampDiff(StartTime,EndTime)/1000000 as  DurationS,
	s.Name as SolverTypeName,
	us.Name as Seeker,
	uh.Name as Hider,
	us2.Name as Seeker2
from Game g left join SolverType s on g.SolverType=s.id
	left join User us on us.id=g.SeekerUserID
	left join User uh on uh.id=g.HiderUserID
    left join User us2 on us2.id=g.Seeker2UserID;

drop view if exists GameLineExt;

 VIEW `GameLineExt` AS
    select 
        `GameLine`.`id` AS `id`,
        `GameLine`.`GameID` AS `GameID`,
        `GameLine`.`ActionNum` AS `ActionNum`,
        `GameLine`.`SentTimeStamp` AS `SentTimeStamp`,
        `GameLine`.`HiderTimeStamp` AS `HiderTimeStamp`,
        `GameLine`.`SeekerTimeStamp` AS `SeekerTimeStamp`,
        `GameLine`.`Seeker2TimeStamp` AS `Seeker2TimeStamp`,
        `GameLine`.`HiderAction` AS `HiderAction`,
        `GameLine`.`SeekerAction` AS `SeekerAction`,
        `GameLine`.`Seeker2Action` AS `Seeker2Action`,
        `GameLine`.`HiderRow` AS `HiderRow`,
        `GameLine`.`HiderCol` AS `HiderCol`,
        `GameLine`.`SeekerRow` AS `SeekerRow`,
        `GameLine`.`SeekerCol` AS `SeekerCol`,
        `GameLine`.`Seeker2Row` AS `Seeker2Row`,
        `GameLine`.`Seeker2Col` AS `Seeker2Col`,
        `GameLine`.`status` AS `status`,
        `GameLine`.`HiderRowCont` AS `HiderRowCont`,
        `GameLine`.`HiderColCont` AS `HiderColCont`,
        `GameLine`.`SeekerRowCont` AS `SeekerRowCont`,
        `GameLine`.`SeekerColCont` AS `SeekerColCont`,
        `GameLine`.`Seeker2RowCont` AS `Seeker2RowCont`,
        `GameLine`.`Seeker2ColCont` AS `Seeker2ColCont`,
        `GameLine`.`d_sh` AS `d_sh`,
        `GameLine`.`d_sb` AS `d_sb`,
        `GameLine`.`d_hb` AS `d_hb`,
        `GameLine`.`d_shEuc` AS `d_shEuc`,
        `GameLine`.`d_s2h` AS `d_s2h`,
        `GameLine`.`d_s2hEuc` AS `d_s2hEuc`,
        `GameLine`.`d_ss2` AS `d_ss2`,
        `GameLine`.`d_ss2Euc` AS `d_ss2Euc`,
        vis_sh,
        vis_s2h,
        vis_ss2,
        `GameLine`.`HiderRowContWNoise` AS `HiderRowContWNoise`,
        `GameLine`.`HiderColContWNoise` AS `HiderColContWNoise`,
        `GameLine`.`HiderRowContWNoise2` AS `HiderRowContWNoise2`,
        `GameLine`.`HiderColContWNoise2` AS `HiderColContWNoise2`,
        `GameLine`.`SeekerBeliefScore` AS `SeekerBeliefScore`,
        `GameLine`.`SeekerReward` AS `SeekerReward`,
        `GameLine`.`Seeker2BeliefScore` AS `Seeker2BeliefScore`,
        `GameLine`.`Seeker2Reward` AS `Seeker2Reward`,
        
        `SeekerGoalS1FromS1Row`,
		`SeekerGoalS1FromS1Col`,
		`SeekerGoalS1FromS1Belief`,
		`SeekerGoalS2FromS1Row`,
		`SeekerGoalS2FromS1Col`,
		`SeekerGoalS2FromS1Belief`,
		`SeekerGoalS1FromS2Row`,
		`SeekerGoalS1FromS2Col`,
		`SeekerGoalS1FromS2Belief`,
		`SeekerGoalS2FromS2Row`,
		`SeekerGoalS2FromS2Col`,
		`SeekerGoalS2FromS2Belief`,
		`Seeker1ChosenGoalRow`,
		`Seeker1ChosenGoalCol`,
		`Seeker2ChosenGoalRow`,
		`Seeker2ChosenGoalCol`,
        (MicroTimestampDiff(`GameLine`.`SentTimeStamp`,
                `GameLine`.`SeekerTimeStamp`) / 1000) AS `SeekerDuration_ms`,
        (MicroTimestampDiff(`GameLine`.`SentTimeStamp`,
                `GameLine`.`HiderTimeStamp`) / 1000) AS `HiderDuration_ms`,
        (MicroTimestampDiff(`GameLine`.`SentTimeStamp`,
                `GameLine`.`Seeker2TimeStamp`) / 1000) AS `Seeker2Duration_ms`,
/*        (case `GameLine`.`HiderRowContWNoise`
            when -(1) then 1
            else 0
        end) AS `IsHidden`,*/
		(case 
            when d_shEuc<1.2 or d_s2hEuc<1.2 then 1
            else 0
        end) AS `IsClose`,
		(case 
            when d_shEuc<=2 or d_s2hEuc<=2 then 1
            else 0
        end) AS `IsClose2`,
		(case 
            when d_shEuc<=3 or d_s2hEuc<=3 then 1
            else 0
        end) AS `IsClose3`,
        (CASE
			WHEN
				d_s2h<d_sh
			THEN
				d_s2h
			ELSE
				d_sh
			END) as d_sh_Min,
        (CASE
			WHEN
				d_s2hEuc<d_shEuc
			THEN
				d_s2hEuc
			ELSE
				d_shEuc
			END) as d_shEuc_Min,
		(CASE
		WHEN
			vis_sh=1 or vis_s2h=1
		THEN
			1
		ELSE
			0
		END) as vis_anysh
    from
        `GameLine`;
create database hsgamelog2015_2r

create view GameFirstStepClose as 
select GameID,min(ActionNum) as FirstStepClose
from GameLineExt
where IsClose=1
group by GameID;


create view GameFirstStepVisibile as 
select GameID,min(ActionNum) as FirstStepVisibile
from GameLineExt
where (vis_sh=1 or vis_s2h=1) and ActionNum>0
group by GameID;


create view GameLineStepDist as
select g2.GameID, g2.id, g2.ActionNum, MicroTimestampDiff(g1.SentTimeStamp,g2.Seeker2TimeStamp) / 1000 as StepDuration_ms,
	DistanceEuc(g1.HiderRowCont,g1.HiderColCont,g2.HiderRowCont,g2.HiderColCont) as HiderStepDist, 
    DistanceEuc(g1.SeekerRowCont,g1.SeekerColCont,g2.SeekerRowCont,g2.SeekerColCont) as SeekerStepDist, 
	DistanceEuc(g1.Seeker2RowCont,g1.Seeker2ColCont,g2.Seeker2RowCont,g2.Seeker2ColCont) as Seeker2StepDist, 
    DistanceEuc(g1.Seeker1ChosenGoalRow,g1.Seeker1ChosenGoalCol,g2.Seeker1ChosenGoalRow,g2.Seeker1ChosenGoalCol) as Seeker1ChosenDist, 
    DistanceEuc(g1.Seeker2ChosenGoalRow,g1.Seeker2ChosenGoalCol,g2.Seeker2ChosenGoalRow,g2.Seeker2ChosenGoalCol) as Seeker2ChosenDist
from GameLine g1 left join GameLine g2 on g1.GameID=g2.GameID and g2.ActionNum=g1.ActionNum+1;



create view GameLineFirstHVisib as
select GameID, min(ActionNum) FirstStepHVisible
from GameLine
where vis_sh=1 or vis_s2h=1
group by GameID;

create view GameLineFirstCloseToH as
select GameID, min(ActionNum) FirstStepCloseToH
from GameLineExt
where IsClose3=1
group by GameID;


-- todo change
CREATE VIEW `GameLineStats` AS
    select 
        `GameLineExt`.`GameID` AS `GameID`,
        max(`GameLineExt`.`ActionNum`) AS `NumActions`,
        avg(`GameLineExt`.`d_sh`) AS `avgDist`,
        std(`GameLineExt`.`d_sh`) AS `stdDist`,
        avg(`GameLineExt`.`d_shEuc`) AS `avgDistEuc`,
        std(`GameLineExt`.`d_shEuc`) AS `stdDistEuc`,
        avg(`GameLineExt`.`SeekerBeliefScore`) AS `avgSeekerBeliefScore`,
        std(`GameLineExt`.`SeekerBeliefScore`) AS `stdSeekerBeliefScore`,
        avg(`GameLineExt`.`SeekerDuration_ms`) AS `avgSeekerDuration_ms`,
        std(`GameLineExt`.`SeekerDuration_ms`) AS `stdSeekerDuration_ms`,
        avg(`GameLineExt`.`HiderDuration_ms`) AS `avgHiderDuration_ms`,
        std(`GameLineExt`.`HiderDuration_ms`) AS `stdHiderDuration_ms`,
        sum(`GameLineExt`.`IsHidden`) AS `NumHidden` -- ,
		-- FirstStepClose
    from
        `GameLineExt` GameLineExt 
	-- left join GameFirstStepClose gf on GameLineExt.GameID=gf.GameID
    group by `GameLineExt`.`GameID`;



