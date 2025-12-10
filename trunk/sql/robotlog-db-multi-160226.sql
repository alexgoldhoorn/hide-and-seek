
-- CREATE tables --
-- drop TABLE if exists `RobotLog`;
CREATE TABLE `RobotLog` (
	id int(11) NOT NULL AUTO_INCREMENT,
	`ImportedTime` datetime,
	FileName varchar(255),
	Info varchar(255),
	`Name` varchar(255),
	PRIMARY KEY (`id`)
);

-- drop TABLE if exists `RobotLogRow`;
CREATE TABLE `RobotLogRow` (
    id int NOT NULL AUTO_INCREMENT,
    LogID int,
    `Time` datetime(3),
    step_num int,
    exp_name varchar(64),

    seeker1_x double,
    seeker1_y double,
    hiderO1_x double,
    hiderO1_y double,

    seeker2_x double,
    seeker2_y double,
    hiderO2_x double,
    hiderO2_y double,

    seeker1_row double,
    seeker1_col double,
    hiderO1_row double,
    hiderO1_col double,
    hiderO1_p double,

    seeker2_row double,
    seeker2_col double,
    hiderO2_row double,
    hiderO2_col double,
    hiderO2_p double,

    hiderChosen_row double,
    hiderChosen_col double,

    d_sh double,
    d_shEuc double,
    d_s2h double,
    d_s2hEuc double,
    d_ss2 double,
    d_ss2Euc double,
    win_state int,

    seeker1_goal_x double,
    seeker1_goal_y double,
    seeker1_goal_or double,
    seeker1_goal_row double,
    seeker1_goal_col double,
    seeker2_goal_row double,
    seeker2_goal_col double,
    seeker1_numGoals_HBs double,
    seeker2_numGoals_HBs double,
    seeker_reward double,

    PRIMARY KEY (`id`),
    FOREIGN KEY (LogID) REFERENCES RobotLog(id)
);


-- add isHidden and HasConnection
create or replace view RobotLogRowExt as
select *,
    case when hiderO1_x is null then 0 else 1 end as is1Visible,
    case when hiderO1_x is null and hiderO2_x is null then 0 else 1 end as isVisible,
    case when seeker2_x is null then 0 else 1 end as HasConnection,
    case when hiderO1_x is null then hiderO2_x else hiderO1_x end as hiderOJ_x,
    case when hiderO1_y is null then hiderO2_y else hiderO1_y end as hiderOJ_y
from RobotLogRow;


-- View for distance travelled dist for robot (own)
create or replace view RobotLogRowDistRobot as
select r1.LogId, r1.id as p1ID, r2.id as p2ID, r1.Time as p1Time, r2.Time as p2Time, 
	sqrt(power(r1.seeker1_x-r2.seeker1_x,2)+power(r1.seeker1_y-r2.seeker1_y,2)) as robotDistDiff,
	MicroTimestampDiffInS(r1.Time,r2.Time) as robotTimeDiff
from RobotLogRow r1 inner join RobotLogRow r2 on r1.LogID=r2.LogID 
        and r2.step_num=r1.step_num+1;
-- and r2.id=r1.id+1

-- distance travelled for person
CREATE OR REPLACE VIEW RobotLogRowDistPerson as
select r1.LogId,r1.id as p1ID, r2.id as p2ID, r1.Time as p1Time, r2.Time as p2Time, 
	sqrt(power(r1.hiderO1_x-r2.hiderO1_x,2)+power(r1.hiderO1_y-r2.hiderO1_y,2)) as person1DistDiff,
	sqrt(power(r1.hiderOJ_x-r2.hiderOJ_x,2)+power(r1.hiderOJ_y-r2.hiderOJ_y,2)) as personJDistDiff,
	MicroTimestampDiffInS(r1.Time,r2.Time) as personTimeDiff
from RobotLogRowExt r1 inner join RobotLogRowExt r2 on r1.LogID=r2.LogID 
        and r2.step_num=r1.step_num+1;
        
