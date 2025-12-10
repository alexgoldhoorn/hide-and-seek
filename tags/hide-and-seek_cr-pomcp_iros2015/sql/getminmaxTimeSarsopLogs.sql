select min(Time),max(Time)
from SarsopLog sl left join SarsopLogTrial st on sl.id=st.FileID
left join SarsopLogTrialRow sr on sr.TrialID=st.id
where Info like '%2c%'

