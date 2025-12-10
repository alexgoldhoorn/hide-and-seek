#include "HideSeekModel.h"
#include "MAction.h"
#include "Solver.h"
#include "ParticlesBeliefSet.h"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;



/*string particleToString(Particle* p) {
    stringstream s;
    s << "Particle: state="<<stateToString(p->state)<<", weigth="<<p->weight<<", pathLength="<<p->pathLength;
    return s.str();
}*/

int main(int argc, char **argv)
{
    /*Solver solver;

    solver.input(argc, argv, 2);
    */

    cout << "MCVI solver"<<endl<<endl;

    ostringstream message;

    message << "-m <mapfile> as the first argument\n";

    if (argc < 5 || argv[1][0] != '-' || argv[1][1] != 'm') {
        cout << message.str() << "\n";
        exit(1);
    }
    const char* map_file = argv[2];

    Solver solver;

    solver.input(argc,argv,4);

/*TODO!! check why segmentation fault!!
        use valgrind!!!
*/

    cout << "Parameters:"<<endl;
    cout << "- Policy file: " << solver.policy_file<<endl;
    cout << "- discount: " << solver.discount <<endl;
    cout << "- target precision: " << solver.targetPrecision <<endl;
    cout << "- # backup streams: " << solver.numBackUpStreams << endl;
    cout << "- # next belief streams: " << solver.numNextBeliefStreams<<endl;
    cout << "- max simulation length: " << solver.maxSimulLength<<endl;
    cout << "- iter deep mult: " << solver.iterDeepMult<<endl;
    cout << "- use macro: " << solver.useMacro<<endl;
    cout << "- seed: "<<solver.seed<<endl;
    cout << "- display interval: " << solver.displayInterval<<endl;

    cout <<endl<<"Map:"<<endl;
    GMap map(map_file);
    map.printMap();

    cout <<"Loading model ..."<<endl;

    HideSeekModel currModel(&map);
    ParticlesBeliefSet currSet;
    MAction::initStatic(&currModel);
    ParticlesBelief *root = currModel.getInitBelief(solver.numNextBeliefStreams);

    solver.solve(currModel, currSet, (Belief*)root);
}
