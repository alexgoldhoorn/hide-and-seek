-- AG150113: last update - add seeker2

-- create user
CREATE USER 'hsserver'@'localhost' IDENTIFIED BY 'hsserver_us3r_p@ss';

-- create db
CREATE DATABASE IF NOT EXISTS hsgamelog;

-- give privilges
GRANT ALL PRIVILEGES ON hsgamelog.* TO hsserver@'%' IDENTIFIED BY 'hsserver_us3r_p@ss' WITH GRANT OPTION;


USE hsgamelog;


-- AG151109: The tables with constants:
--  SolverType and MapName are created and filled in hsgamelog-constants.sql


-- user db
-- DROP TABLE IF EXISTS User;
CREATE TABLE User (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	Name VARCHAR(255),
	Automated bool 
);

-- DROP TABLE IF EXISTS ServerStart;
CREATE TABLE ServerStart (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	StartTime DATETIME,
	StopTime DATETIME
);

-- DROP TABLE IF EXISTS Game;
CREATE TABLE `Game` (    `id` int(11) NOT NULL AUTO_INCREMENT,
    `MapName` varchar(255) DEFAULT NULL,
    `MapWidth` int(11) DEFAULT NULL,
    `MapHeight` int(11) DEFAULT NULL,
    `MapNumObst` int(11) DEFAULT NULL,
    `BaseRow` int(11) DEFAULT NULL,
    `BaseCol` int(11) DEFAULT NULL,
    
    `ExpName` varchar(255) DEFAULT NULL, -- AG160314: name of experiment

    `StartTime` datetime(3) DEFAULT NULL,
    `EndTime` datetime(3) DEFAULT NULL,

    `OpponentType` int(11) DEFAULT NULL,
    `OpponentPosFile` varchar(255) DEFAULT NULL,
    `AutoWalkerType` int(11) DEFAULT NULL,
    `AutoWalkerN` int(11) DEFAULT NULL,
    `AutoWalkerPosFile` varchar(255) DEFAULT NULL,

    `GameType` smallint(6) NOT NULL,
    `UseContinuous` bool NOT NULL,
    `SolverType` smallint(6) NOT NULL,
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

    PRIMARY KEY (`id`),
    FOREIGN KEY (SolverType) REFERENCES SolverType(id)
);

-- ALTER TABLE `Game` ADD COLUMN 
--    `ExpName` varchar(255) DEFAULT NULL AFTER `BaseCol`;


-- alter table Game add index MapName (MapName);

-- DROP TABLE IF EXISTS GameUser;
CREATE TABLE `GameUser` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `GameID` int(11) NOT NULL,
    `UserID` int(11) NOT NULL,    
    `IsSeeker` bool NOT NULL, 
    `MetaInfo` varchar(1023) DEFAULT NULL,
    `Comments` varchar(255) DEFAULT NULL,

    PRIMARY KEY (`id`),
    FOREIGN KEY (GameID) REFERENCES Game(id),
    FOREIGN KEY (UserID) REFERENCES User(id)
);

-- DROP TABLE IF EXISTS GameLine;
CREATE TABLE `GameLine` (
    `id` bigint(20) NOT NULL AUTO_INCREMENT,
    `GameID` int(11) NOT NULL,
    `ActionNum` int(11) DEFAULT NULL,
    `SentTimeStamp` datetime(3) DEFAULT NULL,
    `status` smallint(6) DEFAULT NULL,

    PRIMARY KEY (`id`),
    FOREIGN KEY (GameID) REFERENCES Game(id)
);


