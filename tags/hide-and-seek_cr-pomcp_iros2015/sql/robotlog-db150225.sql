

-- truncate TABLE `RobotLog`
CREATE TABLE `RobotLog` (
	id int(11) NOT NULL AUTO_INCREMENT,
	`ImportedTime` datetime,
	FileName varchar(255),
	Info varchar(255),
	`Name` varchar(255),
	PRIMARY KEY (`id`)
);



-- drop TABLE `RobotLogRow`		TODO: REVIEW (in Libreoffice)
CREATE TABLE `RobotLogRow` (
	id int NOT NULL AUTO_INCREMENT,
	LogID int,
	`Time` datetime(3),
	exp_name varchar(64),
	seeker_x double,
	seeker_y double,
	seeker_orient double,
	seeker_row double,
	seeker_col double,
	hider_visible bool,
	hider_x double,
	hider_y double,
	hider_row double,
	hider_col double,
	hider_id int,
    seeker2_x double,
	seeker2_y double,	
	seeker2_row double,
	seeker2_col double,	
	hidero2_x double,
	hidero2_y double,
	hidero2_row double,
	hidero2_col double,    
	d_sh double,
    d_sh2 double,
    d_s2h double,
    d_s2h2 double,
    d_ss2 double,
	win_state int,
    new_seeker1_froms1_x double,
new_seeker1_froms1_y double,
new_seeker1_froms1_row double,
new_seeker1_froms1_col double,
new_seeker2_froms1_x double,
new_seeker2_froms1_y double,
new_seeker2_froms1_row double,
new_seeker2_froms1_col double,
new_seeker1_froms1_belief double,
new_seeker2_froms1_belief double,
new_seeker1_froms2_x double,
new_seeker1_froms2_y double,
new_seeker1_froms2_row double,
new_seeker1_froms2_col double,
new_seeker2_froms2_x double,
new_seeker2_froms2_y double,
new_seeker2_froms2_row double,
new_seeker2_froms2_col double,
new_seeker1_froms2_belief double,
new_seeker2_froms2_belief double,
new_seeker1_goal_x double,
new_seeker1_goal_y double,
new_seeker1_goal_orient double,
new_seeker1_goal_row double,
new_seeker1_goal_col double,    
    PRIMARY KEY (`id`)
);

$$

CREATE TABLE `RobotLogMap` (
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

-- dist
create view RobotLogRowDist as
select r1.LogId, r1.id as p1ID, r2.id as p2ID, r1.Time as p1Time, r2.Time as p2Time, 
	sqrt(power(r1.seeker_x-r2.seeker_x,2)+power(r1.seeker_y-r2.seeker_y,2)) as distDiff,
	MicroTimestampDiffInS(r1.Time,r2.Time) as timeDiff
from RobotLogRow r1 inner join RobotLogRow r2 on r1.LogID=r2.LogID and r2.id=r1.id+1


-- count number hidden and total
create view RobotLogStats as
select LogID, count(*) as n, sum(isHidden) as hiddenN, sum(isHidden)/count(*) as hiddenPerc, 
	MicroTimestampDiffInS(min(`Time`),max(`Time`)) AS `Duration_s`, 
	avg(distance) as avgDistanceToHider, std(distance) as stdDistanceToHider	
from RobotLogRowExt
where win_state>=0
group by LogID;


create view RobotLogRowExt as
select *, case when hider_id=-1 then 1 else 0 end as isHidden  from RobotLogRow	;

CREATE DEFINER=`root`@`localhost` FUNCTION `MicroTimestampDiffInS`(
t1 datetime(3),
t2 datetime(3)
) RETURNS double
    DETERMINISTIC
return TIMESTAMPDIFF(microsecond, t1,t2)/1000000.0

-- person pos
drop view if exists RobotLogRowDistPerson;
create view RobotLogRowDistPerson as
    select 
        `r1`.`LogID` AS `LogId`,
        `r1`.`id` AS `p1ID`,
        `r2`.`id` AS `p2ID`,
        `r1`.`Time` AS `p1Time`,
        `r2`.`Time` AS `p2Time`,
        sqrt((pow((`r1`.`hider_x` - `r2`.`hider_x`), 2) + 
			pow((`r1`.`hider_y` - `r2`.`hider_y`), 2))) AS `distDiffH1`,
        sqrt((pow((`r1`.`hidero2_x` - `r2`.`hidero2_x`), 2) + 
			pow((`r1`.`hidero2_y` - `r2`.`hidero2_y`), 2))) AS `distDiffH2`,
        sqrt((pow((`r1`.`seeker_x` - `r2`.`seeker_x`), 2) + 
			pow((`r1`.`seeker_y` - `r2`.`seeker_y`), 2))) AS `distDiffS1`,
        sqrt((pow((`r1`.`seeker2_x` - `r2`.`seeker2_x`), 2) + 
			pow((`r1`.`seeker2_y` - `r2`.`seeker2_y`), 2))) AS `distDiffS2`,
        MicroTimestampDiffInS(`r1`.`Time`, `r2`.`Time`) AS `timeDiff`
    from
        (`RobotLogRow` `r1`
        join `RobotLogRow` `r2` ON 
			( (`r1`.`LogID` = `r2`.`LogID`)
				and (`r2`.`id` = (`r1`.`id` + 1))
				and (not r1.hider_x is null) and (not r2.hider_x is null)
			)
		);

-- drop view if exists RobotLogSumm;
create view RobotLogSumm as
select LogID,count(*) numRows, MicroTimestampDiffInS(min(Time),max(Time)) AS TimeS,
	avg(d_sh)  as avg_d_sh, std(d_sh)  as std_d_sh,
	avg(d_sh2)  as avg_d_sh2, std(d_sh2)  as std_d_sh2,
	avg(d_s2h)  as avg_d_s2h, std(d_s2h)  as std_d_s2h,
	avg(d_s2h2)  as avg_d_s2h2, std(d_s2h2)  as std_d_s2h2,
	avg(d_ss2)  as avg_d_ss2, std(d_ss2)  as std_d_ss2,
    sum(case when not hider_x is null or not hidero2_x is null then 1 else 0 end) as numVisib,
    sum(case when not seeker2_x is null then 1 else 0 end) as numConn,
    count(*) n,
    abs(sum(win_state)) as NumGames
from RobotLogRow group by LogID;
  



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

RobotLogRowDistPerson


-- for dist pers + hider
select Robot, sum(NumGames) as NumGames, sum(numRows) as TotNumRows, sum(TimeS) as TotTimeS,
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


-- avg d_sh
select 	avg(d_sh)  as avg_d_sh, std(d_sh)  as std_d_sh,
	avg(d_sh2)  as avg_d_sh2, std(d_sh2)  as std_d_sh2,
	avg(d_s2h)  as avg_d_s2h, std(d_s2h)  as std_d_s2h,
	avg(d_s2h2)  as avg_d_s2h2, std(d_s2h2)  as std_d_s2h2,
avg( case when d_s2h<d_sh then d_s2h else d_sh end ) avg_min_d_ss2h,
std( case when d_s2h<d_sh then d_s2h else d_sh end ) std_min_d_ss2h
from RobotLog l left join RobotLogCat c on l.Info=c.Info
	left join RobotLogRow r on l.id=r.LogID
    where Test=False;


select  sum(NumGames) as NumGames, sum(numRows) as TotNumRows, sum(TimeS) as TotTimeS,
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

