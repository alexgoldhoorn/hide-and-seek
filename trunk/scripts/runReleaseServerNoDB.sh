#!/bin/sh
echo "Running HS server from release on port 1120 without DB log"
cd /home/$USER/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/
/home/$USER/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/build/build/release/server/hsserver /home/$USER/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/cfg/serverconf_release_nodb.xml