-- DROP TABLE IF EXISTS GameUserLine;
CREATE TABLE `GameUserLine` (
    `id` bigint(20) NOT NULL AUTO_INCREMENT,
    `GameLineID` bigint(20) NOT NULL,
    `GameUserID` int(11) NOT NULL,    
    -- `ActionNum` int(11) DEFAULT NULL,
    -- `SentTimeStamp` datetime(3) DEFAULT NULL,    
    `ClientTimeStamp` datetime(3) DEFAULT NULL, -- was seeker/hiderTimestamp	
    `Action` smallint(6) DEFAULT NULL,
    `Row` int(11) DEFAULT NULL,
    `Col` int(11) DEFAULT NULL,
    `RowCont` float(14,6) DEFAULT NULL,
    `ColCont` float(14,6) DEFAULT NULL,

    `HiderRowContWNoise` float(14,6) DEFAULT NULL, -- observed pos of hider
    `HiderColContWNoise` float(14,6) DEFAULT NULL,

    `d_sh` float(14,6) DEFAULT NULL,	-- distance seeker-hider 
    `d_shEuc` float(14,6) DEFAULT NULL,  -- eucledian dist s-h
    `d_pb` float(14,6) DEFAULT NULL,  -- dist. player-base
    `d_pbEuc` float(14,6) DEFAULT NULL,  -- euc. distance p-b
    `vis_sh` bool DEFAULT NULL, -- is hider visib to seeker
    `visDyn_sh` bool DEFAULT NULL, -- is hider visib to seeker (including occlusions by dyn. obst.)

    `SeekerBeliefScore` float(14,6) DEFAULT NULL,
    `SeekerReward` float(14,6) DEFAULT NULL,

    /*    --not of that much interest
    `SeekerGoalS1FromS1Row` float(14,6) DEFAULT NULL,
    `SeekerGoalS1FromS1Col` float(14,6) DEFAULT NULL,
    `SeekerGoalS1FromS1Belief` float(14,6) DEFAULT NULL,
    `SeekerGoalS2FromS1Row` float(14,6) DEFAULT NULL,
    `SeekerGoalS2FromS1Col` float(14,6) DEFAULT NULL,
    `SeekerGoalS2FromS1Belief` float(14,6) DEFAULT NULL, */

    `ChosenGoalRow` float(14,6) DEFAULT NULL,
    `ChosenGoalCol` float(14,6) DEFAULT NULL,

    PRIMARY KEY (`id`),
    FOREIGN KEY (GameLineID) REFERENCES GameLine(id),
    FOREIGN KEY (GameUserID) REFERENCES GameUser(id)    
);



-- AG150826: added indexes (TODO: could be included in table)
-- alter table GameUser add index GameID (GameID);
-- alter table GameLine add index GameID (GameID);
-- alter table GameUserLine add index GameLineID (GameLineID);
-- AG160613: clienttimestamp should have ms resolution
-- alter table GameUserLine modify ClientTimeStamp datetime(3);


-- --------------------------------------------------------------------------------
-- Routine DDL
-- Note: comments before and after the routine body will not be stored by the server
-- --------------------------------------------------------------------------------
DELIMITER $$

DROP FUNCTION IF EXISTS AddGame $$
CREATE FUNCTION `AddGame`(
    `MapName` varchar(255) ,
    `MapWidth` int(11),
    `MapHeight` int(11),
    `MapNumObst` int(11),
    `BaseRow` int(11),
    `BaseCol` int(11),
    
    `ExpName` varchar(255),

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
    
		`ExpName`,

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
        
        `ExpName`,

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

DROP FUNCTION IF EXISTS AddGameUserByName $$
CREATE FUNCTION `AddGameUserByName`(	
    `GameID` int(11),
    `PName` VARCHAR(255),
    `IsSeeker` bool, 
    `MetaInfo` varchar(1023),
    `Comments` varchar(255)
)RETURNS int(11)
BEGIN
    DECLARE UserID INT(11);    
    DECLARE checkExists INT;
    SET checkExists = 0;
    SET UserID = -1;
	
    SELECT count(*) INTO checkExists FROM User WHERE Name=PName;
    IF (checkExists <= 0) THEN
        INSERT INTO User (Name,Automated) VALUES (PName, 0);
    END IF;

    SELECT id INTO UserID FROM User WHERE Name=PName;
    
    INSERT INTO GameUser (
        `GameID`,
        `UserID`,
        `IsSeeker`,
        `MetaInfo`,
        `Comments`
        ) VALUES (
        GameID,
        UserID,
        IsSeeker,
        MetaInfo,
        Comments
    );
    RETURN LAST_INSERT_ID(); 
END

$$

DROP FUNCTION IF EXISTS AddGameLine $$
CREATE FUNCTION `AddGameLine`(
    GameID int(11),
    ActionNum int(11),
    SentTimeStamp DATETIME(3),	
    `status` SMALLINT
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

DROP FUNCTION IF EXISTS AddGameUserLine $$
CREATE FUNCTION `AddGameUserLine`(
    GameUserID INT(11),
    GameLineID bigint(20),
    ClientTimeStamp DATETIME(3),
    `Action` SMALLINT,
    `Row` INT(11),
    `Col` INT(11),
    `RowCont` float(14,6),
    `ColCont` float(14,6),
    HiderRowContWNoise float(14,6), -- observed pos of hider
    HiderColContWNoise float(14,6),
    d_sh float(14,6),	-- distance seeker-hider 
    d_shEuc float(14,6), -- eucledian dist s-h
    d_pb float(14,6),  -- dist. player-base
    d_pbEuc float(14,6), -- euc. distance p-b
    vis_sh bool, -- is hider visib to seeker
    visDyn_sh bool, -- is hider visib to seeker (including occlusions by dyn. obst.)
    SeekerBeliefScore float(14,6),
    SeekerReward float(14,6),
    `ChosenGoalRow` float(14,6),
    `ChosenGoalCol` float(14,6)
) RETURNS bigint(20)
BEGIN    
    INSERT INTO GameUserLine (
        GameUserID,
        GameLineID,
        ClientTimeStamp,
        `Action`,
        `Row`,
        `Col`,	
        `RowCont`,
        `ColCont`,
        HiderRowContWNoise,
        HiderColContWNoise,
        d_sh,
        d_shEuc,
        d_pb,
        d_pbEuc,
        vis_sh,
        visDyn_sh,	
        SeekerBeliefScore,
        SeekerReward,
        `ChosenGoalRow`,
        `ChosenGoalCol`
        ) VALUES (
        GameUserID,
        GameLineID,
        ClientTimeStamp,
        `Action`,
        `Row`,
        `Col`,	
        `RowCont`,
        `ColCont`,
        HiderRowContWNoise,
        HiderColContWNoise,
        d_sh,
        d_shEuc,
        d_pb,
        d_pbEuc,
        vis_sh, 
        visDyn_sh,	
        SeekerBeliefScore,
        SeekerReward,
        `ChosenGoalRow`,
        `ChosenGoalCol`
    );
    RETURN LAST_INSERT_ID(); 
END

$$
DROP FUNCTION IF EXISTS StopGame $$
CREATE FUNCTION `StopGame`(
	GameID INT
) RETURNS int(11)
BEGIN
    UPDATE Game 
    SET EndTime=NOW(3)
    WHERE id=GameID;

    RETURN GameID;
END 

$$

DROP FUNCTION IF EXISTS StartServer $$
CREATE FUNCTION `StartServer`(
) RETURNS int(11)
BEGIN
    INSERT INTO ServerStart (
		StartTime
	) VALUES (
		NOW(3)
	);
    RETURN LAST_INSERT_ID(); -- AG TODO: doe this always work, session dependend??
END 

$$

DROP FUNCTION IF EXISTS StopServer $$
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

DROP FUNCTION IF EXISTS MicroTimestampDiff $$
create function MicroTimestampDiff (
t1 datetime(6),
t2 datetime(6)
) returns bigint(20)
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2);

$$

DROP FUNCTION IF EXISTS MicroTimestampDiffInS $$
create function MicroTimestampDiffInS (
t1 datetime(6),
t2 datetime(6)
) returns bigint(20)
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2)/1e6;