/*select r1.LogId, r1.id as p1ID, r2.id as p2ID, r1.Time as p1Time, r2.Time as p2Time, 
	sqrt(power(r1.seeker1_x-r2.seeker1_x,2)+power(r1.seeker1_y-r2.seeker1_y,2)) as personDistDiff,
    from
        (`RobotLogRow` `r1`
        join `RobotLogRow` `r2` ON 
			( (`r1`.`LogID` = `r2`.`LogID`)
				and (r2.step_num=(r1.step_num+1) )
				and (not r1.hider1_x is null) and (not r2.hider_x is null)
			)
		);*/

-- now per robot travelled distance of the robot
CREATE OR REPLACE VIEW `RobotLogDistRobotPerRobot` AS
select Name, sum(robotDistDiff) as robotDist_m, sum(robotTimeDiff) as robotTime_s
from RobotLog l left join RobotLogRowDistRobot ld on l.id=ld.LogID
group by Name;
-- now per robot travelled distance of the person
CREATE OR REPLACE VIEW `RobotLogDistPersonPerRobot` AS
select Name, sum(person1DistDiff) as person1Dist_m, sum(personJDistDiff) as personJDist_m, sum(personTimeDiff) as personTime_s
from RobotLog l left join RobotLogRowDistPerson ld on l.id=ld.LogID
group by Name;


-- summary per log id
create or replace view RobotLogSumm as
select LogID,count(*) numRows, MicroTimestampDiffInS(min(Time),max(Time)) AS TimeS,
	avg(d_sh)  as avg_d_sh, std(d_sh)  as std_d_sh,
	avg(d_shEuc)  as avg_d_shEuc, std(d_shEuc)  as std_d_shEuc,
	avg(d_s2h)  as avg_d_s2h, std(d_s2h)  as std_d_s2h,
	avg(d_s2hEuc)  as avg_d_s2hEuc, std(d_s2hEuc)  as std_d_s2hEuc,
	avg(d_ss2)  as avg_d_ss2, std(d_ss2)  as std_d_ss2,
	avg(d_ss2Euc)  as avg_d_ss2Euc, std(d_ss2Euc)  as std_d_ss2Euc,
    sum(IsVisible) as numVisible,
    sum(HasConnection) as numConn,
    -- count(*) n,
    abs(sum(win_state)) as NumGames
    -- sum(case when step_num=0 then 1 else 0 end) as NumGames2
from RobotLogRowExt group by LogID;


-- summary per log id, with percentages and robot log meta data
select l.*, rls.*, 1.0*numHidden/numRows hiddenPerc, 1.0*numConn/numRows connPerc
from RobotLog l left join RobotLogSumm s on l.id=s.LogID;

-- steps
select LogID,min(step_num),max(step_num)
from RobotLogRow group by LogID;

-- robot log
select * 
from RobotLog;

-- summary per robot
create or replace view RobotLogSummPerRobot as
select Name, count(*) numRows, 
	avg(d_sh)  as avg_d_sh, std(d_sh)  as std_d_sh,
	avg(d_shEuc)  as avg_d_shEuc, std(d_shEuc)  as std_d_shEuc,
	avg(d_s2h)  as avg_d_s2h, std(d_s2h)  as std_d_s2h,
	avg(d_s2hEuc)  as avg_d_s2hEuc, std(d_s2hEuc)  as std_d_s2hEuc,
	avg(d_ss2)  as avg_d_ss2, std(d_ss2)  as std_d_ss2,
	avg(d_ss2Euc)  as avg_d_ss2Euc, std(d_ss2Euc)  as std_d_ss2Euc,
    sum(Is1Visible) as numVisibleThisRobot,
    sum(IsVisible) as numVisibleJoined,
    sum(HasConnection) as numConn,
    abs(sum(win_state)) as NumGames,
    1.0*sum(Is1Visible)/count(*) as visibleThisRobotPerc,
    1.0*sum(IsVisible)/count(*) as visibleJoinedPerc,
    1.0*sum(HasConnection)/count(*) as connPerc
