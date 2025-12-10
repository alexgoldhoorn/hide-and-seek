
-- GRANT ALL PRIVILEGES ON hsgamelog2015_2r.* TO hsserver@'%' IDENTIFIED BY 'hsserver_us3r_p@ss' WITH GRANT OPTION;


delimiter $$

-- truncate TABLE `RobotLog`
CREATE TABLE `RobotLog` (
	id int(11) NOT NULL AUTO_INCREMENT,
	`ImportedTime` datetime,
	FileName varchar(255),
	Info varchar(255),
	`Name` varchar(255),
	PRIMARY KEY (`id`)
)

$$

-- drop TABLE `RobotLogRow`
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
	distance double,
	new_seeker_x double,
	new_seeker_y double,
	new_seeker_orient double,
	new_seeker_row double,
	new_seeker_col double,
	win_state int,PRIMARY KEY (`id`)
)

$$

CREATE TABLE `RobotLogMap` (
	id int NOT NULL AUTO_INCREMENT,
	map varchar(64),
	name_cat varchar(64),PRIMARY KEY (`id`)
);

insert into RobotLogMap (map,name_cat) values ('BRL','brl2_140530');
insert into RobotLogMap (map,name_cat) values ('FME','fme3_140519_ok');
insert into RobotLogMap (map,name_cat) values ('FME','fme4_140520_ok');
insert into RobotLogMap (map,name_cat) values ('UPCN','master3_140602');
insert into RobotLogMap (map,name_cat) values ('UPCN','master4_140603');

drop view if exists RobotLogExt;
create view RobotLogExt as
select rl.*,m.map
from RobotLog rl left join RobotLogMap m on rl.Info=m.name_cat;

-- dist
create or replace view RobotLogRowDist as
select r1.LogId, r1.id as p1ID, r2.id as p2ID, r1.Time as p1Time, r2.Time as p2Time, 
	DistanceEuc(r1.seeker_x,r1.seeker_y,r2.seeker_x,r2.seeker_y) as distDiff_m,
    -- DistanceEuc(r1.seeker_row,r1.seeker_col,r2.seeker_row,r2.seeker_col) as distDiff,
	MicroTimestampDiffInS(r1.Time,r2.Time) as timeDiff
from RobotLogRow r1 inner join RobotLogRow r2 on r1.LogID=r2.LogID and r2.id=r1.id+1;
-- AG151127: now using DistanceEuc function which is same as:
-- sqrt(power(r1.seeker_x-r2.seeker_x,2)+power(r1.seeker_y-r2.seeker_y,2)) as distDiff,


-- count number hidden and total
create or replace view RobotLogStats as
select LogID, count(*) as n, sum(isHidden) as hiddenN, sum(isHidden)/count(*) as hiddenPerc, 
	MicroTimestampDiffInS(min(`Time`),max(`Time`)) AS `Duration_s`, 
	avg(distance) as avgDistanceToHider, std(distance) as stdDistanceToHider	
from RobotLogRowExt
where win_state>=0
group by LogID;

-- extra row information: hiddne or not, and eucl. distance seeker-hider (x,y)
create or replace view RobotLogRowExt as
select *, case when hider_row is null then 1 else 0 end as isHidden, -- was hider_id=-1
	DistanceEuc( seeker_x,seeker_y,hider_x,hider_y) distEuc_m
    -- DistanceEuc( seeker_row,seeker_col,hider_row,hider_col) distEuc
from RobotLogRow;



-- person pos
create view RobotLogRowDistPerson as
    select 
        `r1`.`LogID` AS `LogId`,
        `r1`.`id` AS `p1ID`,
        `r2`.`id` AS `p2ID`,
        `r1`.`Time` AS `p1Time`,
        `r2`.`Time` AS `p2Time`,
        sqrt((pow((`r1`.`hider_x` - `r2`.`hider_x`), 2) + 
			pow((`r1`.`hider_y` - `r2`.`hider_y`), 2))) AS `distDiff`,
        MicroTimestampDiffInS(`r1`.`Time`, `r2`.`Time`) AS `timeDiff`
    from
        (`RobotLogRow` `r1`
        join `RobotLogRow` `r2` ON 
			( (`r1`.`LogID` = `r2`.`LogID`)
				and (`r2`.`id` = (`r1`.`id` + 1))
				and (not r1.hider_x is null) and (not r2.hider_x is null)
			)
		)
