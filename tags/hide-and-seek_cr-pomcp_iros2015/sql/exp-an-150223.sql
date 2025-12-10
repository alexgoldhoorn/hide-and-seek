
-- summary data
select 'MapName', 'SolverTypeName', 'Seeker',
	'AutoWalkerN', 'NumGames', 'avg_actionNum',
	'avg_d_sh', 'std_d_sh',
    'avg_d_shEuc', 'std_d_shEuc',
	'avg_d_s2h', 'std_d_s2h',
    'avg_d_s2hEuc', 'std_d_s2hEuc',
    'avg_d_sh_min', 'std_d_sh_min',
	'avg_d_shEuc_min', 'std_d_shEuc_min',
    'avg_d_ss2', 'std_d_ss2',
    'avg_d_ss2Euc', 'std_d_ss2Euc',
    'IsVis_sh_Perc', 'IsVis_s2h_Perc', 
    'IsVis_anysh_Perc',
    'IsVis_ss2_Perc', 
    'isClosePerc', 'isClose2Perc', 'isClose3Perc',
    'avg_FirstStepHVisible', 'std_FirstStepHVisible', 
    'avg_FirstStepCloseToH', 'std_FirstStepCloseToH',    
    'avg_SeekerDuration_ms', 'std_SeekerDuration_ms',
    'avg_Seeker2Duration_ms', 'std_Seeker2Duration_ms',
    'avg_HiderDuration_ms', 'std_HiderDuration_ms',
    'avg_SeekerBeliefScore', 'std_SeekerBeliefScore',
    'avg_Seeker2BeliefScore', 'std_Seeker2BeliefScore'
union all
select MapName, SolverTypeName, u.Name as Seeker, -- , uh.Name as Hider, us2.Name as Seeker2,
	autoWalkerN, count(distinct g.id) NumGames, avg(ActionNum) avg_actionNum,
	avg(d_sh) avg_d_sh, std(d_sh) std_d_sh,
    avg(d_shEuc) avg_d_shEuc, std(d_shEuc) std_d_shEuc,
	avg(d_s2h) avg_d_s2h, std(d_s2h) std_d_s2h,
    avg(d_s2hEuc) avg_d_s2hEuc, std(d_s2hEuc) std_d_s2hEuc,
    avg(d_sh_min) as avg_d_sh_min, std(d_sh_min) as std_d_sh_min,
	avg(d_shEuc_min) as avg_d_shEuc_min, std(d_shEuc_min) as std_d_shEuc_min,
    avg(d_ss2) avg_d_ss2, std(d_s2h) std_d_ss2,
    avg(d_ss2Euc) avg_d_ss2Euc, std(d_ss2Euc) std_d_ss2Euc,
    sum(vis_sh)/count(*) as IsVis_sh_Perc, sum(vis_s2h)/count(*) as IsVis_s2h_Perc, 
    sum(vis_anysh)/count(*) as IsVis_anysh_Perc,
    sum(vis_ss2)/count(*) as IsVis_ss2_Perc, 
    sum(isClose)/count(*) as isClosePerc, sum(isClose2)/count(*) as isClose2Perc, sum(isClose3)/count(*) as isClose3Perc,    
    avg(FirstStepHVisible) as avg_FirstStepHVisible, std(FirstStepHVisible) as std_FirstStepHVisible, 
    avg(FirstStepCloseToH) as avg_FirstStepCloseToH, std(FirstStepCloseToH) as std_FirstStepCloseToH,    
    avg(SeekerDuration_ms) as avg_SeekerDuration_ms, std(SeekerDuration_ms) as std_SeekerDuration_ms,
    avg(Seeker2Duration_ms) as avg_Seeker2Duration_ms, std(Seeker2Duration_ms) as std_Seeker2Duration_ms,
    avg(HiderDuration_ms) as avg_HiderDuration_ms, std(HiderDuration_ms) as std_HiderDuration_ms,
    avg(SeekerBeliefScore) as avg_SeekerBeliefScore, std(SeekerBeliefScore) as std_SeekerBeliefScore,
    avg(Seeker2BeliefScore) as avg_Seeker2BeliefScore, std(Seeker2BeliefScore) as std_Seeker2BeliefScore
    
from GameExt g left join GameLineExt l on g.id=l.GameID
		left join GameLineFirstHVisib v on g.id=v.GameID
        left join GameLineFirstCloseToH c on g.id=c.GameID
        left join User u on g.SeekerUserID=u.id
        /* left join User uh on g.HiderUserID=uh.id
        left join User us2 on g.Seeker2UserID=us2.id */
where u.Name like 'multi4%' and not u.Name like '%_s2' -- SeekerUserID in (12,13,14)
group by MapName, SolverTypeName,AutoWalkerN,u.Name -- ,uh.Name,us2.Name;
order by MapName, SolverTypeName,u.Name,AutoWalkerN
into outfile '/tmp/anly_gamesumm150226.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

