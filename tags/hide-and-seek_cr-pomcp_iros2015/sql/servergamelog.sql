/***** Create Server Game Log ******

Notes:
- should be run as root (on MySQL)



Author      : Alex Goldhoorn
Last update : 30/5/2012
Version     : 0.1
*/


-- create user
CREATE USER 'hsserver'@'localhost' IDENTIFIED BY 'hsserver_us3r_p@ss';

-- create db
CREATE DATABASE IF NOT EXISTS hsgamelog;

-- give privilges
GRANT ALL PRIVILEGES ON hsgamelog.* TO hsserver@localhost IDENTIFIED BY 'hsserver_us3r_p@ss' WITH GRANT OPTION;



/*
TODO: 
- put params in the game table


*/


USE hsgamelog;

-- user db
DROP TABLE IF EXISTS User;
CREATE TABLE User (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	Name VARCHAR(50),
	Automated BIT(1) 
);-- this is a boolean (0: false, rest: true) 

-- experiment
DROP TABLE IF EXISTS Game;
CREATE TABLE Game (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	MapName VARCHAR(50),
	MapWidth INT,
	MapHeight INT,
	MapNumObst INT,
	BaseRow INT,
	BaseCol INT,
	MaxActions INT,
	StartTime DATETIME,	
    	EndTime DATETIME,
	SeekerUserID INT NOT NULL REFERENCES User(id),
	HiderUserID INT NOT NULL REFERENCES User(id)
);

DROP TABLE IF EXISTS GameLine;
CREATE TABLE GameLine (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),	
    	GameID INT NOT NULL REFERENCES Game(id),
	ActionNum INT,
	HiderTimeStamp DATETIME,
	SeekerTimeStamp DATETIME,
	HiderAction SMALLINT,
	SeekerAction SMALLINT,
	HiderRow int,
	HiderCol int,
	SeekerRow int,
	SeekerCol int,
	status SMALLINT
);


DROP TABLE IF EXISTS ServerStart;
CREATE TABLE ServerStart (
	id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id),
	StartTime DATETIME,
	StopTime DATETIME
);




DELIMITER $$

-- TODO: VIEWS

-- ag131202: added since the line timestampdiff(microsecond, ..) did not work in a view
--  (see bug: http://bugs.mysql.com/bug.php?id=60628)

create function MicroTimestampDiff (
t1 datetime,
t2 datetime
) returns bigint(20)
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2);

$$

-- drop VIEW GameList
CREATE OR REPLACE VIEW GameList AS
SELECT GameID, max(status) WinStatus, max(ActionNum) NumActions,
	-- timestampdiff(MICROSECOND,min(SeekerTimeStamp),max(SeekerTimeStamp))/1000000 as GameDuration
	MicroTimestampDiff(min(SeekerTimeStamp),max(SeekerTimeStamp))/1000000 as GameDuration
FROM GameLine
GROUP BY GameID;





$$
select * from GameListDetail
CREATE VIEW GameListDetail2 AS
select g.*, gl.WinStatus as WinState_id, gl.NumActions, us.Name as Seeker, uh.name as Hider,
        if((uh.Name = 'RandomHider'),
            'Random',
            if((uh.Name='AllKnowingVerySmartHider'),
            'All Knowing Very Smart',
                if((uh.Name='AllKnowingSmartHider'),
                'All Knowing Smart',
                    if((uh.Name='VerySmartHider'),
                    'Very Smart',
                        if((uh.Name='SmartHider'),
                        'Smart',
                        if((`uh`.`Name` like 'ActionList%'),
                            'Random List',
                            'unknown')))))) AS `HiderCat`,
	if (us.Name like '%off%','offline','online') as OnOffline,
	if (us.Name like '%newrew%', 'triangle', 
		if (us.Name like '%sr_fsc%','final cross',
			if (us.Name like '%sr_fs%', 'final','unknown')
		)
	) as Reward,
	if (us.Name like '%gr' or us.Name like '%gr_%','Robot Centred',
		if (us.Name like '%gc' or us.Name like '%gc_%','Combi Centred','none')
		
	) as Segmentation,
	if (us.Name like '%grx%','Robot Centred X','none') as SegmentationX,
	if (us.Name like '%maxd%', substring(us.Name,instr(us.Name,'maxd')+4,4), 'none') as MaxDepth,
	case gl.WinStatus 
		when 0 then '0 - unfinished'
		when 1 then '1 - win'
		when 2 then '2 - lose'
		when 3 then '3 - tie'
		else 'unknown'
	end as WinState,
	if (us.Name like '%maxd%','yes','no') as MaxDepthFilter,
	concat(if(MapWidth<10,' ',''), cast(MapWidth as char(2)), 'x', cast(MapHeight as char(2)) ) as MapSize,
	timestampdiff(second,StartTime,EndTime) as DurationWithLoad,
	--timestampdiff(second,StartTime,EndTime)/NumActions as DurationPerActionWithLoad,
	gl.GameDuration,
	gl.GameDuration/gl.NumActions as DurationPerAction,
    gln.SeekerRow as LastSeekerRow,
    gln.SeekerCol as LastSeekerCol,
    gln.HiderRow as LastHiderRow,
    gln.HiderCol as LastHiderCol,
    if(SeekerRow=BaseRow and SeekerCol=BaseCol,1,0) as SeekerAtBase,
    if(HiderRow=BaseRow and HiderCol=BaseCol,1,0) as HiderAtBase
from ((Game g inner join GameList gl on gl.GameID=g.id) 
    left join User us on us.id=g.SeekerUserID)
    left join User uh on uh.id=g.HiderUserID
    left join GameLine gln on gln.GameID=g.id and gln.status=gl.WinStatus