$$


DROP FUNCTION IF EXISTS DistanceEuc $$
create function DistanceEuc (
    r1 float(14,6),
    c1 float(14,6),
    r2 float(14,6),
    c2 float(14,6)
) returns float(14,6)
    DETERMINISTIC
return SQRT(POW(r1-r2,2)+POW(c1-c2,2));

$$

DROP FUNCTION IF EXISTS ExtractItFromHiderName $$
create function ExtractItFromHiderName (
    name varchar(255),
    isSeeker bool
) returns int(11)
BEGIN
    DECLARE str varchar(10);
    DECLARE it int(11);
    DECLARE sidx int;
    DECLARE eidx int;

    if (isSeeker) then
        -- only return for hider
        return null;
    else
        -- get iteration number
        set eidx := instr(name,'_walker');
        -- search last 'run' by searching in the reverse string for the reverse string
        set sidx := instr(reverse(name),'nur') ;
        
        if (eidx<=0 or sidx<=0) then
            -- no it. number
            return null;
        else
			-- fix start index since it is from the reverse string, and add 3 to compensate length of 'run' - 1
            set sidx := char_length(name) - sidx + 2;
        
            -- get string with it. nr
            set str := substr(name, sidx, eidx-sidx);
            -- convert it. nr to int
            set it := convert(str, signed integer);
            
            -- check if conversion ok
            if (str<>'0' and it=0) then
                -- not an int
                return null;
            else
               -- return the it. nr
               return it;
            end if;
        end if;
    end if;
END

$$
-- select ExtractItFromHiderName('blaas?sfdrun1234_walkervsdatxt.txt',false);
-- select ExtractItFromHiderName('File_/mnt/ramdisk/run2/run31_walker.txt',false);


DELIMITER $$ 
-- AG160413: find the 
DROP FUNCTION IF EXISTS FindFirstOccOrLength $$
create function FindFirstOccOrLength (
    name varchar(255),
    str varchar(255)
) returns int
BEGIN
    DECLARE idx int;     
    set idx := instr(name,str);
    
    if (idx<=0) then
        return char_length(name)+1;
    else
        return idx;
    end if;
