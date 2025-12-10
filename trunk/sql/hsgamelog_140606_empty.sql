-- MySQL dump 10.13  Distrib 5.5.37, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: hsgamelog
-- ------------------------------------------------------
-- Server version	5.6.14

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `Game`
--

DROP TABLE IF EXISTS `Game`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
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
  `SeekerUserID` int(11) NOT NULL,
  `HiderUserID` int(11) NOT NULL,
  `OpponentType` int(11) DEFAULT NULL,
  `OpponentPosFile` varchar(255) DEFAULT NULL,
  `AutoWalkerType` int(11) DEFAULT NULL,
  `AutoWalkerN` int(11) DEFAULT NULL,
  `AutoWalkerPosFile` varchar(255) DEFAULT NULL,
  `GameType` smallint(6) NOT NULL,
  `UseContinuous` tinyint(1) NOT NULL,
  `SolverType` smallint(6) NOT NULL,
  `StopAfterNumSteps` int(11) NOT NULL,
  `StopAtWin` tinyint(1) NOT NULL,
  `MaxNumActions` int(11) DEFAULT NULL,
  `WinDist` float(14,6) NOT NULL,
  `AllowInconsistObs` tinyint(1) NOT NULL,
  `SimObsNoiseStd` float(14,6) NOT NULL,
  `SimObsFalseNegProb` float(14,6) NOT NULL,
  `SimObsFalsePosProb` float(14,6) NOT NULL,
  `SeekerStepDistance` float(14,6) NOT NULL,
  `HiderStepDistance` float(14,6) NOT NULL,
  `SeekerMetaInfo` varchar(255) DEFAULT NULL,
  `HiderMetaInfo` varchar(255) DEFAULT NULL,
  `SeekerComments` varchar(255) DEFAULT NULL,
  `HiderComments` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Game`
--

LOCK TABLES `Game` WRITE;
/*!40000 ALTER TABLE `Game` DISABLE KEYS */;
/*!40000 ALTER TABLE `Game` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `GameLine`
--

DROP TABLE IF EXISTS `GameLine`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `GameLine` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `GameID` int(11) NOT NULL,
  `ActionNum` int(11) DEFAULT NULL,
  `SentTimeStamp` datetime(3) DEFAULT NULL,
  `HiderTimeStamp` datetime(3) DEFAULT NULL,
  `SeekerTimeStamp` datetime(3) DEFAULT NULL,
  `HiderAction` smallint(6) DEFAULT NULL,
  `SeekerAction` smallint(6) DEFAULT NULL,
  `HiderRow` int(11) DEFAULT NULL,
  `HiderCol` int(11) DEFAULT NULL,
  `SeekerRow` int(11) DEFAULT NULL,
  `SeekerCol` int(11) DEFAULT NULL,
  `status` smallint(6) DEFAULT NULL,
  `HiderRowCont` float(14,6) DEFAULT NULL,
  `HiderColCont` float(14,6) DEFAULT NULL,
  `SeekerRowCont` float(14,6) DEFAULT NULL,
  `SeekerColCont` float(14,6) DEFAULT NULL,
  `d_sh` float(14,6) DEFAULT NULL,
  `d_sb` float(14,6) DEFAULT NULL,
  `d_hb` float(14,6) DEFAULT NULL,
  `d_shEuc` float(14,6) DEFAULT NULL,
  `SeekerBeliefScore` float(14,6) DEFAULT NULL,
  `HiderRowContWNoise` float(14,6) DEFAULT NULL,
  `HiderColContWNoise` float(14,6) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `GameLine`
--

LOCK TABLES `GameLine` WRITE;
/*!40000 ALTER TABLE `GameLine` DISABLE KEYS */;
/*!40000 ALTER TABLE `GameLine` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Temporary table structure for view `GameList`
--

DROP TABLE IF EXISTS `GameList`;
/*!50001 DROP VIEW IF EXISTS `GameList`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `GameList` (
  `GameID` tinyint NOT NULL,
  `WinStatus` tinyint NOT NULL,
  `NumActions` tinyint NOT NULL,
  `GameDuration` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `GameListMapAndPlayers`
--

DROP TABLE IF EXISTS `GameListMapAndPlayers`;
/*!50001 DROP VIEW IF EXISTS `GameListMapAndPlayers`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `GameListMapAndPlayers` (
  `MapName` tinyint NOT NULL,
  `SeekerUserID` tinyint NOT NULL,
  `HiderUserID` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `GameListWinCounter`
--

DROP TABLE IF EXISTS `GameListWinCounter`;
/*!50001 DROP VIEW IF EXISTS `GameListWinCounter`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `GameListWinCounter` (
  `MapName` tinyint NOT NULL,
  `SeekerUserID` tinyint NOT NULL,
  `HiderUserID` tinyint NOT NULL,
  `NumWin` tinyint NOT NULL,
  `NumLose` tinyint NOT NULL,
  `NumTie` tinyint NOT NULL,
  `AvgGameDurationPerStepWin` tinyint NOT NULL,
  `AvgGameDurationPerStepLose` tinyint NOT NULL,
  `AvgGameDurationPerStepTie` tinyint NOT NULL,
  `AvgNumActionsWin` tinyint NOT NULL,
  `AvgNumActionsLose` tinyint NOT NULL,
  `AvgNumActionsTie` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `GameListWinPerc`
--

DROP TABLE IF EXISTS `GameListWinPerc`;
/*!50001 DROP VIEW IF EXISTS `GameListWinPerc`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `GameListWinPerc` (
  `MapName` tinyint NOT NULL,
  `SeekerUserID` tinyint NOT NULL,
  `HiderUserID` tinyint NOT NULL,
  `NumWin` tinyint NOT NULL,
  `NumLose` tinyint NOT NULL,
  `NumTie` tinyint NOT NULL,
  `AvgGameDurationPerStepWin` tinyint NOT NULL,
  `AvgGameDurationPerStepLose` tinyint NOT NULL,
  `AvgGameDurationPerStepTie` tinyint NOT NULL,
  `AvgNumActionsWin` tinyint NOT NULL,
  `AvgNumActionsLose` tinyint NOT NULL,
  `AvgNumActionsTie` tinyint NOT NULL,
  `Seeker` tinyint NOT NULL,
  `Hider` tinyint NOT NULL,
  `NumTotal` tinyint NOT NULL,
  `WinPerc` tinyint NOT NULL,
  `LosePerc` tinyint NOT NULL,
  `TiePerc` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `GameListWinStateCounter`
--

DROP TABLE IF EXISTS `GameListWinStateCounter`;
/*!50001 DROP VIEW IF EXISTS `GameListWinStateCounter`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `GameListWinStateCounter` (
  `MapName` tinyint NOT NULL,
  `SeekerUserID` tinyint NOT NULL,
  `HiderUserID` tinyint NOT NULL,
  `WinStatus` tinyint NOT NULL,
  `n` tinyint NOT NULL,
  `AvgGameDuration` tinyint NOT NULL,
  `AvgGameDurationPerStep` tinyint NOT NULL,
  `AvgNumActions` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `GameSteps`
--

DROP TABLE IF EXISTS `GameSteps`;
/*!50001 DROP VIEW IF EXISTS `GameSteps`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `GameSteps` (
  `id` tinyint NOT NULL,
  `GameID` tinyint NOT NULL,
  `ActionNum` tinyint NOT NULL,
  `SentTimeStamp` tinyint NOT NULL,
  `HiderTimeStamp` tinyint NOT NULL,
  `SeekerTimeStamp` tinyint NOT NULL,
  `HiderAction` tinyint NOT NULL,
  `SeekerAction` tinyint NOT NULL,
  `HiderRow` tinyint NOT NULL,
  `HiderCol` tinyint NOT NULL,
  `SeekerRow` tinyint NOT NULL,
  `SeekerCol` tinyint NOT NULL,
  `status` tinyint NOT NULL,
  `HiderStepTime` tinyint NOT NULL,
  `SeekerStepTime` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `ServerStart`
--

DROP TABLE IF EXISTS `ServerStart`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ServerStart` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `StartTime` datetime(3) DEFAULT NULL,
  `StopTime` datetime(3) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=21 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ServerStart`
--

LOCK TABLES `ServerStart` WRITE;
/*!40000 ALTER TABLE `ServerStart` DISABLE KEYS */;
INSERT INTO `ServerStart` VALUES (1,'2014-06-06 20:08:27.428',NULL),(2,'2014-06-06 20:12:54.058',NULL),(3,'2014-06-06 20:18:03.209',NULL),(4,'2014-06-06 20:19:13.107',NULL),(5,'2014-06-06 20:23:44.330',NULL),(6,'2014-06-06 20:33:46.498',NULL),(7,'2014-06-06 20:34:46.794',NULL),(8,'2014-06-06 20:39:14.137',NULL),(9,'2014-06-06 20:39:28.483',NULL),(10,'2014-06-06 20:41:12.575',NULL),(11,'2014-06-06 20:41:35.738',NULL),(12,'2014-06-06 20:53:09.271',NULL),(13,'2014-06-06 20:54:21.775',NULL),(14,'2014-06-06 20:54:36.767',NULL),(15,'2014-06-06 20:56:21.391',NULL),(16,'2014-06-06 20:56:38.423',NULL),(17,'2014-06-06 21:21:29.518',NULL),(18,'2014-06-06 21:22:34.860',NULL),(19,'2014-06-06 21:34:16.574',NULL),(20,'2014-06-06 22:47:15.719',NULL);
/*!40000 ALTER TABLE `ServerStart` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `User`
--

DROP TABLE IF EXISTS `User`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `User` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  `Automated` bit(1) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `User`
--

LOCK TABLES `User` WRITE;
/*!40000 ALTER TABLE `User` DISABLE KEYS */;
/*!40000 ALTER TABLE `User` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `varsPomcp`
--

DROP TABLE IF EXISTS `varsPomcp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `varsPomcp` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  `version` varchar(50) DEFAULT NULL,
  `reward` varchar(10) DEFAULT NULL,
  `depth` int(11) DEFAULT NULL,
  `numSim` int(11) DEFAULT NULL,
  `numBelief` int(11) DEFAULT NULL,
  `explorationConst` float DEFAULT NULL,
  `expandCount` int(11) DEFAULT NULL,
  `rewardAggregType` varchar(10) DEFAULT NULL,
  `hiderSimType` varchar(10) DEFAULT NULL,
  `hiderSimRandomProb` float DEFAULT NULL,
  `discount` float DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `varsPomcp`