$$
drop VIEW GameListDetailFiltered
CREATE VIEW GameListDetailFiltered AS
select *, 
	if (HiderCat='Random List','Random',HiderCat) as HiderCatShort,
	if (Segmentation='Robot Centred','rc',if (Segmentation='Combi Centred','rcc',Segmentation)) as SegmentationShort,
	if (SegmentationX='grx','rc',SegmentationX) as SegmentationXShort
from GameListDetail
where not HiderCat='unknown'
	and not Seeker like '%test%' and not Hider like '%test%'
	and WinState_id<>0
--	and StartTime>=(select min(StartTime) from SarsopLogTrial where StartTime>0) 


$$

DROP FUNCTION IF EXISTS AddGame;
CREATE DEFINER=`root`@`localhost` FUNCTION `AddGame`(
	MapName VARCHAR(50),
	MapWidth INT,
	MapHeight INT,
	MapNumObst INT,
	BaseRow INT,
	BaseCol INT,
	MaxActions INT,
	SeekerUserID INT,
	HiderUserID INT
) RETURNS int(11)
BEGIN
    INSERT INTO Game (
		MapName,
		MapWidth,
		MapHeight,
		MapNumObst,
		BaseRow,
		BaseCol,
		MaxActions,
		StartTime,
		SeekerUserID,
		HiderUserID
	) VALUES (
		MapName,
		MapWidth,
		MapHeight,
		MapNumObst,
		BaseRow,
		BaseCol,
		MaxActions,
		NOW(),
		SeekerUserID,
		HiderUserID
	);
    RETURN LAST_INSERT_ID(); -- AG TODO: doe this always work, session dependend??
END

$$

DROP FUNCTION IF EXISTS AddGameByName;
CREATE DEFINER=`root`@`localhost` FUNCTION `AddGameByName`(
	MapName VARCHAR(50),
	MapWidth INT,
	MapHeight INT,
	MapNumObst INT,
	BaseRow INT,
	BaseCol INT,
	MaxActions INT,
	SeekerUser VARCHAR(50),
	HiderUser VARCHAR(50)
) RETURNS int(11)
BEGIN
    DECLARE SeekerUserID INT;
    DECLARE HiderUserID INT;
    DECLARE checkExists INT;
    SET checkExists = 0;
    SET SeekerUserID = -1;
    SET HiderUserID = -1;

	-- check user, if not add
    SELECT count(*) INTO checkExists FROM User WHERE Name=SeekerUser;
    IF (checkExists <= 0) THEN
	INSERT INTO User (Name,Automated) VALUES (SeekerUser, 0);
    END IF;

    SELECT count(*) INTO checkExists FROM User WHERE Name=HiderUser;
    IF (checkExists <= 0) THEN
	INSERT INTO User (Name,Automated) VALUES (HiderUser, 0);
    END IF;

    SELECT id INTO SeekerUserID FROM User WHERE Name=SeekerUser;
    SELECT id INTO HiderUserID FROM User WHERE Name=HiderUser;

    INSERT INTO Game (
		MapName,
		MapWidth,
		MapHeight,
		MapNumObst,
		BaseRow,
		BaseCol,
		MaxActions,
		StartTime,
		SeekerUserID,
		HiderUserID
	) VALUES (
		MapName,
		MapWidth,
		MapHeight,
		MapNumObst,
		BaseRow,
		BaseCol,
		MaxActions,
		NOW(),
		SeekerUserID,
		HiderUserID
	);
    RETURN LAST_INSERT_ID(); -- AG TODO: doe this always work, session dependend??
END
 
$$

DROP FUNCTION IF EXISTS AddGameLine;

CREATE DEFINER=`root`@`localhost` FUNCTION `AddGameLine`(
	GameID INT,
	ActionNum INT,
	HiderTimeStamp DATETIME,
	SeekerTimeStamp DATETIME,
	HiderAction SMALLINT,
	SeekerAction SMALLINT,
	HiderRow INT,
	HiderCol INT,
	SeekerRow INT,
	SeekerCol INT,
	status SMALLINT
) RETURNS INT(11)
BEGIN 
   
    INSERT INTO GameLine (
	    	GameID,
		ActionNum,
		HiderTimeStamp,
		SeekerTimeStamp,
		HiderAction,
		SeekerAction,
		HiderRow,
		HiderCol,
		SeekerRow,
		SeekerCol,
		status
	) VALUES (
	    	GameID,
		ActionNum,
		HiderTimeStamp,
		SeekerTimeStamp,
		HiderAction,
		SeekerAction,
		HiderRow,
		HiderCol,
		SeekerRow,
		SeekerCol,
		status
	);
    RETURN LAST_INSERT_ID(); -- AG TODO: doe this always work, session dependend??
END

$$

DROP FUNCTION IF EXISTS StopGame;
CREATE DEFINER=`root`@`localhost` FUNCTION `StopGame`(
	GameID INT
) RETURNS int(11)
BEGIN
    UPDATE Game 
    SET EndTime=NOW()
    WHERE id=GameID;

    RETURN GameID;
END 

$$

DROP FUNCTION IF EXISTS StartServer;
CREATE DEFINER=`root`@`localhost` FUNCTION `StartServer`(
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

DROP FUNCTION IF EXISTS StopServer;
CREATE DEFINER=`root`@`localhost` FUNCTION `StopServer`(
	sessionid INT
) RETURNS int(11)
BEGIN
    UPDATE ServerStart 
    SET StopTime=NOW()
    WHERE id=sessionid;

    RETURN sessionid;
END 

$$

DELIMITER ;

-- insert user
INSERT INTO User (Name,Automated) VALUES ('Alex', 0);
INSERT INTO User (Name,Automated) VALUES ('RandomHider', 1);
INSERT INTO User (Name,Automated) VALUES ('SmartHider', 1);

SELECT * FROM User;
