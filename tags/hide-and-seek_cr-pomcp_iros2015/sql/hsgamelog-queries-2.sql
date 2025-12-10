DataValue
use urdb;
select * from User;


select * from Experiment;

select * from UserExperiment;


select d.*, f.Name as FieldTypeName from DataType d left join FieldType f on f.id=d.FieldType


    ExperimentID INT,
    UserID INT,
    timeStamp DATETIME,
    dataTypeID INT,
    value FLOAT

SELECT AddDataFloat(1,1,now(),1,2.2);

 SELECT AddDataFloat(?,NULL,?,?,?);


select * from DataFloat
    where timestamp>=subdate(now(),interval 10 MINUTE);
select * from DataInt;
    where timestamp>=subdate(now(),interval 10 MINUTE);
select * from DataString;
    where timestamp>=subdate(now(),interval 10 MINUTE);


drop database urdb;