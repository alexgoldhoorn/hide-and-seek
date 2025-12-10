
# queries to make graphs for H&S analysis #

delimiter $$




$$

# Table 1 / Fig 1a: # games won, online/off-, reward, smart/random, win%
# OnOffline,Hider,Reward,Win State,n
select OnOffline,HiderCat,Reward,WinState,count(*) n
from GameListDetailFiltered
where seeker like '%_fme_w1'
group by OnOffline,HiderCat,Reward,WinState
into outfile '/tmp/anly2_games_won.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Table 1 / Fig 1a: # games won, online/off-, reward, smart/random, win%
# OnOffline,MaxDepthFilter,Hider,Reward,Win State,n
select OnOffLine,MaxDepthFilter,HiderCat,Reward,WinState,count(*) n
from GameListDetailFiltered
where seeker like '%_fme_w1'
group by OnOffLine , MaxDepthFilter,HiderCat,Reward,WinState
into outfile '/tmp/anly2_games_won2.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Table 1 / Fig 1a: # games won, online/off-, reward, smart/random, win%
# OnOffline,MaxDepthFilter,Hider,Reward,Segmentation,Segmentation X,Win State,n
select OnOffLine,MaxDepthFilter,HiderCat,Reward,Segmentation,SegmentationX,WinState,count(*) n
from GameListDetailFiltered
where seeker like '%_fme_w1'
group by OnOffLine , MaxDepthFilter,HiderCat,Reward,WinState,Segmentation,SegmentationX
into outfile '/tmp/anly2_games_won3.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';



$$

# Table 1b: # games won, online/off-, reward, smart/random, win%
# Max Depth,Hider,Reward,Win State,n
select MaxDepthFilter,HiderCat,Reward,WinState,count(*) n
from GameListDetailFiltered
where seeker like '%_fme_w1'
and OnOffline='online'
group by MaxDepthFilter,HiderCat,Reward,WinState
into outfile '/tmp/anly2_gameswon_maxdfil_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 1b: count of game sims per size,reward,on/offline (,Hider)
# Hider,Map Size,Reward,OnOffline,Max Depth Filter,n
select HiderCat, MapSize, Reward, OnOffline, MaxDepthFilter, count(*) n
from GameListDetailFiltered
where seeker like '%_fme_w1'
group by HiderCat, MapSize, Reward, OnOffline, MaxDepthFilter
into outfile '/tmp/anly2_game_count.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'

$$

# ag130219
# OnOffline,Hider,Map Size,Win State,Duration (s),Duration per Action (s),Number of Actions
select OnOffline,HiderCat, MapSize, WinState, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where MaxDepthFilter='no'
into outfile '/tmp/anly2_numact_dur.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';
$$

# ag130219
# OnOffline,Hider,Map Size,Win State,Reward,Segmentation,Segmentation X,Duration (s),Duration per Action (s),Number of Actions
select OnOffline,HiderCat, MapSize, WinState,Reward,Segmentation,SegmentationX, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where seeker like '%_fme_w1'
and MaxDepthFilter='no'
into outfile '/tmp/anly2_numact_dur_nomaxd.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 2a: game durations of offline
# Fig 3b/4: game # actions of offline
# Hider,Map Size,Win State,Duration (s),Duration per Action (s),Number of Actions
select HiderCat, MapSize, WinState, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where seeker like '%_fme_w1'
and OnOffline='offline'
into outfile '/tmp/anly2_game_dur_off.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 2b: game durations of online
# Fig 3b/4: game # actions of online
# Fig 6b: duration per action
# Max Depth,Hider,Map Size,Win State,Duration (s),Duration per Action (s),Number of Actions
select MaxDepthFilter, HiderCat, MapSize, WinState, Duration, DurationPerAction, NumActions
from GameListDetailFiltered
where seeker like '%_fme_w1'
and OnOffline='online'
into outfile '/tmp/anly2_game_dur_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Table 2: avg time per action (online) (median not in mysql, if needed get whole list and use other prog)
# Map Size,Duration Per Action Avg,Duration Per Action Std
select MapSize,avg(DurationPerAction),std(DurationPerAction)
from GameListDetailFiltered
where seeker like '%_fme_w1'
and OnOffline='online' and MaxDepthFilter='no'
group by MapSize
into outfile '/tmp/anly2_timeperaction_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n'

$$

# Fig 5a: win % online
# Fig 7b: # actions avg online
# Hider,Reward,Segmentation,Win State,n,Number of Actions avg,Number of Actions std
select HiderCat,Reward,Segmentation,WinState,count(*) n, avg(NumActions), std(NumActions)
from GameListDetailFiltered
where seeker like '%_fme_w1'
and OnOffline='online' and MaxDepthFilter='no'
group by HiderCat,Reward,WinState,Segmentation
into outfile '/tmp/anly2_numact_win_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 5b: win % offline
# Fig 7a: avg num actions
select HiderCat,WinState,count(*) n, avg(NumActions), std(NumActions)
from GameListDetailFiltered
where seeker like '%_fme_w1'
and OnOffline='offline' and MaxDepthFilter='no'
group by OnOffline,HiderCat,WinState
into outfile '/tmp/anly2_numact_win_off.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Fig 5c: win % online for max depth
# (5d: without seperation for reward,segmentation,..)
# Fig 7b: avg num actions online
# Hider,Reward,Segmentation,Max Depth,Win State,n,Number of Actions avg,Number of Actions std,Duration (s) avg,Duration (s) std,Duration per Action (s) avg,Duration per Action (s) std
select HiderCatShort,Reward,SegmentationShort,MaxDepth,WinState,count(*) n, avg(NumActions), std(NumActions),
	avg(Duration),std(Duration),avg(DurationPerAction),std(DurationPerAction)
from GameListDetailFiltered
where OnOffline='online'  and SegmentationX='none'
group by HiderCat,Reward,WinState,Segmentation,MaxDepth
into outfile '/tmp/anly2_numact_win_maxd_on.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

# Hider,MapSize,Reward,Segmentation,Max Depth,Win State,Number of Actions,Duration (s),Duration per Action (s)
select HiderCatShort,MapSize,Reward,SegmentationShort,MaxDepth,WinState, NumActions, 
	Duration,DurationPerAction
from GameListDetailFiltered
where seeker like '%_fme_w1'
and OnOffline='online' and SegmentationX='none'
-- group by HiderCat,Reward,WinState,Segmentation,MaxDepth
into outfile '/tmp/anly2_numact_win_maxd_on_all.csv'
FIELDS TERMINATED BY ','
LINES TERMINATED BY '\n';

$$

#ag130221: not time limited games
select TimeLimit,WinState,count(*) N from (
	select WinState,
		if(Seeker like '%_rh' or Seeker like '%sh_T','No TL','TL') as TimeLimit
		-- count(*) n
	from GameListDetailFiltered
	where /*Seeker like '%_rh' or Seeker like '%sh_T'
	and*/ onOffline='online' and MaxDepthFilter='no' and SegmentationX='none' and MapSize='10x10'
	-- group by HiderCatShort,MapSize,Reward,SegmentationShort,WinState
) t group by TimeLimit,WinState
$$