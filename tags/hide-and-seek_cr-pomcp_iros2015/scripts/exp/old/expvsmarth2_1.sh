
#!/bin/sh

x=1
while [ $x -le 50 ]
do
  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP (file 'EXP.RUN' is not present)"
	break
  fi


  echo "Run $x"



#./hsmomdp -or -ss -sshn -ssan -u SmartSeeker_Rand -m map3_6x5  &
#sleep 1
#./hsautohider localhost 1120 8 0 0 0 1 1 3 0.2 0.8 0 1

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map1_40x40 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2

sleep 2

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map2_40x40 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2

sleep 2

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map3_40x40 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2

sleep 2

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map4_40x40 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2

sleep 2

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map4_20x20 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2

sleep 2

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map5_20x20 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2


sleep 2

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map3_6x5 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2

sleep 2

./hsmomdp -oh -ss -sshn -ssan -u SmartSeeker_shtv3 -m map4_6x5 -s localhost 1120  &
sleep 1
./hsautohider localhost 1120 8 0 0 0 1 1 5 0.5 1.5 0 -0.2



echo DONE
  x=$(( $x + 1 ))




sleep 2
echo DONE SLEEP
done
