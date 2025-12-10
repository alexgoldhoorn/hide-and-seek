#!/bin/sh
if [  $# -lt 1 ];
then
	port=1120
else
	port=$1
fi

echo "Stopping HS server at port $port" 
/home/$USER/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/build/build/release/server/hsserver -stop $port