END

$$

DROP FUNCTION IF EXISTS ExtractExpNameFromSeekerName $$
create function ExtractExpNameFromSeekerName (
    name varchar(255)
) returns varchar(255)
BEGIN
    DECLARE eidx int;     
    DECLARE l int;

    set l := char_length(name);
    
    set eidx := least(FindFirstOccOrLength(name,'_nc'),FindFirstOccOrLength(name,'_H'),FindFirstOccOrLength(name,'_d1'),FindFirstOccOrLength(name,'_bf'));
    
    if (eidx<=0) then
		return name;
	else
		return substr(name,1,eidx-1);    
    end if;
END

$$


delimiter ;

-- first user (for name of game)
create or replace view FirstGameUser as 
select GameID, min(gu.id) as FirstSeekerUserID, u.Name as FirstSeeker
from GameUser gu left join User u on gu.UserID=u.id
where IsSeeker=1  
group by GameID;
-- AG160413: this query does not use the Name of the first seeker necessarily, the following query does.
--  However when comparing the results of the following query with the previuos, the FirstSeeker name was the same.
-- AG160609: the join was wrong: u on UserID=gu.id -> was joining on SAME table!!

/*select * 
from FirstGameUser f 
left join 
(
select guf.GameID, FirstSeekerUserID, u.Name as FirstSeeker
from (
	select GameID, min(gu.id) as FirstSeekerUserID
	from GameUser gu
    group by GameID) gum    
left join GameUser guf on guf.id=gum.FirstSeekerUserID
left join User u on guf.UserID=u.id
) f2 on f.GameID=f2.GameID
where f.FirstSeeker<>f2.FirstSeeker
;*/


-- users per game
create or replace view GameUserCount as
select GameID, count(*) as nPlayers, sum(isSeeker) as nSeekers  
from GameUser
group by GameID;

-- game user extended 
create or replace view GameUserExt as
select gu.id, GameID, UserID, Name, IsSeeker, MetaInfo, Comments 
from GameUser gu left join User u on UserID=u.id;

-- game extended
create or replace view GameExt as
select g.*, 
	MicroTimestampDiffInS(StartTime,EndTime) as  DurationS,
	s.Name as SolverTypeName,
    s.PubName as SolverTypePubName,
	us.nPlayers, nSeekers, nPlayers-nSeekers as nHiders, FirstSeeker,
    m.PubName as MapPubName
from Game g 
	left join GameUserCount us on g.id=us.GameID
    left join FirstGameUser fgu on g.id=fgu.GameID
    left join SolverType s on g.SolverType=s.id
    left join MapName m on g.MapName=m.FileName;
    
-- data from gameline
create or replace view GameExtFromLine as 
select GameID, count(*) n, max(ActionNum) lastActionNum, min(SentTimeStamp) FirstSentTime, max(SentTimeStamp) LastSentTimeStamp
from GameLine
group by GameID;
	
-- game line extended: best
create or replace view GameLineBest as
select GameID, GameLineID, ActionNum, min(d_sh) min_d_sh, min(d_shEuc) min_d_shEuc, max(vis_sh) max_vis_sh, max(visDyn_sh) max_visDyn_sh,
max(SeekerBeliefScore) max_SeekerBeliefScore, max(SeekerReward) max_seekerReward
from GameUserLine gul left join GameLine gl on gl.id=gul.GameLineID
group by GameID, GameLineID, ActionNum;

-- Game res
create or replace view GameRes as
select GameID, count(*) n, max(ActionNum) maxActNum, avg(min_d_sh) avg_min_d_sh, avg(min_d_shEuc) avg_min_d_shEuc,
	avg(max_vis_sh) avg_max_vis_sh, avg(max_visDyn_sh) avg_max_visDyn_sh, avg(max_SeekerBeliefScore) avg_max_SeekerBeliefScore,
    avg(max_seekerReward) avg_max_seekerReward
from GameLineBest
group by GameID;


-- first visible (with dyn. obst, in case of none, it is the same as vis_sh)
-- AG160421: updated, more efficient
-- AG160506: used "not HiderRowContWNoise is null" instead of "visDyn_sh=true", since
-- 			 since the first is what the robot has observed (sent from server to client as obs.)
create or replace view FirstVisibStep as 
select gl.GameID, min(ActionNum) FirstVisibStep
from GameLine gl
	left join GameUserLine gul on gl.id=gul.GameLineID
	left join GameUser gu on gu.id=gul.GameUserID
where gu.isSeeker=true and not HiderRowContWNoise is null and HiderRowContWNoise >=0 -- AG160616: was 'is null' only, but it is -1 !
group by gl.GameID;