--

LOCK TABLES `varsPomcp` WRITE;
/*!40000 ALTER TABLE `varsPomcp` DISABLE KEYS */;
/*!40000 ALTER TABLE `varsPomcp` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'hsgamelog'
--
/*!50003 DROP FUNCTION IF EXISTS `AddGameByName` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` FUNCTION `AddGameByName`(
	`SeekerUser` VARCHAR(255),
	`HiderUser` VARCHAR(255),

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
    `HiderStepDistance` float(14,6),

	`SeekerMetaInfo` varchar(255),
	`HiderMetaInfo` varchar(255),
	`SeekerComments` varchar(255),
	`HiderComments` varchar(255)
) RETURNS int(11)
BEGIN
    DECLARE SeekerUserID INT;
    DECLARE HiderUserID INT;
    DECLARE checkExists INT;
    SET checkExists = 0;
    SET SeekerUserID = -1;
    SET HiderUserID = -1;

	
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
		`MapName`,
		`MapWidth`,
		`MapHeight`,
		`MapNumObst`,
		`BaseRow`,
		`BaseCol`,

		`StartTime`,
		`SeekerUserID`,
		`HiderUserID`,

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
		`HiderStepDistance`,

		`SeekerMetaInfo`,
		`HiderMetaInfo`,
		`SeekerComments`,
		`HiderComments`
	) VALUES (
		`MapName`,
		`MapWidth`,
		`MapHeight`,
		`MapNumObst`,
		`BaseRow`,
		`BaseCol`,

		NOW(3),
		SeekerUserID,
		HiderUserID,

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
		`HiderStepDistance`,

		`SeekerMetaInfo`,
		`HiderMetaInfo`,
		`SeekerComments`,
		`HiderComments`
	);
    RETURN LAST_INSERT_ID(); 
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `AddGameLine` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` FUNCTION `AddGameLine`(
	GameID INT,
	ActionNum INT,
	SentTimeStamp DATETIME(3),
	HiderTimeStamp DATETIME(3),
	SeekerTimeStamp DATETIME(3),
	HiderAction SMALLINT,
	SeekerAction SMALLINT,
	HiderRow INT,
	HiderCol INT,
	SeekerRow INT,
	SeekerCol INT,
	status SMALLINT,
	d_sh float(14,6),
	d_sb float(14,6),
	d_hb float(14,6), 
	d_shEuc float(14,6),
	SeekerBeliefScore float(14,6),
	HiderRowContWNoise float(14,6),
	HiderColContWNoise float(14,6)
) RETURNS bigint(20)
BEGIN    
    INSERT INTO GameLine (
		GameID,
		ActionNum,
		SentTimeStamp,
		HiderTimeStamp,
		SeekerTimeStamp,
		HiderAction,
		SeekerAction,
		HiderRow,
		HiderCol,
		SeekerRow,
		SeekerCol,
		status, 
		d_sh, 
		d_sb, 
		d_hb, 
		d_shEuc,
		SeekerBeliefScore,
		HiderRowContWNoise,
		HiderColContWNoise
	) VALUES (
		GameID,
		ActionNum,
		SentTimeStamp,
		HiderTimeStamp,
		SeekerTimeStamp,
		HiderAction,
		SeekerAction,
		HiderRow,
		HiderCol,
		SeekerRow,
		SeekerCol,
		status,
		d_sh, 
		d_sb, 
		d_hb, 
		d_shEuc,
		SeekerBeliefScore,
		HiderRowContWNoise,
		HiderColContWNoise
	);
    RETURN LAST_INSERT_ID(); 
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `AddGameLineCont` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`%` FUNCTION `AddGameLineCont`(
	GameID INT,
	ActionNum INT,
	SentTimeStamp DATETIME(3),
	HiderTimeStamp DATETIME(3),
	SeekerTimeStamp DATETIME(3),
	HiderAction SMALLINT,
	SeekerAction SMALLINT,
	HiderRow INT,
	HiderCol INT,
	SeekerRow INT,
	SeekerCol INT,
	status SMALLINT,
	HiderRowCont float(14,6),
	HiderColCont float(14,6),
	SeekerRowCont float(14,6),
	SeekerColCont float(14,6), 
	d_sh float(14,6),
	d_sb float(14,6),
	d_hb float(14,6), 
	d_shEuc float(14,6),
	SeekerBeliefScore float(14,6),
	HiderRowContWNoise float(14,6),
	HiderColContWNoise float(14,6)
) RETURNS bigint(20)
BEGIN    
    INSERT INTO GameLine (
		GameID,
		ActionNum,
		SentTimeStamp,
		HiderTimeStamp,
		SeekerTimeStamp,
		HiderAction,
		SeekerAction,
		HiderRow,
		HiderCol,
		SeekerRow,
		SeekerCol,
		status,
		HiderRowCont,
		HiderColCont,
		SeekerRowCont,
		SeekerColCont,
		d_sh, 
		d_sb, 
		d_hb, 
		d_shEuc,
		SeekerBeliefScore,
		HiderRowContWNoise,
		HiderColContWNoise
	) VALUES (
		GameID,
		ActionNum,
		SentTimeStamp,
		HiderTimeStamp,
		SeekerTimeStamp,
		HiderAction,
		SeekerAction,
		HiderRow,
		HiderCol,
		SeekerRow,
		SeekerCol,
		status,
		HiderRowCont,
		HiderColCont,
		SeekerRowCont,
		SeekerColCont, 
		d_sh, 
		d_sb, 
		d_hb, 
		d_shEuc,
		SeekerBeliefScore,
		HiderRowContWNoise,
		HiderColContWNoise
	);
    RETURN LAST_INSERT_ID(); 
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `MicroTimestampDiff` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` FUNCTION `MicroTimestampDiff`(
t1 datetime(3),
t2 datetime(3)
) RETURNS bigint(20)
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2) ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `MicroTimestampDiffInS` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` FUNCTION `MicroTimestampDiffInS`(
t1 datetime(3),
t2 datetime(3)
) RETURNS double
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2)/1000000.0 ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `StartServer` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` FUNCTION `StartServer`(
) RETURNS int(11)
BEGIN
    INSERT INTO ServerStart (
		StartTime
	) VALUES (
		NOW(6)
	);
    RETURN LAST_INSERT_ID(); 
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `StopGame` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` FUNCTION `StopGame`(
	GameID INT
) RETURNS int(11)
BEGIN
    UPDATE Game 
    SET EndTime=NOW(6)
    WHERE id=GameID;

    RETURN GameID;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `StopServer` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` FUNCTION `StopServer`(
	sessionid INT
) RETURNS int(11)
BEGIN
    UPDATE ServerStart 
    SET StopTime=NOW(6)
    WHERE id=sessionid;

    RETURN sessionid;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `ClearNotFinishedGames` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `ClearNotFinishedGames`()
begin
delete from GameLine where GameID in (
select GameID from
GameList where WinStatus=0);

delete from Game
where id in (select GameID from
GameList where WinStatus=0);
end ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Final view structure for view `GameList`
--

/*!50001 DROP TABLE IF EXISTS `GameList`*/;
/*!50001 DROP VIEW IF EXISTS `GameList`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `GameList` AS select `GameLine`.`GameID` AS `GameID`,max(`GameLine`.`status`) AS `WinStatus`,max(`GameLine`.`ActionNum`) AS `NumActions`,(`MicroTimestampDiff`(min(`GameLine`.`SeekerTimeStamp`),max(`GameLine`.`SeekerTimeStamp`)) / 1000000) AS `GameDuration` from `GameLine` group by `GameLine`.`GameID` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `GameListMapAndPlayers`
--

/*!50001 DROP TABLE IF EXISTS `GameListMapAndPlayers`*/;
/*!50001 DROP VIEW IF EXISTS `GameListMapAndPlayers`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `GameListMapAndPlayers` AS select distinct `Game`.`MapName` AS `MapName`,`Game`.`SeekerUserID` AS `SeekerUserID`,`Game`.`HiderUserID` AS `HiderUserID` from `Game` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `GameListWinCounter`
--

/*!50001 DROP TABLE IF EXISTS `GameListWinCounter`*/;
/*!50001 DROP VIEW IF EXISTS `GameListWinCounter`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `GameListWinCounter` AS select `g`.`MapName` AS `MapName`,`g`.`SeekerUserID` AS `SeekerUserID`,`g`.`HiderUserID` AS `HiderUserID`,coalesce(`gcw`.`n`,0) AS `NumWin`,coalesce(`gcl`.`n`,0) AS `NumLose`,coalesce(`gct`.`n`,0) AS `NumTie`,`gcw`.`AvgGameDurationPerStep` AS `AvgGameDurationPerStepWin`,`gcl`.`AvgGameDurationPerStep` AS `AvgGameDurationPerStepLose`,`gct`.`AvgGameDurationPerStep` AS `AvgGameDurationPerStepTie`,`gcw`.`AvgNumActions` AS `AvgNumActionsWin`,`gcl`.`AvgNumActions` AS `AvgNumActionsLose`,`gct`.`AvgNumActions` AS `AvgNumActionsTie` from (((`GameListMapAndPlayers` `g` left join `GameListWinStateCounter` `gcw` on(((`g`.`MapName` = `gcw`.`MapName`) and (`g`.`SeekerUserID` = `gcw`.`SeekerUserID`) and (`g`.`HiderUserID` = `gcw`.`HiderUserID`) and (`gcw`.`WinStatus` = 1)))) left join `GameListWinStateCounter` `gcl` on(((`g`.`MapName` = `gcl`.`MapName`) and (`g`.`SeekerUserID` = `gcl`.`SeekerUserID`) and (`g`.`HiderUserID` = `gcl`.`HiderUserID`) and (`gcl`.`WinStatus` = 2)))) left join `GameListWinStateCounter` `gct` on(((`g`.`MapName` = `gct`.`MapName`) and (`g`.`SeekerUserID` = `gct`.`SeekerUserID`) and (`g`.`HiderUserID` = `gct`.`HiderUserID`) and (`gct`.`WinStatus` = 3)))) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `GameListWinPerc`
--

/*!50001 DROP TABLE IF EXISTS `GameListWinPerc`*/;
/*!50001 DROP VIEW IF EXISTS `GameListWinPerc`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `GameListWinPerc` AS select `gl`.`MapName` AS `MapName`,`gl`.`SeekerUserID` AS `SeekerUserID`,`gl`.`HiderUserID` AS `HiderUserID`,`gl`.`NumWin` AS `NumWin`,`gl`.`NumLose` AS `NumLose`,`gl`.`NumTie` AS `NumTie`,`gl`.`AvgGameDurationPerStepWin` AS `AvgGameDurationPerStepWin`,`gl`.`AvgGameDurationPerStepLose` AS `AvgGameDurationPerStepLose`,`gl`.`AvgGameDurationPerStepTie` AS `AvgGameDurationPerStepTie`,`gl`.`AvgNumActionsWin` AS `AvgNumActionsWin`,`gl`.`AvgNumActionsLose` AS `AvgNumActionsLose`,`gl`.`AvgNumActionsTie` AS `AvgNumActionsTie`,`us`.`name` AS `Seeker`,`uh`.`name` AS `Hider`,((`gl`.`NumWin` + `gl`.`NumLose`) + `gl`.`NumTie`) AS `NumTotal`,((100.0 * `gl`.`NumWin`) / ((`gl`.`NumWin` + `gl`.`NumLose`) + `gl`.`NumTie`)) AS `WinPerc`,((100.0 * `gl`.`NumLose`) / ((`gl`.`NumWin` + `gl`.`NumLose`) + `gl`.`NumTie`)) AS `LosePerc`,((100.0 * `gl`.`NumTie`) / ((`gl`.`NumWin` + `gl`.`NumLose`) + `gl`.`NumTie`)) AS `TiePerc` from ((`GameListWinCounter` `gl` left join `User` `us` on((`gl`.`SeekerUserID` = `us`.`id`))) left join `User` `uh` on((`gl`.`HiderUserID` = `uh`.`id`))) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `GameListWinStateCounter`
--

/*!50001 DROP TABLE IF EXISTS `GameListWinStateCounter`*/;
/*!50001 DROP VIEW IF EXISTS `GameListWinStateCounter`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `GameListWinStateCounter` AS select `gd`.`MapName` AS `MapName`,`gd`.`SeekerUserID` AS `SeekerUserID`,`gd`.`HiderUserID` AS `HiderUserID`,`gl`.`WinStatus` AS `WinStatus`,count(0) AS `n`,avg(`gl`.`GameDuration`) AS `AvgGameDuration`,((1.0 * avg(`gl`.`GameDuration`)) / count(0)) AS `AvgGameDurationPerStep`,avg(`gl`.`NumActions`) AS `AvgNumActions` from (`Game` `gd` join `GameList` `gl` on((`gd`.`id` = `gl`.`GameID`))) group by `gd`.`MapName`,`gd`.`SeekerUserID`,`gd`.`HiderUserID`,`gl`.`WinStatus` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `GameSteps`
--

/*!50001 DROP TABLE IF EXISTS `GameSteps`*/;
/*!50001 DROP VIEW IF EXISTS `GameSteps`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `GameSteps` AS select `GameLine`.`id` AS `id`,`GameLine`.`GameID` AS `GameID`,`GameLine`.`ActionNum` AS `ActionNum`,`GameLine`.`SentTimeStamp` AS `SentTimeStamp`,`GameLine`.`HiderTimeStamp` AS `HiderTimeStamp`,`GameLine`.`SeekerTimeStamp` AS `SeekerTimeStamp`,`GameLine`.`HiderAction` AS `HiderAction`,`GameLine`.`SeekerAction` AS `SeekerAction`,`GameLine`.`HiderRow` AS `HiderRow`,`GameLine`.`HiderCol` AS `HiderCol`,`GameLine`.`SeekerRow` AS `SeekerRow`,`GameLine`.`SeekerCol` AS `SeekerCol`,`GameLine`.`status` AS `status`,`MicroTimestampDiffInS`(`GameLine`.`SentTimeStamp`,`GameLine`.`HiderTimeStamp`) AS `HiderStepTime`,`MicroTimestampDiffInS`(`GameLine`.`SentTimeStamp`,`GameLine`.`SeekerTimeStamp`) AS `SeekerStepTime` from `GameLine` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-06-07  1:54:55