from RobotLogRowExt rl left join RobotLog l on l.id=rl.LogID
group by Name;

-- show all info per robot
select s.*,dr.*,dp.* 
from RobotLogSummPerRobot s left join RobotLogDistRobotPerRobot dr on s.Name=dr.Name
    left join RobotLogDistPersonPerRobot dp on s.Name=dp.Name;
    
select * from RobotLogDistRobotPerRobot


-- ----- OLD -----
-- create time diff
/* CREATE DEFINER=`root`@`localhost` FUNCTION `MicroTimestampDiffInS`(
t1 datetime(3),
t2 datetime(3)
) RETURNS double
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2)/1000000.0 */

-- person pos

-- count number hidden and total
create view RobotLogStats as
select LogID, count(*) as n, sum(isHidden) as hiddenN, sum(isHidden)/count(*) as hiddenPerc, 
	MicroTimestampDiffInS(min(`Time`),max(`Time`)) AS `Duration_s`, 
	avg(distance) as avgDistanceToHider, std(distance) as stdDistanceToHider	
from RobotLogRowExt
where win_state>=0
group by LogID;


/*CREATE TABLE `RobotLogMap` (
	id int NOT NULL AUTO_INCREMENT,
	map varchar(64),
	name_cat varchar(64),PRIMARY KEY (`id`)
)

insert into RobotLogMap (map,name_cat) values ('BRL','brl2_140530');
insert into RobotLogMap (map,name_cat) values ('FME','fme3_140519_ok');
insert into RobotLogMap (map,name_cat) values ('FME','fme4_140520_ok');
insert into RobotLogMap (map,name_cat) values ('UPCN','master3_140602');
insert into RobotLogMap (map,name_cat) values ('UPCN','master4_140603');

create view RobotLogExt as
select rl.*,m.map
from RobotLog rl left join RobotLogMap m on rl.Info=m.name_cat;
*/

update VIEW `RobotLogCat` AS
    SELECT DISTINCT
        `RobotLog`.`Info` AS `Info`,
        (CASE
            WHEN (`RobotLog`.`Info` LIKE 'dabo%') THEN 'dabo'
            ELSE (CASE
                WHEN (`RobotLog`.`Info` LIKE 'tibi%') THEN 'tibi'
                ELSE '?'
            END)
        END) AS `Robot`,
        (CASE
            WHEN (`RobotLog`.`Info` LIKE '%fme2015_map0%') THEN 'fme2015_map0'
            ELSE (CASE
                WHEN (`RobotLog`.`Info` LIKE '%fme2015_map1%') THEN 'fme2015_map1'
                ELSE (CASE
					WHEN (`RobotLog`.`Info` LIKE '%master29f%') THEN 'master29f'
					ELSE (CASE 
						WHEN (`RobotLog`.`Info` LIKE '%master29e%') THEN 'master29e'
						ELSE (CASE 
							WHEN (`RobotLog`.`Info` LIKE '%brl%') THEN 'brl'
							ELSE '?'
						END)
					END)
				END)
            END)
        END) AS `Map`,
        (CASE
            WHEN (`RobotLog`.`Info` LIKE '%test%') THEN 1
            ELSE 0
        END) AS `Test`
    FROM
        `RobotLog`
        
        $$



select l.*,r.numRows, TimeS,
from RobotLog l left join (
	select LogID,count(*) numRows, MicroTimestampDiffInS(min(Time),max(Time)) AS TimeS,
		avg(d_sh)
    from RobotLogRow group by LogID
) r on l.id=r.LogID
where numRows>0 and FileName like '%tibi%'

select distinct win_state from RobotLogRow;

drop view if exists RobotLogCat;
create view RobotLogCat as 
select distinct Info, 
(case when Info like 'dabo%' then 'dabo' else (case when Info like 'tibi%' then 'tibi' else '?' end) end) as Robot,
(case when Info like '%fme2015_map0%' then 'fme2015_map0' else (case when Info like '%fme2015_map1%' then 'fme2015_map1' else '?' end) end) as Map,
(case when Info like '%test%' then True else False end) as Test
from RobotLog;