/*select GameID, min(ActionNum) FirstVisibStep
from GameLineBest
where max_visDyn_sh=1
group by GameID;*/

-- first close step (of one of the robots) for 1, 3, 5 cells
-- AG151104: changed such that always a step higher than first visib step is given

-- drop view if exists FirstClose1Step;
create or replace view FirstClose1Step as 
select gl.GameID, min(ActionNum) FirstClose1Step, FirstVisibStep
from GameLineBest gl left join FirstVisibStep f on gl.GameID=f.GameID
where min_d_shEuc<=1 and gl.ActionNum>=FirstVisibStep
group by GameID, FirstVisibStep;


-- drop view if exists FirstClose3Step;
create or replace view FirstClose3Step as 
select gl.GameID, min(ActionNum) FirstClose3Step, FirstVisibStep
from GameLineBest gl left join FirstVisibStep f on gl.GameID=f.GameID
where min_d_shEuc<=3 and gl.ActionNum>=FirstVisibStep
group by GameID;

-- drop view if exists FirstClose5Step;
create or replace view FirstClose5Step as 
select gl.GameID, min(ActionNum) FirstClose5Step, FirstVisibStep
from GameLineBest gl left join FirstVisibStep f on gl.GameID=f.GameID
where min_d_shEuc<=5 and gl.ActionNum>=FirstVisibStep
group by GameID;

-- AG160209
-- calculate difference between lines
create or replace view GameLineStepDuration as
select gl2.id as GameLineID,gl2.GameID,gul2.GameUserID,gu.UserID,gu.IsSeeker,gl2.ActionNum, MicroTimestampDiff(gl1.SentTimeStamp,gul2.ClientTimeStamp) / 1000 as StepDuration_ms
from GameLine gl1 
	-- left join GameUserLine gul1 on gl1.id = gul1.GameLineID
    -- left join GameUser gu on gu.id=gul.GameUserID
    left join GameLine gl2 on gl1.GameID=gl2.GameID and gl2.ActionNum=gl1.ActionNum+1
	left join GameUserLine gul2 on gl2.id = gul2.GameLineID 
    left join GameUser gu on gu.id=gul2.GameUserID;

-- AG160411
-- Extract the iteration number of the hider game user by using the Name field which has the format
-- "File_run#_walker.txt", and we retrieve 
create or replace view GameUserHiderIteration as
select distinct Name, /*@it:=case when isSeeker=1 then NULL else substr(Name, 9, instr(Name,'_walker')-9) end as itStr, 
		  @itN:=case when isSeeker=1 then NULL else convert(@it,unsigned integer) end as itConv,
		  case when isSeeker=1 or (@it<>'0' and @itN=0) then NULL else @itN end as itNum,*/
                  ExtractItFromHiderName(Name, isSeeker) as itNumF
from GameUserExt;
-- where isSeeker=0;

-- AG150113
-- list the hider per Game (assuming there is only 1)
create or replace view GameHider as
select GameID, Name as Hider, MetaInfo as MetaInfoHider
from GameUserExt 
where isSeeker=false;

-- AG160413
-- Extended Game overview with hider name, hider meta info, and experiment name (extracted from the FirstSeeker).
-- The last two columns are used to get the statistics.
/*create or replace view GameExt2 as
select g.*, gh.Hider, gh.MetaInfoHider, ExtractExpNameFromSeekerName(FirstSeeker) as ExpName
from GameExt g left join GameHider gh on g.id=gh.GameID;*/

-- select * from GameExt;


-- --

select *
from GameExt2
where FirstSeeker like 'multi_j1%';




create or replace view GameExt2 as
select g.*, guh.Hider, guh.MetaInfoHider, ExtractExpNameFromSeekerName(FirstSeeker) as ExpName
from GameExt g left join (
    select GameID, Name as Hider, MetaInfoHider
    from GameUserExt 
) guh on g.id=guh.GameID;



select * 
from GameUserExt
order by id desc
-- TODO: use the Name of the hider to link same iterations, BUT should be same experiment 
		(Name of experiment or part of name of Seeker AND same directory where run, if possible should be in different directory 
			OR USE: MetaInfo -> contains port of server
		In SUMMARY: MetaInfo of hider should be same, find/follow same, username partly same.... 
    OR: create other queries

select *
from GameExt
where FirstSeeker like ''

set @test='multi_j1_follow_H_nc';
select @test;
select @test,@i:=least(instr(@test,'_nc'),instr(@test,'_H')),substr(@test,1,@i)



select FindFirstOccOrLength('abcdef','b');
select ExtractExpNameFromSeekerName('testing_bf');