-- extra game data
select 'GameID', 'MapName', 'SolverTypeName', 'Seeker', 'Hider', 'Seeker2',
	'AutoWalkerN', 'DurationS','FirstStepCloseToH','FirstStepHVisible'
union all
select g.id as GameID, MapName, SolverTypeName, u.Name as Seeker, uh.Name as Hider, us2.Name as Seeker2,
	autoWalkerN, DurationS,FirstStepCloseToH,FirstStepHVisible    
from GameExt g 
		left join GameLineFirstHVisib v on g.id=v.GameID
        left join GameLineFirstCloseToH c on g.id=c.GameID
        left join User u on g.SeekerUserID=u.id
        left join User uh on g.HiderUserID=uh.id
        left join User us2 on g.Seeker2UserID=us2.id 
where u.Name like 'multi4%' -- SeekerUserID in (12,13,14)
into outfile '/tmp/anly_gameinfo150225.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';


-- all data
select 'GameID', 'LineID', 'MapName', 'SolverTypeName', 'Seeker', 'Hider', 'Seeker2',
	'AutoWalkerN', 'ActionNum', 'status',
    'd_sh','d_shEuc','d_s2h','d_s2hEuc', 'd_s2hEuc', 
    'd_sh_min', 'd_shEuc_min',
	'd_ss2', 'd_ss2Euc', 'vis_sh', 'vis_s2h', 'vis_ss2', 
    'isClose', 'isClose2', 'isClose3',
    'SeekerBeliefScore', 'Seeker2BeliefScore',
    'SeekerDuration_ms','HiderDuration_ms','Seeker2Duration_ms'	
union all
select GameID, l.id as LineID, MapName, SolverTypeName, u.Name as Seeker, uh.Name as Hider, us2.Name as Seeker2,
	autoWalkerN, ActionNum,status,
    d_sh,d_shEuc,d_s2h,d_s2hEuc, d_s2hEuc, 
    d_sh_min, d_shEuc_min,
	d_ss2, d_ss2Euc, vis_sh, vis_s2h,vis_ss2, 
    isClose, isClose2, isClose3,
    SeekerBeliefScore, Seeker2BeliefScore,
    SeekerDuration_ms,HiderDuration_ms,Seeker2Duration_ms	
from GameExt g left join GameLineExt l on g.id=l.GameID
        left join User u on g.SeekerUserID=u.id
        left join User uh on g.HiderUserID=uh.id
        left join User us2 on g.Seeker2UserID=us2.id 
where u.Name like 'multi4%' -- SeekerUserID in (12,13,14)
into outfile '/tmp/anly_all150225.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';



select MapName, SolverTypeName, AutoWalkerN, count(*) 
from GameExt
where SeekerUserID in (12,13,14)
group by MapName, SolverTypeName,AutoWalkerN;

select * from User

		
select *
from GameExt -- left join 
where SeekerUserID in (12,13,14) -- (33,34)



select id from User where Name like 'multi2%';


-- first time visibile
select GameID, min(ActionNum)
from GameLine
where vis_sh=1 or vis_s2h=1
group by GameID

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



-----



select g.id, MapName,SolverTypeName,u.Name as Seeker,uh.Name as Hider,us2.Name as Seeker2,
	autoWalkerN,count(distinct g.id) NumGames,avg(ActionNum) avg_actionNum,
	avg(d_shEuc) avg_d_shEuc, std(d_shEuc) std_d_shEuc,
	avg(d_s2hEuc) avg_d_s2hEuc, std(d_s2hEuc) std_d_s2hEuc,
    avg(d_sh_min) as avg_d_sh_min, std(d_sh_min) as std_d_sh_min,
	avg(d_shEuc_min) as avg_d_shEuc_min, std(d_shEuc_min) as std_d_shEuc_min,
    sum(isClose)/count(*) as isClosePerc, sum(isClose3)/count(*) as isClose3Perc,
    avg(FirstStepHVisible) as avg_FirstStepHVisible, std(FirstStepHVisible) as std_FirstStepHVisible, 
    avg(FirstStepCloseToH) as avg_FirstStepCloseToH, std(FirstStepCloseToH) as std_FirstStepCloseToH
from GameExt g left join GameLineExt l on g.id=l.GameID
		left join GameLineFirstHVisib v on g.id=v.GameID
        left join GameLineFirstCloseToH c on g.id=c.GameID
        left join User u on g.SeekerUserID=u.id
        left join User uh on g.HiderUserID=uh.id
        left join User us2 on g.Seeker2UserID=us2.id
where g.id = (select max(id) from Game)  -- Name like 'multi2%' -- SeekerUserID in (12,13,14)
group by MapName, SolverTypeName,AutoWalkerN,u.Name,uh.Name,us2.Name;


select * from GameExt order by id desc;
select * from Game order by id desc;

select * from User;

select * from ServerStart order by id desc;