select Robot, Map,Test, sum(NumGames) as NumGames, sum(numRows) as TotNumRows, sum(TimeS) as TotTimeS,
	sum(distDiffH1) sum_distDiffH1, sum(distDiffH2) sum_distDiffH2,
    sum(distDiffS1) sum_distDiffS1, sum(distDiffS2) sum_distDiffS2,
    sum(timeDiff) TotTimeDiff, avg(avg_d_sh) avg2_d_sh, avg(avg_d_sh2) avg2_d_sh2, 
    avg(avg_d_s2h) avg2_d_s2h, avg(avg_d_s2h2) avg2_d_s2h2, 
    avg(avg_d_ss2) avg2_d_ss2,
    sum(numVisib) as numVisib, sum(numVisib)/sum(n) visibPerc,
    sum(numConn) as numConn, sum(numConn)/sum(n) connPerc,
    sum(n) as n
from RobotLog l left join RobotLogCat c on l.Info=c.Info
	left join RobotLogRowDistPerson as d on l.id=d.LogID
    left join RobotLogSumm as s on l.id=s.LogID
group by Robot, Map,Test;


select distinct Info from RobotLogCat



-- for dist pers + hider
select Robot,  sum(NumGames) as NumGames, sum(numRows) as TotNumRows, sum(TimeS) as TotTimeS,
	sum(distDiffH1) sum_distDiffH1, sum(distDiffH2) sum_distDiffH2,
    sum(distDiffS1) sum_distDiffS1, sum(distDiffS2) sum_distDiffS2,
    sum(timeDiff) TotTimeDiff,    
    sum(numVisib) as numVisib, sum(numVisib)/sum(n) visibPerc,
    sum(numConn) as numConn, sum(numConn)/sum(n) connPerc,
    sum(n) as n
    -- into outfile '/tmp/mpres.csv'
from RobotLog l left join RobotLogCat c on l.Info=c.Info
	left join RobotLogRowDistPerson as d on l.id=d.LogID
    left join RobotLogSumm as s on l.id=s.LogID 
    where Test=false
group by Robot, Map;


-- avg d_sh
select 	Map, avg(d_sh)  as avg_d_sh, std(d_sh)  as std_d_sh,
	avg(d_sh2)  as avg_d_sh2, std(d_sh2)  as std_d_sh2,
	avg(d_s2h)  as avg_d_s2h, std(d_s2h)  as std_d_s2h,
	avg(d_s2h2)  as avg_d_s2h2, std(d_s2h2)  as std_d_s2h2,
avg( case when d_s2h<d_sh then d_s2h else d_sh end ) avg_min_d_ss2h,
std( case when d_s2h<d_sh then d_s2h else d_sh end ) std_min_d_ss2h
from RobotLog l left join RobotLogCat c on l.Info=c.Info
	left join RobotLogRow r on l.id=r.LogID
    where Test=False
    group by Map;


select  Robot,Map,,sum(NumGames) as NumGames, sum(numRows) as TotNumRows, sum(TimeS) as TotTimeS,
	sum(distDiffH1) sum_distDiffH1, sum(distDiffH2) sum_distDiffH2,
    sum(distDiffS1) sum_distDiffS1, sum(distDiffS2) sum_distDiffS2,
    sum(timeDiff) TotTimeDiff,    
    sum(numVisib) as numVisib, sum(numVisib)/sum(n) visibPerc,
    sum(numConn) as numConn, sum(numConn)/sum(n) connPerc,
    sum(n) as n
from RobotLog l left join RobotLogCat c on l.Info=c.Info
	left join RobotLogRowDistPerson as d on l.id=d.LogID
    left join RobotLogSumm as s on l.id=s.LogID 
group by Robot;


select * from RobotLog;
select * from RobotLogRow;


/*SET FOREIGN_KEY_CHECKS = 0;
truncate table RobotLogRow;
truncate table RobotLog;
SET FOREIGN_KEY_CHECKS = 1;*/

