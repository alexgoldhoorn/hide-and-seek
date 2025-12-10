
#include <QtCore/QCoreApplication>

#ifdef GUI_DEBUG
#include <QApplication>
#include "POMCP/treewindow.h"
#endif

//#include "rungamethread.h"

#include <QDir>
#include <QDateTime>
#include <QThread>

#include <cstdlib>
#include <iomanip>

//ag120208: added to prevent atof() being dependend on the systems locale, later set with setlocale()
#include <locale.h>

//appl
//#include "GlobalResource.h"
//#include "solverUtils.h"
//opencv
#include <opencv/cv.h>

//iriutils
#include "exceptions.h"

#include "HSGame/gplayer.h"

#include "gameconnectorclient.h"

#ifndef DO_NOT_USE_MOMDP
#include "Solver/hsmomdp.h"
#include "Solver/hsmomdp_layered.h"
#include "Solver/hsmomdp_runpolicy.h"

#include "Segment/segment.h"
#include "Segment/basesegmenter.h"
#include "Segment/kmeanssegmenter.h"
#include "Segment/robotcenteredsegmentation.h"
#include "Segment/combinecenteredsegmentation.h"
#include "Segment/testsegmenter.h"
#endif

#include "Utils/timer.h"
#include "Utils/generic.h"

#include "hsconfig.h"
#include "hsglobaldata.h"

#include "seekerhs.h"
#include "Smart/smartseeker.h"

#include "POMCP/hssimulator.h"
#include "POMCP/hssimulatorcont.h"
#include "POMCP/hssimulatorpred.h"
//#include "POMCP/hsstate.h"
#include "POMCP/mcts.h"

//#include "gamenoserver.h"

//#include "mcvi/hideseekDiscrete/hideseekmcvi.h"
#include "AutoHider/fromlisthider.h"

#include "hsglobaldata.h"

#include "PeoplePrediction/peoplepredictionwrapper.h"

//TMP --
//#include "POMCP/hspomcp.h"
/*#include <sys/time.h>
#include <time.h>
#include <random>
#include "Smart/follower.h"
#include "Utils/timer.h"*/

using namespace std;


/*__asm__(".symver realpath,realpath@GLIBC_2.11");
__asm__(".symver realpath,realpath@GLIBCXX_3.4.13");*/


inline void testx(int x) {
    if (x<0) exit(0);
}

//>pre defined constants to be defined GLOBALLY!!!!!

//static const int DEFAULT_SERVER_PORT = 1120;
//static char* DEFAULT_SERVER_IP = "localhost";


static const char MODE_RUN_GAME = 0;
static const char MODE_TEST = 1;
static const char MODE_TEST_SEGMENT = 2;
static const char MODE_SHOW_MAP_LIST = 3;
static const char MODE_SHOW_MAP = 4;
static const char MODE_TEST_POMCP = 5;
static const char MODE_WRITE_MAP = 6;
static const char MODE_GUI_TREE = 7;
static const char MODE_TEST_MCTS = 8;
static const char MODE_TEST_PEOPLEPRED = 9;


QStringList RUN_MODES = QStringList()  << "Run Game"
                        << "Test Mode"
                        << "Test segmentation"
                        << "Show Maps list"
                        << "Show Map"
                        << "Test POMCP (simulator)"
                        << "Write Map"
                        << "Tree GUI"
                        << "Test POMCP (nodes)"
                        << "Test MCTS"
                        << "Test People Prediction";

QStringList INITUB_TYPES = QStringList() << "FIB (Fast Informed Bound)"
                           << "(Q)MDP";

QStringList SEGMENTER_TYPES = QStringList() << "[not set]"
                              << "Basic"
                              << "K-means"
                              << "Robot Centered"
                              << "Test"
                              << "Robot Centered Combined";

QStringList SEGMENTER_VALUE_TYPES = QStringList()   << "Belief only"
                                    << "Reward only"
                                    << "Belief x reward";


QStringList TOP_REWARD_AGGR = QStringList() << "Sum"
                              << "Average"
                              << "Minimum"
                              << "Maximum";

QStringList GAME_STATES = QStringList() << "Init"  //note: add +1 since STATE_INIT=-1
                          << "Playing"
                          << "Seeker Wins"
                          << "Hider Wins"
                          << "Tie";

//<

#ifndef DO_NOT_USE_MOMDP
void testHSMOMDP(HSMOMDP* hsmomdp, char* mapFile, SeekerHSParams* params);
void testSegmentation(char* mapFile,Segmenter* segmenter, int row, int col, char* beliefTestFile, SeekerHSParams* params);
#endif
void showMaps();
void showMap(string mapFile, bool usePomcpSim, SeekerHSParams* params);
void testPOMCP(string mapFile, SeekerHSParams* params, bool testMCTS);
void showHelp(string error="", int exitCode=EXIT_FAILURE, char* arg=NULL);
void showVersion();
int getMapIDFromFileName(string file);
void testSeekerHS(SeekerHS* seeker);
void writeMap(string mapFile, string mapFileOut, Pos base, int zoomOutFac, double cellSize, double rowDist_m, double colDist_m, bool isYalm, SeekerHSParams* params);
void writeDistMat(SeekerHSParams* params);
void testPeoplePrediction(SeekerHSParams* params);

inline double rad2deg(double r) {
    return 180*r/M_PI;
}

/*Pos calcOtherRobotPos(const Pos& s1p, const Pos& hpos, const Pos& s2p, SeekerHSParams& ps, GMap& map) {
    static const double M_2PI=2*M_PI;

    //minimum angle between seekers (with min dist to eachother and to hider)
    double a = 2 * asin( ps.minDistBetweenRobots/(2*ps.followPersonDist));
    //direction of already located seeker as seen from hider
    double b1 = map.getDirection(hpos,s1p);
    //the other seeker can at a min distance to hider if the angle is between a1 and a2
    double a1 = (b1+a);
    if (a1>M_2PI) a1 -= M_2PI;
    double a2 = (b1-a);
    if (a2<0) a2 += M_2PI;
    //current direction hider-other seeker
    double b2 = map.getDirection(hpos,s2p);

    //goal direction other seeker
    double c =-1;

    if (a1>a2) {
        //the allowed range contains the 0 rad direction
        if ((b2>=a1 && b2<=M_2PI) || (b2>=0 && b2<=a2)) {
            //already correct direction
            c = b2; //already correct direction
        } else //find closest
            if (b2>b1) {
                c = a1;
            } else {
                c = a2;
            }

    } else {
        //0 rad dir not included
        if (b2>=a1 && b2<=a2) {
            //already correct direction
            c = b2;
        } else { //find closest
            if (b1<a1) { //seeker1 dir between 0 rad and a1 rad
                if (b2<b1 || b2>a2) {
                    c = a2;
                } else {
                    c = a1;
                }
            } else {
                if (b2>a2 && b2<b1) {
                    c = a2;
                } else {
                    c = a1;
                }
            }
        }
    }

    Pos s2p2 = map.tryMoveDir(c,hpos,ps.followPersonDist,false);

    cout << "S1:"<<s1p.toString()<<" H:"<<hpos.toString()<<" S2:"<<s2p.toString()<<endl
         << "a="<<rad2deg(a)<<"º b1="<<rad2deg(b1)<<"º a1="<<rad2deg(a1)<<"º a2="<<rad2deg(a2)<<"º"<<endl
         << "b2="<<rad2deg(b2)<<"º --> c="<<rad2deg(c)<<"º - "<< s2p2.toString()<<endl<<endl;

    return s2p2;
}

static const bool CHANGE_ORIENTATION = false;

double ang(double y, double x) {

cout<<"y="<<y<<" x="<<x<<endl;
    double ang = 0;

    if (CHANGE_ORIENTATION) {
        ang = atan2(-x,-y); //-y,x);
    } else {
        ang = atan2(-y,x);
    }

    ang -= M_PI_2;

    if (ang<=-M_PI)
        ang += 2*M_PI;


    return ang;
}
double getOrientation(const Pos& pos1, const Pos& pos2) {
    double x,y;
    x = pos1.colDouble() - pos2.colDouble();
    y = pos1.rowDouble() - pos2.rowDouble();

    return ang(y,x);
}
*/
int main(int argc, char *argv[]) {
/*    Pos p1,p2;

    do {
        cout <<"p1.row=";
        double r,c;
        cin >> r;
        cout<<"p1.col=";
        cin>>c;
        p1.set(r,c);
        cout <<"p2.row=";
        cin >> r;
        cout<<"p2.col=";
        cin>>c;
        p2.set(r,c);
        cout <<" "<<p1.toString()<<" -> "<<p2.toString()<<": "<<getOrientation(p1,p2)*180/M_PI<<"º"<<endl;

    } while (p1.isSet() && p2.isSet());
    return 0;*/

    /*NormalBivariateDist n;

    vector<double> meanVec(2);
    vector<double> covVec(4);


    char in[10];
    cout << "Now enter the paremeters, first the mean:"<<endl<<"mu_x: ";
    cin >>in;
    meanVec[0] = atof(in);
    cout << "mu_y: ";
    cin >>in;
    meanVec[1] = atof(in);

    cout<<"Now the covariance:"<<endl<<"sigma_xx: ";
    cin >>in;
    covVec[0] = atof(in);
    cout << "sigma_xy: ";
    cin >>in;
    covVec[1] = atof(in);
    cout << "sigma_yx: ";
    cin >>in;
    covVec[2] = atof(in);
    cout << "sigma_yy: ";
    cin >>in;
    covVec[3] = atof(in);


cout << "Set vectors:"<<flush;
    n.setMeanCovar(meanVec, covVec);	
    cout << "ok"<<endl<<"100000 random: "<<flush;

    ofstream out("samples.txt");

    for (int i=0;i<100000;i++) {
        double x,y;
        n.getRandPoint(x,y);
        //cout << "("<<x<<","<<y<<") ";
        out << x<<","<<y<<endl;
    }
    cout <<"ok"<<endl;
    out.close();

    return 0;*/


   /* Pos p1(1,1),p2;

    for(int r=0;r<3;r++) {
        for(int c=0;c<3;c++) {
            p2.set(r,c);
            cout << p1.toString()<<"-"<<p2.toString()<<", orient:"<<(180.0*SeekerHS::getOrientation(p1,p2)/M_PI)<<"º"<<endl;
        }
    }

    double d1,d2;
    while (true) {
        cout << "Orientation diff - or1: ";
        cin>>d1;
        cout<<"or2: ";
        cin>>d2;
        cout <<"diff: "<<SeekerHS::orientDiff(d1,d2)<<endl;
    }

    return 0;*/
/*
    SeekerHSParams ps;
    ps.useContinuousPos=true;
    Pos p2(2.5,4.5),p1(2.5,2.5);
    GMap map(&ps);
    map.createMap(5,5);
    map.setMapFixed();*/
    /*map.printMap(p1,p2);
    Pos pn = map.getNextStep(p1,p2);
    cout <<"Next: "<<pn.toString()<<endl;
    double d = map.getDirection(p1,pn);
    cout<<"dir: "<<d<<endl;
    Pos pt = map.tryMoveDirStep(d,p1,1.0,0.3,true);
    cout <<"step:"<<pt.toString()<<endl;

    double dir = map.getDirection(p1, p2);

    cout<<" Person's' dir="<<(180*dir/M_PI)<<"º";;

    //now get the two poses behind the robot
    Pos nextPos1 = map.tryMoveDirStep(dir+M_PI-ps.multiSeekerFollowAngle_rad/2, p2, ps.followPersonDist,
                                        ps.seekerStepDistance, ps.doVisibCheckBeforeMove);
    Pos nextPos2 = map.tryMoveDirStep(dir+M_PI+ps.multiSeekerFollowAngle_rad/2, p2, ps.followPersonDist,
                                        ps.seekerStepDistance, ps.doVisibCheckBeforeMove);

    cout<<" pos1:"<<nextPos1.toString()<<", pos2:"<<nextPos2.toString()<<endl;

    cout<<"exploration: "<<endl;
    Pos seekerPos(0,0);
    vector<Pos> highestBeliefPosVec(2);
    vector<double> highestBeliefVec(2);
    Pos* otherRobotGoal = NULL;

    highestBeliefPosVec[0].set(0,4);
    highestBeliefPosVec[1].set(4,0);
    highestBeliefVec[0] = 0.3;
    highestBeliefVec[1] = 0.2;

    int i = calcExplorationEvaluation(seekerPos, highestBeliefPosVec, highestBeliefVec, otherRobotGoal, &ps, &map);
    otherRobotGoal = &highestBeliefPosVec[i];
    cout <<"for s1:"<<seekerPos.toString()<<": i"<<i<<flush<<" -> "<<otherRobotGoal->toString()<<endl;

    i = calcExplorationEvaluation(seekerPos, highestBeliefPosVec, highestBeliefVec, otherRobotGoal, &ps, &map);
    otherRobotGoal = &highestBeliefPosVec[i];
    cout <<"for s2:"<<seekerPos.toString()<<": i"<<i<<flush<<" -> "<<otherRobotGoal->toString()<<endl;
*/

/*
    cout<<"----"<<endl;
    Pos hpos(2.5,2.5);
    Pos s1p(3.5,2.5);
    Pos s2p(0,0);

    for(int rs2=-1;rs2<=1;rs2++) {
        for(int cs2=-1;cs2<=1;cs2++) {
            s2p.set(hpos.rowDouble()+rs2,hpos.colDouble()+cs2);

            for(int rs=-1;rs<=1;rs++) {
                for(int cs=-1;cs<=1;cs++) {
                    s1p.set(hpos.rowDouble()+rs,hpos.colDouble()+cs);

                    map.printMap(s1p,hpos);
                    calcOtherRobotPos(s1p,hpos,s2p,ps,map);
                }
            }

        }
    }




    return 0;*/

#ifdef GUI_DEBUG
    //cout << "GUI_DEBUG"<<endl;
    QApplication a(argc,argv);
#else
    //cout << "NO!! GUI_DEBUG"<<endl;
    QCoreApplication a(argc, argv);
#endif

    //ag140521: reset locale after Qt sets to system's local (http://qt-project.org/doc/qt-5/QCoreApplication.html#locale-settings)
    setlocale(LC_NUMERIC,"C");
    //ag120208: set locale to en-US such that the atof uses decimal dots and not comma
    setlocale(LC_NUMERIC,"en_US");

    try {
        cout <<endl
             << "+-----------------------------------+" << endl
             << "| Automatic Seeker Hide&Seek Client |" << endl
             << "+-----------------------------------+" << endl<<endl;

        //the params struct used in the whole project
        SeekerHSParams params;

        //local params, used only in this main
        char* beliefTestFile = NULL;
        char runMode = MODE_RUN_GAME;
        char* logFilePrefix = NULL;
        bool showVer = false;
        int robotRow = -1, robotCol = -1;
        bool sendMap = false;
        bool sendMapPGM = false;
        bool mapIsYalm = false;
        Pos basePos;
        double rowDist_m = 0;
        double colDist_m = 0;
        int zoomOutFac = 1;
        double cellSize = 1;
        string mapFileOut;
        bool passSeekerHSToGameConnectorClient = false;
        bool debugAddNewTracksManually = false;
        QString comments;
        //QString startPosFile;

        //AG120914: use solverparams to pass params
        #ifndef DO_NOT_USE_MOMDP
        SolverParams* solverParams = &GlobalResource::getInstance()->solverParams;
        solverParams->useLookahead = false;
        #endif


        if (argc < 2) {
            showHelp("missing parameters");
        }

        //loop the params
        for (int i=1; i<argc; i++) {
            char* arg = argv[i];
            if (strlen(arg)<2 || arg[0]!='-') {
                showHelp("unknown parameter",EXIT_FAILURE,arg);
            }
            //cout << " arg: '"<<arg<<"'"<<endl;

            switch (arg[1]) {
                case 't': // test
                    if (strlen(arg)>2) {
                        switch (arg[2]) {
                            case 's':
                                #ifdef DO_NOT_USE_MOMDP
                                cout <<"WARNING: cannot use segmentation because compiled with DO_NOT_USE_MOMDP."<<endl;
                                #else
                                runMode = MODE_TEST_SEGMENT; //segmentation
                                if (i+1<argc && argv[i+1][0]!='-') {
                                    beliefTestFile = argv[++i];

                                    if (i+2<argc && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
                                        QString n = QString::fromLatin1(argv[++i]);
                                        bool ok = false;
                                        robotRow = n.toInt(&ok);
                                        if (!ok) showHelp("expected a number for position row at",EXIT_FAILURE,arg);

                                        n = QString::fromLatin1(argv[++i]);
                                        ok = false;
                                        robotCol = n.toInt(&ok);
                                        if (!ok) showHelp("expected a number for position col at",EXIT_FAILURE,arg);
                                    }
                                }
                                #endif
                                break;
                            case 'm':
                                runMode = MODE_SHOW_MAP;
                                break;
                            case 'c':
                                runMode = MODE_TEST_POMCP;
                                break;
                            case 'C':
                                runMode = MODE_TEST_MCTS;
                                break;
                            case 'g':
                                runMode = MODE_GUI_TREE;
                                break;
                            case 'S':
                                passSeekerHSToGameConnectorClient = true;
                                break;
                            case 'D':
                                passSeekerHSToGameConnectorClient = true;
                                debugAddNewTracksManually = true;
                                break;
                            case 'N':
                                params.multiSeekerNoCommunication = true;
                                break;
                            case 'p':
                                runMode = MODE_TEST_PEOPLEPRED;
                                break;
                            default:
                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                                break;
                        }
                    } else {
                        runMode = MODE_TEST;
                    }
                    break;
                    /*case 'H':
                    case '?':*/
                case 'G': //game type
                    if (strlen(arg)>=3) {
                        switch (arg[2]) {
                            case 'f':
                                if (strlen(arg)==4 && arg[3]=='2')
                                    params.gameType = HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB;
                                else
                                    params.gameType = HSGlobalData::GAME_FIND_AND_FOLLOW;
                                break;
                            case 'h':
                                params.gameType = HSGlobalData::GAME_HIDE_AND_SEEK;
                                break;
                            default:
                                showHelp("unknown game type",EXIT_FAILURE,arg);
                        }
                    }
                    break;
                case 'h': // help
                    switch(strlen(arg)) {
                        case 2:
                            showHelp("",EXIT_SUCCESS); //help
                            break;
                        case 4: //simulated hider type
                            if (arg[2]=='s') {
                                switch(arg[3]) {
                                    case 'r':
                                        params.simHiderType = SeekerHSParams::SIM_HIDER_TYPE_RANDOM;
                                        break;
                                    case 'b':
                                        params.simHiderType = SeekerHSParams::SIM_HIDER_TYPE_TOBASE;
                                        break;
                                    case 's':
                                        params.simHiderType = SeekerHSParams::SIM_HIDER_TYPE_SMART;
                                        break;
                                    /*case 'p':
                                        params.simHiderType = SeekerHSParams::SIM_HIDER_TYPE_PREDICT;
                                        break;*/
                                    default:
                                        showHelp("unexpected simulated hider type",EXIT_FAILURE,arg);
                                }

                                //check if random prob is passed
                                if (i+1<argc && argv[i+1][0]!='-') {
                                    //expected the prob
                                    QString probStr(argv[++i]);
                                    bool ok = false;
                                    params.simHiderRandomActProb = probStr.toDouble(&ok);
                                    if (!ok) showHelp("expected probablity instead of ",EXIT_FAILURE,argv[i]);
                                }

                                break;
                            }
                        default:
                            showHelp("unexpected simulated hider type",EXIT_FAILURE,arg);
                    }
                    break;
                case 's': //server or solver
                    if (strlen(arg)==2) { //server: "-s"
                        if (i+1>=argc && argv[i+1][0]=='0') {
                            params.serverIP.clear();
                            params.serverPort = 0;
                            i++;
                            break;
                        } else if (i+2>=argc) {
                            showHelp("server expecting two parameters: ip port");
                        }
                        params.serverIP = argv[++i];
                        QString portStr(argv[++i]);
                        bool ok = false;
                        params.serverPort = portStr.toInt(&ok);
                        if (!ok) showHelp("server port is not an integer");
                    } else {
                        switch (arg[2]) {
                            case 'o':
                                params.solverType=SeekerHSParams::SOLVER_OFFLINE;
                                break;
                            case 'l':
                                params.solverType=SeekerHSParams::SOLVER_LAYERED;
                                if (strlen(arg)==4 && arg[3]=='c') {
                                    params.solverType=SeekerHSParams::SOLVER_LAYERED_COMPARE;
                                }
                                break;
                            case 's':
                                if (strlen(arg)>3) {
                                    switch (arg[3]) {
                                        case 'a': //-ssa: score type
                                            if (strlen(arg)==5) {
                                                switch(arg[4]) {
                                                    case 'a':
                                                        params.smartSeekerScoreActionType = SmartSeeker::SCORE_AVG;
                                                        break;
                                                    case 'x':
                                                        params.smartSeekerScoreActionType = SmartSeeker::SCORE_MAX;
                                                        break;
                                                    case 'n':
                                                        params.smartSeekerScoreActionType = SmartSeeker::SCORE_MIN;
                                                        break;
                                                    default:
                                                        showHelp("unknown smart seeker score (action) type",EXIT_FAILURE,arg);
                                                }
                                            } else {
                                                showHelp("unknown smart seeker parameter",EXIT_FAILURE,arg);
                                            }
                                            break;
                                        case 'h': //-ssh: score type
                                            if (strlen(arg)==5) {
                                                switch(arg[4]) {
                                                    case 'a':
                                                        params.smartSeekerScoreHiddenType = SmartSeeker::SCORE_AVG;
                                                        break;
                                                    case 'x':
                                                        params.smartSeekerScoreHiddenType = SmartSeeker::SCORE_MAX;
                                                        break;
                                                    case 'n':
                                                        params.smartSeekerScoreHiddenType = SmartSeeker::SCORE_MIN;
                                                        break;
                                                    default:
                                                        showHelp("unknown smart seeker score (hidden) type",EXIT_FAILURE,arg);
                                                }
                                            } else {
                                                showHelp("unknown smart seeker parameter",EXIT_FAILURE,arg);
                                            }
                                            break;
                                        case 'm': { //max
                                            if (i+1>=argc) showHelp("smart seeker maximum cell calculation requires pararameter");
                                            QString numStr(argv[++i]);
                                            bool ok = false;
                                            params.smartSeekerMaxCalcsWhenHidden = numStr.toInt(&ok);
                                            if (!ok) showHelp("smart seeker maximum cell calculation parameter is not an integer");
                                            break;
                                        }
                                        default:
                                            showHelp("unknown (smart seeker) parameter", EXIT_FAILURE, arg);
                                            break;
                                    }
                                } else {
                                    params.solverType=SeekerHSParams::SOLVER_SMART_SEEKER;
                                }
                                break;
                            case 'm':
                                params.solverType=SeekerHSParams::SOLVER_MCVI_OFFLINE;
                                break;
                            case 'h':                                
                                if (strlen(arg)==3) {
                                    params.solverType=SeekerHSParams::SOLVER_FOLLOWER_HIGHEST_BELIEF;
                                    params.pomcpSimType=SeekerHSParams::POMCP_SIM_DISCR;
                                } else if (strlen(arg)>3) {
                                    switch (arg[3]) {
                                        case 'c': //POMCP continuous
                                            params.solverType=SeekerHSParams::SOLVER_FOLLOWER_HIGHEST_BELIEF;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_CONT;
                                            break;
                                        case 'd': //POMCP discrete
                                            params.solverType=SeekerHSParams::SOLVER_FOLLOWER_HIGHEST_BELIEF;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_DISCR;
                                            break;
                                        case 'p': //AG140905: POMCP predictor
                                            params.solverType=SeekerHSParams::SOLVER_FOLLOWER_HIGHEST_BELIEF;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_PRED;
                                            break;
                                        default:
                                            showHelp("unknown POMCP solver type", EXIT_FAILURE, arg);
                                    }
                                }
                                break;
                            case 'c':
                                if (strlen(arg)==3) {
                                    params.solverType=SeekerHSParams::SOLVER_POMCP;
                                    params.pomcpSimType=SeekerHSParams::POMCP_SIM_DISCR;
                                } else if (strlen(arg)>3) {
                                    switch (arg[3]) {
                                        case 'i':
                                            params.setInitNodeValue = true;
                                            break;
                                        case 'c': //POMCP continuous
                                            params.solverType=SeekerHSParams::SOLVER_POMCP;
                                            //params.solverType=SeekerHSParams::SOLVER_POMCP_CONT;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_CONT;
                                            break;
                                        case 'd': //POMCP discrete
                                            params.solverType=SeekerHSParams::SOLVER_POMCP;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_DISCR;
                                            break;
                                        case 'p': //AG140905: POMCP predictor
                                            params.solverType=SeekerHSParams::SOLVER_POMCP;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_PRED;
                                            break;
                                        default:
                                            showHelp("unknown POMCP solver type", EXIT_FAILURE, arg);
                                    }
                                }
                                break;
                            case 'C':
                                if (strlen(arg)==3) {
                                    params.solverType=SeekerHSParams::SOLVER_COMBI_POMCP_FOLLOWER;
                                    params.pomcpSimType=SeekerHSParams::POMCP_SIM_DISCR;
                                } else if (strlen(arg)>3) {
                                    switch (arg[3]) {
                                        case 'c': //POMCP continuous
                                            params.solverType=SeekerHSParams::SOLVER_COMBI_POMCP_FOLLOWER;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_CONT;
                                            break;
                                        case 'd': //POMCP discrete
                                            params.solverType=SeekerHSParams::SOLVER_COMBI_POMCP_FOLLOWER;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_DISCR;
                                            break;
                                        case 'p': //AG140905: POMCP predictor
                                            params.solverType=SeekerHSParams::SOLVER_COMBI_POMCP_FOLLOWER;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_PRED;
                                            break;
                                        case 'h': //highest belief
                                            params.solverType=SeekerHSParams::SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_CONT;
                                            break;
                                        case 'H': //AG140905: POMCP predictor
                                            params.solverType=SeekerHSParams::SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF;
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_PRED;
                                            break;
                                        default:
                                            showHelp("unknown POMCP combi solver type", EXIT_FAILURE, arg);
                                    }
                                }
                                break;
                            case 'f': {
                                if (strlen(arg)==3) {
                                    params.solverType=SeekerHSParams::SOLVER_FOLLOWER;
                                } else {
                                    if (arg[3]=='e') {
                                        params.solverType=SeekerHSParams::SOLVER_FOLLOWER_LAST_POS_EXACT;
                                    } else {
                                        showHelp("Unknown follower type");
                                    }
                                }
                                break;
                            }
                            case 'M':
                                if (strlen(arg)==3) {
                                    params.pomcpSimType=SeekerHSParams::POMCP_SIM_CONT;
                                } else if (strlen(arg)>3) {
                                    switch (arg[3]) {
                                        case 'c': //POMCP continuous
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_CONT;
                                            break;
                                        case 'd': //POMCP discrete
                                            params.pomcpSimType=SeekerHSParams::POMCP_SIM_DISCR;
                                            break;
                                        default:
                                            showHelp("unknown POMCP multi seeker solver type", EXIT_FAILURE, arg);
                                    }
                                }
                                params.solverType = SeekerHSParams::SOLVER_MULTI_HB_EXPL;
                                break;
                            default:
                                showHelp("unknown parameter/solver", EXIT_FAILURE, arg);
                                break;
                        }
                    }
                    break;
                case 'm': {// momdp file
                    if (strlen(arg)==2) {
                        if (i+1>=argc) showHelp("expected MOMDP file");
                        params.pomdpFile = argv[++i];
                    } else if (strlen(arg)==3) {
                        switch(arg[2]) {
                            case 'o': {
                                QString sdStr(argv[++i]);
                                bool ok = false;
                                params.multiSeekerOwnObsChooseProb = sdStr.toDouble(&ok);
                                if (!ok) showHelp("own observ. probability expected",EXIT_FAILURE,argv[i]);
                                break;
                            }
                            case 'd': {
                                QString sdStr(argv[++i]);
                                bool ok = false;
                                params.multiSeekerSelectPosSmallDistBetwGoals = sdStr.toDouble(&ok);
                                if (!ok) showHelp("expected small dist betw. goal",EXIT_FAILURE,argv[i]);
                                break;
                            }
                            case 'r': {
                                QString dStr(argv[++i]);
                                bool ok = false;
                                params.minDistBetweenRobots = dStr.toDouble(&ok);
                                if (!ok) showHelp("expected min. dist. betw. robots",EXIT_FAILURE,argv[i]);
                                break;
                            }
                        }
                    } else {
                        showHelp("unknown parameter", EXIT_FAILURE, arg);
                    }
                    break;
                }
                case 'p': //policy file or precision
                    if (strlen(arg)==2) { // policy file
                        if (i+1>=argc) showHelp("expected policy file");
                        params.policyFile = argv[++i];
                    } else if (arg[2]=='d') {
                        //AG121008: disable pruning
                        params.doPruning = false;
                    } else if (arg[2]=='p') {
                        if (strlen(arg)==3) {
                            //AG121016: print params
                            params.showParams = true;
                        } else if (strlen(arg)==4) {
                            switch(arg[3]) {
                                case 'f':
                                    if (i+1>=argc) {
                                        showHelp("unknown parameter/missing people prediction destination file",EXIT_FAILURE, arg);
                                    } else {
                                        params.ppDestinationFile = argv[++i];
                                        cout << "Destination file: "<<params.ppDestinationFile<<endl;
                                    }
                                    break;
                                case 'h':
                                    if (i+1>=argc) {
                                        showHelp("unknown parameter/missing people prediction horizon",EXIT_FAILURE, arg);
                                    } else {
                                        QString ppHorVal(argv[++i]);
                                        bool ok=false;
                                        params.ppHorizonSteps = ppHorVal.toUInt(&ok);
                                        if (!ok) {
                                            showHelp("expecting people prediction horizon (uint) for given parameter",EXIT_FAILURE, arg);
                                        }
                                    }
                                    break;
                                default:
                                    showHelp("unknown parameter", EXIT_FAILURE, arg);
                                    break;
                            }
                        }
                    } else {
                        if (i+1>=argc) {
                            showHelp("unknown parameter/missing probability value",EXIT_FAILURE, arg);
                        }
                        //now get precision value
                        QString pvalQStr(argv[++i]);
                        bool ok=false;
                        double pval = pvalQStr.toDouble(&ok);
                        if (!ok) {
                            showHelp("expecting precision value (double) for given parameter",EXIT_FAILURE, arg);
                        }

                        switch (arg[2]) {
                            case 't':
                                params.targetPrecision = pval;
                                break;
                            case 'i':
                                params.targetInitPrecFact = pval;
                                break;
                            default:
                                showHelp("unknown parameter", EXIT_FAILURE, arg);
                                break;
                        }
                    }
                    break;
                case 'a': // map file
                    if (strlen(arg)==3) {
                        sendMap = true;
                        if (arg[2]!='s' && arg[2]!='p' && arg[2]!='y') {
                            showHelp("expected map send type",EXIT_FAILURE,arg);
                        }
                        if (arg[2]=='p' || arg[2]=='y') {
                            sendMapPGM = true;
                        }
                        if (arg[2]=='y') {
                            mapIsYalm = true;
                        }
                    }

                    if (i+1>=argc) showHelp("expected map file");
                    params.mapFile = argv[++i];
                    //cout << "map:"<<params.mapFile<<endl<<"next:"<<(argc>i+1?argv[i+1]:"[n/a]")<<endl<<"argc="<<argc<<endl<<"sendmappgm:"<<sendMapPGM<<endl<<"i="<<i<<endl;
                    if (!mapIsYalm && sendMapPGM && i+1<argc && argv[i+1][0]!='-') {
                        QString zmStr(argv[++i]);
                        bool ok=false;
                        zoomOutFac = zmStr.toInt(&ok);
                        if (!ok) showHelp("expected zoomoutfactor",EXIT_FAILURE,argv[i]);
                        //cout << "zoomoutf="<<zoomOutFac<<endl;
                        //cout <<"i="<<i<<endl;                        
                    } else if (mapIsYalm) {
                        if (i+3>=argc) showHelp("expected cellsize, rowDist and colDist in m to get of map");
                        bool ok = false;

                        QString cellSStr(argv[++i]);
                        cellSize = cellSStr.toDouble(&ok);
                        if (!ok) showHelp("cell size not a value",EXIT_FAILURE,argv[i]);

                        QString rowDistStr(argv[++i]);
                        rowDist_m = rowDistStr.toDouble(&ok);
                        if (!ok) showHelp("row distance not a value",EXIT_FAILURE,argv[i]);

                        QString colDistStr(argv[++i]);
                        colDist_m = colDistStr.toDouble(&ok);
                        if (!ok) showHelp("col distance not a value",EXIT_FAILURE,argv[i]);
                    }
                    break;
                case 'W': //write map file
                    if (i+1>=argc) showHelp("expected map file");
                    mapFileOut = argv[++i];
                    runMode = MODE_WRITE_MAP;
                    break;
                case 'A': { //map ID
                    if (i+1>=argc) showHelp("expected map ID or '?'");
                    char* idchr = argv[++i];
                    if (strlen(idchr)==1 && idchr[0]=='?') {
                        runMode = MODE_SHOW_MAP_LIST;
                    } else {
                        QString idStr(idchr);
                        bool ok = false;
                        params.mapID = idStr.toInt(&ok);
                        if (!ok) showHelp("expected map ID instead of ",EXIT_FAILURE,argv[i]);
                    }
                    break;
                }
                case 'u': //user name or upper bound init type
                    if (strlen(arg)==2) { //user name
                        if (i+1>=argc) showHelp("expected user name");
                        params.userName = argv[++i];
                    } else if (strlen(arg)!=3) { //upper bound initializer
                        showHelp("unknown parameter / bound initializator: ",EXIT_FAILURE, arg);
                    } else {
                        switch (arg[2]) {
                            case 'f':
                                params.ubInitType = SeekerHSParams::UPPERBOUND_INIT_FIB;
                                break;
                            case 'm':
                                params.ubInitType = SeekerHSParams::UPPERBOUND_INIT_MDP;
                                break;
                            case 'd':
                                params.useDeducedAction = true;
                                break;
                            case 's': {
                                if (i+1>=argc) showHelp("expected number of steps", EXIT_FAILURE, arg);
                                QString numSStr(argv[++i]);
                                bool ok = false;
                                params.highBeliefFollowerUpdateGoalNumSteps = numSStr.toUInt(&ok);
                                if (!ok) showHelp("expected number of steps", EXIT_FAILURE, argv[i]);
                                break;
                            }
                            default:
                                showHelp("unknown parameter / bound initializator ",EXIT_FAILURE,arg);
                                break;
                        }
                    }
                    break;
                case 'T': { //solver timeout
                    if (strlen(arg)==2) {
                        if (i+1>=argc) showHelp("expected solver timeout");
                        QString timeOutStr(argv[++i]);
                        bool ok = false;
                        params.timeoutSeconds = timeOutStr.toInt(&ok);
                        if (!ok) showHelp("expected solver timeout instead of ",EXIT_FAILURE,argv[i]);
                    } else if (strlen(arg)==3) {
                        switch (arg[2]) {
                            case 'f':
                                params.setFinalStateOnTop = true;
                                break;
                            case 'r':
                                params.setFinalTopRewards = true;
                                break;
                            default:
                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                        }
                    } else {
                        showHelp("unknown parameter",EXIT_FAILURE,arg);
                    }
                    break;
                }
                case 'o': //opponent type
                    if (strlen(arg)!=3) {  //expecting "-o?"
                        showHelp("unknown parameter / opponent type: ",EXIT_FAILURE, arg);
                    } else {
                        switch (arg[2]) {
                            case 'h':
                                params.opponentType =  HSGlobalData::OPPONENT_TYPE_HUMAN;
                                break;
                            case 'r':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM;
                                break;
                            case 's':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_SMART;
                                break;
                            case 'f':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_FILE;
                                //fallthrough to get file
                            case 'l': { //AG120904: opponent with action list
                                if (arg[2]=='l') params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST;

                                if (i+1>=argc) showHelp("expected action list file");
                                params.oppActionFile = argv[++i];
                                }
                                break;
                            case 'v':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART;
                                break;
                            case 'a':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING;
                                break;
                            case 'A':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING;
                                break;
                            case '2':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART2;
                                break;
                            case 'T':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_V2ALLKNOWING;
                                break;
                            case 'w':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM_WALKER;
                                break;
                            case 'x':
                                params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_FIXED_RAND_POS;
                                break;
                            default:
                                showHelp("unknown parameter / opponent type ",EXIT_FAILURE,arg);
                                break;
                        }

                        //check if more params
                        switch (arg[2]) {
                            case 's':
                            case 'v':
                            case 'a':
                            case 'A':
                                if (i+1<argc) {
                                    if (argv[i+1][0]!='-') {
                                        QString std(argv[++i]);
                                        bool ok = false;
                                        params.oppHiderNoiseStd = std.toDouble(&ok);
                                        if (!ok) showHelp("expected noise std ",EXIT_FAILURE,argv[i]);
                                    }
                                }
                        }
                    }
                    break;
                case 'g': { //segmenter type
                    if (strlen(arg)<3) {  //expecting "-o?"
                        showHelp("unknown parameter / segmentation type: ",EXIT_FAILURE, arg);
                    } else {
                        switch (arg[2]) {
                            case 'b':
                                params.segmenterType = SeekerHSParams::SEGMENTER_BASIC;
                                break;
                            case 'k':
                                params.segmenterType = SeekerHSParams::SEGMENTER_KMEANS;
                                //check if k given
                                if (i+1<argc && argv[i+1][0]!='-') {
                                    //assume the k is passed
                                    QString kvalStr(argv[++i]);
                                    bool ok = false;
                                    params.k = kvalStr.toInt(&ok);
                                    if (!ok) showHelp("expecting k / unknown parameter",EXIT_FAILURE,arg);
                                }

                                break;
                            case 'r':
                                if (strlen(arg)>3 && arg[3]=='x') {
                                    params.segmenterTypeX = SeekerHSParams::SEGMENTER_ROBOT_CENTERED;
                                } else {
                                    params.segmenterType = SeekerHSParams::SEGMENTER_ROBOT_CENTERED;
                                }
                            case 'c': {
                                bool isXSegmenter =  (strlen(arg)>3 && arg[3]=='x');
                                if (arg[2]=='c') {
                                    if (isXSegmenter) {
                                        params.segmenterTypeX = SeekerHSParams::SEGMENTER_CENTERED_COMBINED;
                                    } else {
                                        params.segmenterType = SeekerHSParams::SEGMENTER_CENTERED_COMBINED;
                                    }
                                }

                                //check if k given
                                if (i+1<argc && argv[i+1][0]!='-') {
                                    //assume the k is passed
                                    QString rcSegStr(argv[++i]);
                                    bool ok = false;
                                    if (isXSegmenter)
                                        params.rcXSegmDist = rcSegStr.toDouble(&ok);
                                    else
                                        params.rcSegmDist = rcSegStr.toDouble(&ok);
                                    if (!ok) showHelp("expecting segmentation diagonal / unknown parameter",EXIT_FAILURE,arg);

                                    if (i+1<argc && argv[i+1][0]!='-') {
                                        QString rcAngStr(argv[++i]);
                                        bool ok = false;
                                        if (isXSegmenter)
                                            params.rcXAngDist = rcAngStr.toDouble(&ok);
                                        else
                                            params.rcAngDist = rcAngStr.toDouble(&ok);
                                        if (!ok) showHelp("expecting segmentation angle / unknown parameter",EXIT_FAILURE,arg);

                                        if (i+1<argc && argv[i+1][0]!='-') {
                                            QString rcBaseRadStr(argv[++i]);
                                            bool ok = false;
                                            if (isXSegmenter)
                                                params.rcXBaseRad = rcBaseRadStr.toDouble(&ok);
                                            else
                                                params.rcBaseRad = rcBaseRadStr.toDouble(&ok);
                                            if (!ok) showHelp("expecting base rad / unknown parameter",EXIT_FAILURE,arg);

                                            if (i+1<argc && argv[i+1][0]!='-') {
                                                QString rcHighResRadStr(argv[++i]);
                                                bool ok = false;
                                                if (isXSegmenter)
                                                    params.rcXHighResR = rcHighResRadStr.toDouble(&ok);
                                                else
                                                    params.rcHighResR = rcHighResRadStr.toDouble(&ok);
                                                if (!ok) showHelp("expecting high res rad / unknown parameter",EXIT_FAILURE,arg);
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                            case 't':
                                params.segmenterType = SeekerHSParams::SEGMENTER_TEST;
                                break;
                            case 'v':
                                //<< "   -gvr | -gvb | -gvbr segment value: reward only / belief only / belief*reward" <<endl
                                #ifndef DO_NOT_USE_MOMDP
                                switch (strlen(arg)) {
                                    case 4:
                                        switch(arg[3]) {
                                            case 'r':
                                                params.segmentValueType = LayeredHSMOMDP::SEGMENT_VALUE_REWARD_ONLY;
                                                break;
                                            case 'b':
                                                params.segmentValueType = LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_ONLY;
                                                break;
                                            default:
                                                showHelp("expecting segmentation value type (unknown value type)",EXIT_FAILURE,arg);
                                                break;
                                        }
                                        break;
                                    case 5:
                                        if ((arg[3]=='r' && arg[4]=='b') || (arg[3]=='b' && arg[4]=='r')) {
                                            params.segmentValueType = LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_REWARD;
                                        } else {
                                            showHelp("expecting segmentation value type (unknown combi value type)",EXIT_FAILURE,arg);
                                        }
                                        break;
                                    default:
                                        showHelp("expecting segmentation value type (expected value type: -gvX)",EXIT_FAILURE,arg);
                                        break;
                                }
                                #else
                                cout << "Warning: segment not available, because compiled with DO_NOT_USE_MOMDP."<<endl;
                                #endif
                                break;
                            default:
                                showHelp("unknown parameter / segmenter type ",EXIT_FAILURE,arg);
                                break;
                        }
                    }
                }
                break;
                case 'L': //log file prefix
                    if (i+1>=argc) showHelp("expected log file prefix",EXIT_FAILURE,arg);
                    logFilePrefix = argv[++i];
                    break;
                case 'D': { //discount factor
                    if (i+1>=argc) showHelp("expected discount factor",EXIT_FAILURE,arg);
                    QString discStr(argv[++i]);
                    bool ok = false;
                    params.discount = discStr.toDouble(&ok);
                    if (!ok) showHelp("discount factor not a double",EXIT_FAILURE,arg);
                    break;
                }
                case 'b': // best action option
                    if (strlen(arg)==2) {
                        //base
                        if (i+2>=argc) showHelp("expected base row col",EXIT_FAILURE,arg);
                        QString rStr(argv[++i]);
                        bool ok = false;
                        int row = rStr.toInt(&ok);
                        int col = -1;
                        if (ok) {
                            QString cStr(argv[++i]);
                            col = cStr.toInt(&ok);
                        }
                        if (!ok) showHelp("expected base row col",EXIT_FAILURE,arg);
                        basePos.set(row,col);

                    } else if (strlen(arg)!=3) {  //expecting "-b?"
                        showHelp("unknown parameter / opponent type: ",EXIT_FAILURE, arg);
                    } else {
                        switch (arg[2]) {
                            case 'l':
                                params.useLookahead = true;
                                break;
                            case 'z': {
                                if (i+1>=argc) showHelp("expected belief zoom factor",EXIT_FAILURE,arg);
                                QString zStr(argv[++i]);
                                bool ok=false;
                                params.beliefMapZoomFactor = zStr.toDouble(&ok);
                                if (!ok) showHelp("expected belief zoom factor",EXIT_FAILURE,argv[i]);
                                break;
                            }
                            case 'i': { //AG150203: belief image
                                params.beliefImageFile = argv[++i];
                                break;
                            }
                            case 'f':
                                params.pomcpFilterBeliefAtUpdate = true;
                                break;
                            default:
                            showHelp("unknown parameter / opponent type: ",EXIT_FAILURE, arg);
                            break;
                        }
                    }
                    break;
                case 'r': // top reward option
                    if (strlen(arg)<3) {  //expecting "-r?"
                        showHelp("unknown parameter / top reward type: ",EXIT_FAILURE, arg);
                    } else {
                        #ifndef DO_NOT_USE_MOMDP
                        switch(arg[2]) {
                            case 's':
                                params.topRewAggr = LayeredHSMOMDP::TOP_REWARD_SUM;
                                break;
                            case 'a':
                                params.topRewAggr = LayeredHSMOMDP::TOP_REWARD_AVG;
                                break;
                            case 'n':
                                params.topRewAggr = LayeredHSMOMDP::TOP_REWARD_MIN;
                                break;
                            case 'x':
                                params.topRewAggr = LayeredHSMOMDP::TOP_REWARD_MAX;
                                break;
                            case 'p': {
                                if (strlen(arg)==4) {
                                    switch (arg[3]) {
                                        case 'r':
                                            params.rolloutPolicyType = SeekerHSParams::ROLL_OUT_POLICY_TYPE_RANDOM;
                                            break;
                                        case 's':
                                            params.rolloutPolicyType = SeekerHSParams::ROLL_OUT_POLICY_TYPE_SMART;
                                            break;
                                        default:
                                            showHelp("unknown parameter / roll-out policy type: ",EXIT_FAILURE, arg);
                                            break;
                                    }
                                } else {
                                    showHelp("unknown parameter / roll-out policy type: ",EXIT_FAILURE, arg);
                                }
                            }
                            break;
                            default:
                                showHelp("unknown parameter / top reward type: ",EXIT_FAILURE, arg);
                                break;
                        }
                        #else
                        cout <<"WARNING: top reward can't be defined because compiled with DO_NOT_USE_MOMDP."<<endl;
                        #endif
                    }
                    break;
                case 'R': // reward type
                    if (strlen(arg)>4) {  //expecting "-R?"
                        showHelp("unknown parameter / reward type: ",EXIT_FAILURE, arg);
                    } else {
                        switch(arg[2]) {
                            case 'f':
                                if (strlen(arg)==4) {
                                    switch(arg[3]) {
                                        case 'f':
                                            params.rewardType = SeekerHSParams::REWARD_FIND_SIMPLE;
                                            break;
                                        case 'd':
                                            params.rewardType = SeekerHSParams::REWARD_FIND_REV_DIST;
                                            break;
                                        default:
                                            showHelp("unknown parameter / reward type: ",EXIT_FAILURE, arg);
                                            break;
                                    }
                                } else {
                                    params.rewardType = SeekerHSParams::REWARD_FINAL;
                                }
                                break;
                            case 'c':
                                params.rewardType = SeekerHSParams::REWARD_FINAL_CROSS;
                                break;
                            case 't':
                                params.rewardType = SeekerHSParams::REWARD_TRIANGLE;
                                break;
                            default:
                                showHelp("unknown parameter / reward type: ",EXIT_FAILURE, arg);
                                break;
                        }
                    }
                    break;
                case 'v': //version
                    showVer=true;
                    break;
                case 'd': { //max depth
                    if (strlen(arg)==2) {
                        if (i+1>=argc) showHelp("expected maximum depth");
                        QString depthStr(argv[++i]);
                        bool ok = false;
                        params.maxTreeDepth = depthStr.toInt(&ok);
                        if (!ok) showHelp("expected max depth instead of ",EXIT_FAILURE,argv[i]);
                    } else if(strlen(arg)==3) {
                        switch(arg[2]) {
                            case 'c':
                                params.allowInconsistObs = true;
                                break;
                            case 'w':
                                runMode = MODE_WRITE_MAP;
                            case 'l': { //fallthrough OK
                                if (i+1>=argc) showHelp("expected distance matrix file");
                                QString dfile(argv[++i]);
                                params.mapDistanceMatrixFile = dfile.toStdString();
                                break;                            
                            }
                            default:
                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                                break;
                        }
                    }
                    break;
                }
                case 'M': { // max mem
                    if (strlen(arg)==3) {
                        char maxType = arg[2];

                        if (i+1>=argc) showHelp("max game steps/time requires pararameter");
                        QString numStr(argv[++i]);
                        bool ok = false;
                        long maxN = numStr.toLong(&ok);
                        if (!ok) showHelp("max game steps/time is not a long");

                        switch (maxType) {
                            case 'A':
                            case 'S':
                                params.maxNumActions = maxN;
                                break;
                            case 'T':
                                params.maxGameTime = maxN;
                                break;
                            default:
                                showHelp("unknown param",EXIT_FAILURE,arg);
                        }
                    } else {
                        if (i+1>=argc) showHelp("expected max memory");
                        QString maxmemStr(argv[++i]);
                        bool ok = false;
                        params.memoryLimit = maxmemStr.toInt(&ok);
                        if (!ok) showHelp("expected max memory instead of ",EXIT_FAILURE, argv[i]);
                        params.memoryLimit *= 1024*1024;
                    }
                    break;
                }
                case 'w': { //win dist
                    if (i+1>=argc) showHelp("expected win distance");
                    QString winDStr(argv[++i]);
                    bool ok = false;
                    params.winDist = winDStr.toDouble(&ok);
                    if (!ok) showHelp("expected win dist instead of ",EXIT_FAILURE,argv[i]);
                    break;
                }
                case 'n':
                    if (strlen(arg)!=3) {  //expecting "-n?"
                        showHelp("unknown parameter / number of ..: ",EXIT_FAILURE, arg);
                    } else {
                        if (arg[2]=='c') {
                            params.multiSeekerNoCommunication = true;
                            break;
                        } else {
                            if (i+1>=argc) showHelp("expected number distance");
                            QString numbStr(argv[++i]);
                            bool ok = false;
                            unsigned int numb = numbStr.toUInt(&ok);
                            if (!ok) showHelp("expected number instead of ",EXIT_FAILURE,argv[i]);

                            switch(arg[2]) {
                                case 'i':
                                    params.numBeliefStates = numb;
                                    break;
                                case 's':
                                    params.numSim = numb;
                                    break;
                                default:
                                    showHelp("unknown parameter / number of ..: ",EXIT_FAILURE, arg);
                                    break;
                            }
                        }
                    }
                    break;
                case 'e': {
                    if (strlen(arg)==2) {
                        if (i+1>=argc) showHelp("expand count");
                        QString expCountStr(argv[++i]);
                        bool ok = false;
                        params.expandCount = expCountStr.toUInt(&ok);
                        if (!ok) showHelp("expand count instead of ",EXIT_FAILURE,argv[i]);
                    } else if (strlen(arg)==3) {
                        switch(arg[2]) {
                            case 'w': {
                                if (i+3>=argc) showHelp("exploration weight requires 3 pararameter: util, dist, belief weight",EXIT_FAILURE);
                                bool ok=false;
                                QString dStr(argv[++i]);
                                params.multiSeekerExplorerUtilWeight = dStr.toDouble(&ok);
                                if (!ok) showHelp("exploration util weight expected");
                                dStr = QString::fromLatin1(argv[++i]);
                                params.multiSeekerExplorerDistWeight = dStr.toDouble(&ok);
                                if (!ok) showHelp("exploration distance weight expected");
                                dStr = QString::fromLatin1(argv[++i]);
                                params.multiSeekerExplorerBeliefWeight = dStr.toDouble(&ok);
                                if (!ok) showHelp("exploration belief weight expected");
                                break;
                            }
                            case 'm': {
                                if (i+1>=argc) showHelp("exploration max range expected",EXIT_FAILURE);
                                bool ok=false;
                                QString dStr(argv[++i]);
                                params.multiSeekerExplorerMaxRange = dStr.toDouble(&ok);
                                break;
                            }
                            case 'n': {
                                if (i+1>=argc) showHelp("exploration number highest of belief points expected",EXIT_FAILURE);
                                bool ok=false;
                                QString dStr(argv[++i]);
                                params.multiSeekerExplorerCheckNPoints = dStr.toInt(&ok);
                                break;
                            }
                            default:
                                showHelp("unknown/explorer parameter",EXIT_FAILURE,arg);
                                break;
                        }
                    } else {
                        showHelp("unknown parameter",EXIT_FAILURE,arg);
                    }
                    break;
                }
                case 'x': {
                    if (i+1>=argc) showHelp("exploration constant");
                    QString expConstStr(argv[++i]);
                    bool ok = false;
                    params.explorationConst = expConstStr.toDouble(&ok);
                    if (!ok) showHelp("exploration constant instead of ",EXIT_FAILURE,argv[i]);
                    break;
                }
                case 'E': {
                    if (strlen(arg)!=3) showHelp("expected reward type instead of ",EXIT_FAILURE,argv[i]);
                    switch (arg[2]) {
                        case 's':
                            params.expectedRewardCalc = SeekerHSParams::EXPECTED_REWARD_CALC_SUM;
                            break;
                        case 'n':
                            params.expectedRewardCalc = SeekerHSParams::EXPECTED_REWARD_CALC_NORM;
                            break;
                        default:
                            showHelp("expected reward type instead of ",EXIT_FAILURE,argv[i]);
                    }
                    break;
                }
                case 'I': //initial pos
                    if (strlen(arg)!=3) {  //expecting "-I?"
                        showHelp("unknown parameter / initial position type: ",EXIT_FAILURE, arg);
                    } else {
                        switch (arg[2]) {
                            case 'r':
                                params.initPosType =  HSGlobalData::INIT_POS_TYPE_RANDOM;
                                break;
                            case 'b':
                                params.initPosType =  HSGlobalData::INIT_POS_TYPE_BASE;
                                break;
                            case 'd': {
                                params.initPosType =  HSGlobalData::INIT_POS_TYPE_DIST;
                                if (i+1>=argc) showHelp("expected a position for the fixed initial position");
                                QString distStr = QString::fromStdString(argv[++i]);
                                bool ok = false;
                                params.randomPosDistToBase = distStr.toInt(&ok);
                                if (!ok) showHelp("expected a distance for the position type");
                                break;
                            }
                            case 'f': {
                                params.initPosType =  HSGlobalData::INIT_POS_TYPE_FIXED;
                                if (i+2>=argc) showHelp("expected a position for the fixed initial position");
                                QString rStr = QString::fromStdString(argv[++i]);
                                QString cStr = QString::fromStdString(argv[++i]);
                                bool ok = false;
                                int col = cStr.toInt(&ok);
                                int row = rStr.toInt(&ok);
                                if (!ok) showHelp("expected a distance for the position type");
                                if (params.initPosFixed!=NULL) {
                                    delete params.initPosFixed;                                    
                                }
                                params.initPosFixed = new Pos();
                                params.initPosFixed->set(row,col);

                                break;
                            }
                            default:
                                showHelp("unknown parameter / opponent type ",EXIT_FAILURE,arg);
                                break;
                        }

                        //check if randomPosDist for hider passed
                        if (i+1<argc) {
                            QString distStr = QString::fromStdString(argv[++i]);
                            bool ok = false;
                            params.randomPosDistToBaseHider = distStr.toInt(&ok);
                            if (!ok) showHelp("expected distance to base of init pos for the hider");
                        }

                    }
                    break;
                case 'c': //continuous std dev
                    if (strlen(arg)==2) {
                        if (i+1>=argc) showHelp("expected comments",EXIT_FAILURE, arg);
                        comments = QString::fromStdString(argv[++i]);
                    } if (strlen(arg)!=5 || (strlen(arg)>=5 && arg[2]!='s' && arg[2]!='p')) {  //expecting "-I?"
                        showHelp("unknown parameter / continuous std. dev. / probability",EXIT_FAILURE, arg);
                    } else {
                        //first get value
                        if (i+1>=argc) showHelp("expected a standard deviation or probability",EXIT_FAILURE, arg);
                        QString stddevStr = QString::fromStdString(argv[++i]);
                        bool ok = false;
                        double stdDev = stddevStr.toDouble(&ok);
                        if (!ok) showHelp("a standard deviation / probability expected",EXIT_FAILURE, arg);

                        switch(arg[2]) {
                            case 's': // standard deviation of noise
                                switch (arg[3]) {
                                    case 'n': // next state
                                        switch (arg[4]) {
                                            case 's': // seeker
                                                params.contNextSeekerStateStdDev = stdDev;
                                                break;
                                            case 'h': // hider
                                                params.contNextHiderStateStdDev = stdDev;
                                                break;
                                            default:
                                                showHelp("unknown parameter / std.dev. type ",EXIT_FAILURE,arg);
                                                break;
                                        }
                                        break;
                                    case 'o': // observation
                                        switch (arg[4]) {
                                            case 's': // seeker
                                                params.contSeekerObsStdDev = stdDev;
                                                break;
                                            case 'h': // hider
                                                params.contHiderObsStdDev = stdDev;
                                                break;
                                            default:
                                                showHelp("unknown parameter / std.dev. type ",EXIT_FAILURE,arg);
                                                break;
                                        }
                                        break;
                                    default:
                                        showHelp("unknown parameter / std.dev. type ",EXIT_FAILURE,arg);
                                        break;
                                }
                                break;
                            case 'p': //probability of false pos/neg
                                switch(arg[3]) {
                                   case 'f':
                                        switch(arg[4]) {
                                            case 'p':
                                                params.contFalsePosProb = stdDev;
                                                break;
                                            case 'n':
                                                params.contFalseNegProb = stdDev;
                                                break;
                                            default:
                                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                                                break;
                                        }
                                        break;
                                 case 'i': {
                                    switch(arg[4]) {
                                        case 'p':
                                            params.contIncorPosProb = stdDev;
                                            break;
                                        case 'u':
                                            params.contUpdInconsistAcceptProb = stdDev;
                                            break;
                                        default:
                                            showHelp("unknown parameter",EXIT_FAILURE,arg);
                                            break;
                                    }
                                    break;
                                 }
                                 case 'p': {
                                        switch(arg[4]) {
                                            case 'p':
                                                params.contUseHiderPredStepProb = stdDev;
                                                break;
                                            case 'n':
                                                params.contNextHiderHaltProb = stdDev;
                                                break;
                                            default:
                                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                                                break;
                                        }
                                        break;
                                 }
                                 default: {
                                    showHelp("unknown parameter",EXIT_FAILURE,arg);
                                    break;
                                }
                            }
                            break;
                        default:
                            showHelp("unknown parameter",EXIT_FAILURE,arg);
                            break;
                        }
                    }
                    break;
                case 'C':
                    params.useContinuousPos = true;
                    break;
                case 'k': { //autowalker
                    if (strlen(arg)!=3) {
                        showHelp("unknown parameter / auto walker type: ",EXIT_FAILURE, arg);
                    } else {
                        //type
                        switch (arg[2]) {
                            case 'r':
                                params.autoWalkerType = HSGlobalData::AUTOWALKER_RANDOM;
                                break;
                            case 's':
                                params.autoWalkerType = HSGlobalData::AUTOWALKER_SFM;
                                break;
                            case 'g':
                                params.autoWalkerType = HSGlobalData::AUTOWALKER_RANDOM_GOAL;
                                break;
                            case 'x':
                                params.autoWalkerType = HSGlobalData::AUTOWALKER_FIXED_POS;
                                break;
                            case 'f': {
                                params.autoWalkerType = HSGlobalData::AUTOWALKER_FILE;
                                if (i+1<argc && argv[i+1][0]!='-') {
                                    params.autoWalkerPosFile = argv[++i];
                                } else {
                                    showHelp("expecting auto walkers file");
                                }
                                break;
                            }
                            default:
                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                                break;
                        }

                        //check if n given
                        if (i+1<argc && argv[i+1][0]!='-') {
                            //assume the k is passed
                            QString nStr(argv[++i]);
                            bool ok = false;
                            params.autoWalkerN = nStr.toInt(&ok);
                            if (!ok) showHelp("expecting number of auto walkers / unknown parameter",EXIT_FAILURE,arg);
                        } else {
                            params.autoWalkerN = 1;
                        }
                    }

                    params.segmenterType = SeekerHSParams::SEGMENTER_KMEANS;
                    break;
                }
                case 'f': { //filter
                    if (strlen(arg)!=3) {
                        showHelp("unknown parameter / filter score type: ",EXIT_FAILURE, arg);
                    } else {
                        //type
                        switch (arg[2]) {
                            case 'w':
                                params.filterScoreType = HSGlobalData::FILTER_SCORE_NEW2_WEIGHTED;
                                break;
                            case 't':
                                params.filterScoreType = HSGlobalData::FILTER_SCORE_USE_TAG_ONLY;
                                break;
                            case 's': {
                                //get value
                                if (i+1>=argc) showHelp("expected number of max filter iterations",EXIT_FAILURE, arg);
                                QString maxItStr = QString::fromStdString(argv[++i]);
                                bool ok = false;
                                params.filterCanStopNumberOfIterations = maxItStr.toUInt(&ok);
                                if (!ok) showHelp("max iterations expected",EXIT_FAILURE, arg);
                                break;
                            }
                            case 'd': {
                                //get value
                                if (i+1>=argc) showHelp("expected follow distance",EXIT_FAILURE, arg);
                                QString fdStr = QString::fromStdString(argv[++i]);
                                bool ok = false;
                                params.followPersonDist = fdStr.toDouble(&ok);
                                if (!ok) showHelp("follow distance expected",EXIT_FAILURE, arg);
                                break;
                            }
                            default:
                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                                break;
                        }
                    }

                }
                break;
                case 'S': {
                    bool isStopSteps=true;
                    if (strlen(arg) == 3) {
                        switch(arg[2]) {
                            case 'n':
                                params.stopAtWin = false;
                                break;
                            case 'p': {
                                isStopSteps = false;
                                params.seekerPosFile = argv[++i];
                                break;
                            }
                        }
                    }

                    if (isStopSteps) {
                        if (i+1<argc) {
                            QString numStep = QString::fromStdString(argv[++i]);
                            bool ok = false;
                            params.stopAfterNumSteps = numStep.toUInt(&ok);
                            if (!ok) showHelp("num steps expected",EXIT_FAILURE, arg);
                        } else {
                            params.stopAfterNumSteps = 0;
                        }
                    }
                }
                    break;

                case 'N': //noise
                    if (strlen(arg)!=3 && strlen(arg)!=4) {
                        showHelp("unknown parameter / noise parameter",EXIT_FAILURE, arg);
                    } else {
                        //first get value
                        if (i+1>=argc) showHelp("expected a standard deviation or probability",EXIT_FAILURE, arg);
                        QString stddevStr = QString::fromStdString(argv[++i]);
                        bool ok = false;
                        double stdDev = stddevStr.toDouble(&ok);
                        if (!ok) showHelp("a standard deviation / probability expected",EXIT_FAILURE, arg);

                        switch(arg[2]) {
                            case 's': // standard deviation of noise
                                params.simObsNoiseStd = stdDev;
                                break;
                            case 'f': //probability of false pos/neg
                                if (arg[3]=='f') {
                                    switch(arg[4]) {
                                        case 'p':
                                            params.simObsFalsePosProb = stdDev;
                                            break;
                                        case 'n':
                                            params.simObsFalseNegProb = stdDev;
                                            break;
                                        default:
                                            showHelp("unknown parameter",EXIT_FAILURE,arg);
                                            break;
                                    }
                                } else {
                                    showHelp("unknown parameter",EXIT_FAILURE,arg);
                                }
                                break;
                            default:
                                showHelp("unknown parameter",EXIT_FAILURE,arg);
                                break;
                        }
                    }
                    break;
                case 'l':
                    if (strlen(arg)<3 || arg[2]!='d') {
                        showHelp("unknown parameter",EXIT_FAILURE, arg);
                    } else {
                        params.pomcpDoNotLearn = true;
                    }
                    break;
                case 'P':
                    if (strlen(arg)==3) {
                        switch(arg[2]) {
                            case 'P':
                                params.pathPlannerType = SeekerHSParams::PATHPLANNER_PROPDIST;
                                break;
                            case 'A':
                                params.pathPlannerType = SeekerHSParams::PATHPLANNER_ASTAR;
                                break;
                            default:
                                showHelp("unknown path planner/parameter",EXIT_FAILURE, arg);
                                break;
                        }
                    } else {
                        showHelp("unknown parameter",EXIT_FAILURE, arg);
                    }
                    break;
                default:
                    showHelp("unknown parameter: ",EXIT_FAILURE, arg);
            } //switch

        } // for argv


        /*TODO:    ssta   avg/min/max  : ssta/sstn/sstx
            smartSeekerScoreType = SmartSeeker::SCORE_AVG;
            smartSeekerMaxCalcsWhenHidden = 0;
        */


        if (showVer) { //only show version
            showVersion();
            if (argc==2) return 0;
        }

        //set log file
        QString logFile = "";
        QString logTimeFile = "";
        if (logFilePrefix==NULL) {
            logTimeFile = logFile = "hslog";
        } else {
            logTimeFile = logFile = logFilePrefix;
        }
        logTimeFile += "_time_log.txt";
        logFile += "_log.txt";

        //set username
        if (params.userName.length()==0) {
            params.userName = "autohsUser";
        }

        //AG121113: set map ID if not set and if pomdp is given
        if (params.pomdpFile.size()>0) {
            int mapIDFF = getMapIDFromFileName(params.pomdpFile);
            //check map id
            if (params.mapID==-1) {
                params.mapID = mapIDFF;
            } else if (params.mapID!=mapIDFF && mapIDFF!=-1) {
                cout << "WARNING: given map ID "<<params.mapID<<" but expected from the filename is "<<mapIDFF<<"!"<<endl<<endl;
            }
        }

        //ag130615
        SeekerHS* seekerHS = NULL;
        if (runMode==MODE_RUN_GAME || runMode==MODE_TEST) {
            seekerHS = new SeekerHS(&params, runMode==MODE_TEST);
        }

        //now show params
        cout    << "Run Mode:               " << RUN_MODES[runMode].toStdString();
        if (passSeekerHSToGameConnectorClient) {
            cout << " (testing SeekerHS";
            if (debugAddNewTracksManually) cout << " - debug";
            cout <<")";
        }
        cout <<endl;
        params.printVariables(cout, true);

        //check params
        if (runMode==MODE_TEST || runMode==MODE_TEST_SEGMENT || runMode==MODE_SHOW_MAP) {
            if (params.mapFile.length()==0)
                showHelp("map file required");
        }

        if (runMode == MODE_RUN_GAME) {
            if ( (params.serverIP.length()==0 || params.serverPort<=0) && params.opponentType==HSGlobalData::OPPONENT_TYPE_HUMAN )
                showHelp("server IP and port have to be set");
            if (!sendMap && ( params.mapID<0 || params.mapID>=HSGlobalData::MAPS.size() ) )
                showHelp("give a map ID, use '-A ?' for a list of the maps");
            //ag140605: checked in SeekerHS
            /*if (params.opponentType<0 || params.opponentType>=OPP_TYPES.size())
                showHelp("give a legal opponent type");*/
            if (sendMap && params.mapFile.length()==0)
                showHelp("a map file is expected when sending the map");
            if (sendMap && sendMapPGM && !basePos.isSet())
                showHelp("a base is required when loading a pgm map");
        }

        //return of main
        int ret = 0;

        switch (runMode) {
            case MODE_RUN_GAME: {
                // create map to send (if passed as param)
                GMap* mapToSend = NULL;
                if (sendMap) { //load map to send
                    if (sendMapPGM) {
                        cout << "Zoomoutfactor: "<<zoomOutFac<< endl;
                        mapToSend = new GMap(params.mapFile, basePos,zoomOutFac);
                        mapToSend->printMap();
                    } else {
                        mapToSend = new GMap(params.mapFile, &params);
                        if (basePos.isSet()) {
                            mapToSend->setBase(basePos);
                        }
                    }
                }

                //the autoplayer to be used by the game connector
                AutoPlayer* autoPlayer = seekerHS->getAutoPlayer();

                //ag131229: set distance to base for init pos
                if (params.randomPosDistToBase>0)
                    autoPlayer->setRandomPosDistToBase(params.randomPosDistToBase);

                /* //AG140613: set start pos
                if (!startPosFile.isEmpty()) {
                    FromListHider fromListHider(&params, startPosFile.toStdString());
                    fromListHider.
                    Pos initPos = fromListHider.getInitPos();
                    cout << "Init pos: "<<initPos.toString()<<endl;
                    autoPlayer->setInitPos(initPos);
                }*/

                //if using SeekerHS (for test/debug)
                SeekerHS* seekerHSparam = NULL;
                if (passSeekerHSToGameConnectorClient) seekerHSparam = seekerHS;

                //call game connector
                QString commentsToSend = "hsmomdp: "+QString::fromStdString(autoPlayer->getName());
                if (!comments.isEmpty()) commentsToSend += "; "+comments;
                GameConnectorClient gameConnector(&params, argsToQString(argc,argv), commentsToSend,
                                                  autoPlayer, mapToSend, seekerHSparam);

                gameConnector.debugAddNewTracksManually = debugAddNewTracksManually;

#ifdef GUI_DEBUG
                //create thread
                //AG140108: now QThread used directly
                //RunGameThread runGameConnectorThread(&gameConnector);
                //runGameConnectorThread.start();
                //difference with previous is that in the previous the gameconnector always was 'running', because it was
                //actively waiting for the new

                QThread *t = new QThread;
                //MyNetworkHandler *handler = new MyNetworkHandler;
                //handler->moveToThread(t);
                gameConnector.moveToThread(t);
                t->start();
                //connect
                bool runGC = gameConnector.startGame();
                int ret = 0;

                if (runGC) {
                    //get window instace
                    TreeWindow* twindow = TreeWindow::instance();

                    //give params of program (for showing)
                    /*stringstream str;
                    params.printVariables(str,true);
                    twindow->setParamValues(QString::fromStdString(str.str()));*/
                    twindow->setParams(&params, &gameConnector);

                    //show
                    twindow->show();

                    //execute Qt GUI
                    ret = a.exec();
                } else {
                    ret = -1;
                }

                cout << "Finished"<<endl;

                //delete twindow;
                //return ret;

#else
                gameConnector.setExitOnDisconnect(true);

                //start directly in this thread
                bool runGC = gameConnector.startGame();

                if (runGC) {
                    ret = a.exec();
                } else {
                    ret = -1;
                }

                //return ret;
#endif

                break;
            }
            case MODE_TEST: {
                testSeekerHS(seekerHS);
                break;
            }
            case MODE_TEST_SEGMENT:
                //testSegmentation(mapFile,segmenter, robotRow, robotCol, beliefTestFile, &params);
                break;

            case MODE_SHOW_MAP_LIST:
                showMaps();
                break;

            case MODE_SHOW_MAP:
                showMap(params.mapFile, params.solverType==SeekerHSParams::SOLVER_POMCP, &params);
                break;
            case MODE_TEST_POMCP:
                testPOMCP(params.mapFile, &params,false);
                break;
            case MODE_TEST_MCTS:
                testPOMCP(params.mapFile, &params,true);
                break;
            case MODE_TEST_PEOPLEPRED:
                testPeoplePrediction(&params);
                break;
            case MODE_WRITE_MAP:
                if (params.mapDistanceMatrixFile.length()==0) {
                    writeMap(params.mapFile, mapFileOut, basePos, zoomOutFac, cellSize, rowDist_m, colDist_m, mapIsYalm, &params);
                } else {
                    writeDistMat(&params);
                }
                break;
            case MODE_GUI_TREE: {
#ifdef GUI_DEBUG
                TreeWindow* twindow = TreeWindow::instance();

                //show
                twindow->show();

                //execute Qt GUI
                ret = a.exec();
                delete twindow;
                //return ret;
#else
                cout << "Error: cannot run GUI, because the project has not bee compiled with GUI (DEBUG_GUI flag)."<<endl;
#endif

                break;
            }
        }

        if (seekerHS!=NULL)
            delete seekerHS;


    } catch(bad_alloc &e) {
        #ifndef DO_NOT_USE_MOMDP
        if(GlobalResource::getInstance()->solverParams.memoryLimit == 0) {
        #endif
            cout << "Memory allocation failed. Exit." << endl;
        #ifndef DO_NOT_USE_MOMDP
        } else {
            cout << "Memory limit reached. Please try increase memory limit" << endl;
        }
        #endif
    } catch(CException &ce) {
        cout << "CException: " << ce.what() << endl ;
    } catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }


    return 0;
}


#ifndef DO_NOT_USE_MOMDP
void testHSMOMDP(HSMOMDP* hsmomdp, char* mapFile, SeekerHSParams* params) {
    cout << " TEST mode " << endl;

    GMap map(mapFile,params);
    Player *p = new Player(&map);

    cout << "Loading map "<<endl;
    //map->readMapFile(mapFile);
    //p->sethidertype(-1);
    p->setType(2);
    p->setCurPos(0,0);
    p->setPlayer2Pos(3,3);
    //map.setInitialh(0,0);
    //map.setInitials(3,3);


    Pos inith(0,0); //map.getInitialh();
    Pos inits(map.getBase()); //map.getInitials();
    bool visib = map.isVisible(inits,inith,false);

    //map->printMap();

    //AG130228: disable
    /*
    cout <<"ok\ncalc visib:"<<endl;
    p->calculatevisible();
    cout << "ok"<<endl;*/

    //map->printMap(true);
    p->printInfo();

    cout << "--------" <<endl<<"init belief:"<<endl;
    bool ok = hsmomdp->initBelief(&map,inits,inith,visib);

    if (ok == false) {
        cout << "Failed to load the initial belief" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "ok loaded"<<endl;

    cout << "--------" <<endl<<"first action:"<<endl;

    // loop run
    //HALT_ACT; N_ACT; NE_ACT; E_ACT; SE_ACT; S_ACT; SW_ACT; W_ACT; NW_ACT
    //const string actions[5] = {"h","n","s","e","w"};
    const string actions[9] = {"h","n","ne", "e", "se","s","sw","w","nw"};

    Pos x,y;
    int isVis;
    //int unseenI = map.numFreeCells();
    int action = 0;
    bool stop=false;
    string cmd;


    //TODO CHECK WITH A POMDP!!!

    while (!stop) {
        cout << "now enter pos of hider,seeker and if visible, enter <0 to quit"<<endl<<endl;

        int row,col;
        cout << "Seeker row: ";
        cin >> row;
        //STOPQRY(x.x);
        if (row<0) break; //return 0;
        cout << " col: ";
        cin >> col;
        testx(col);
        x.set(row,col);

        cout << endl << "Hider row: ";
        cin >> row;
        testx(row);
        cout << " col: ";
        cin >> col;
        testx(col);
        y.set(row,col);

        cout << endl << "Hider visible? (1=true,0=false): ";
        cin >> isVis;
        testx(isVis);

        cout << "Calc action ..."<<endl;
        action = hsmomdp->getNextAction(x,y,isVis!=0,-1);
        cout<<endl << "Action: "<<action << " ("<<actions[action]<<")"<<endl<<endl;
    }
}
#endif

void testSeekerHS(SeekerHS* seeker) {
    cout << " TEST SeekerHS " << endl;

    GMap* map = seeker->getMap();
    //Player *p = new Player(map);
    bool visib;

    cout << "pre init: "<<endl;

    map->printMap();

    /*int multipleHiderPosInt = 0;
    cout << "Multiple hider pos (1=true,0=false): "<<flush;
    cin >> multipleHiderPosInt;
    bool multipleHiderPos = (multipleHiderPosInt==1);
    if (multipleHiderPos) cout << "Multiple hider pos.: "<< (multipleHiderPos?"true":"false")<<endl;*/

    vector<double> seekerPos,emptyPos,newSeekerPos; //hiderPos
    vector< vector<double> > mulHiderPos;
    seekerPos.resize(2);
    //hiderPos.resize(2);
    emptyPos.resize(0);
    newSeekerPos.resize(3);

    cout << "--------" <<endl<<"init belief:"<<endl;
    //bool ok = hsmomdp->initBelief(&map,inits,inith,visib);

    bool ok=false;
    int nHPos = 1;

    while (!ok) {
        try {
            cout <<"Seeker init (row,col): ";
            cin >> seekerPos[0];
            cin >> seekerPos[1];

            //if (multipleHiderPos) {
            cout << "Num of hider pos: "<<flush;
            cin >> nHPos;
            //}

            mulHiderPos.resize(nHPos);
            for(int i=0; i<nHPos; i++) {
                mulHiderPos[i].resize(3);
                cout <<"Hider init (row,col) (-1 if hidden): ";
                cin >> mulHiderPos[i][0];
                /*visib = hiderPos[0]>=0;
                if (visib)*/
                cin >> mulHiderPos[i][1];
                cout << " q: ";
                cin >> mulHiderPos[i][2];
            }
            //if (mulHiderPos.size()==0) visib=false;
            visib=(mulHiderPos.size()>0);

            cout << "init s:"<<seekerPos[0]<<","<<seekerPos[1]<<" h:";

            //if (visib) {
            if (visib) {
                cout <<mulHiderPos[0][0]<<","<<mulHiderPos[0][1]<<" visib "<<endl;
            } else {
                cout << "hidden"<<endl;
            }

            //if (multipleHiderPos) {
            seeker->initMultipleObs(seekerPos,mulHiderPos);
            /*} else {
                seeker->init(seekerPos,hiderPos);
            }*/

            /*} else {
                cout <<" hidden:"<<endl;
                if (multipleHiderPos) {
                    seeker->initMultipleObs(seekerPos,hiderPos);
                } else {
                    seeker->init(seekerPos,emptyPos);
                }
            }*/
            cout<<"ok"<<endl;
            ok = true;

        } catch(CException& ce) {
            cout << "CEException: "<<ce.what()<<endl;
            cout <<"TRY AGAIN!"<<endl;
        }
    }

    cout << "ok loaded"<<endl;

    cout << "--------" <<endl<<"first action:"<<endl;

    // loop run
    //HALT_ACT; N_ACT; NE_ACT; E_ACT; SE_ACT; S_ACT; SW_ACT; W_ACT; NW_ACT
    //const string actions[5] = {"h","n","s","e","w"};
    const string actions[9] = {"h","n","ne", "e", "se","s","sw","w","nw"};

    //Pos x,y;
    //int isVis;
    //int unseenI = map.numFreeCells();
    //int action = 0;
    bool stop=false;
    //string cmd;
    int winState;
    Pos sPos,hPos;
    vector<int> actVec;



    //TODO CHECK WITH A POMDP!!!

    while (!stop) {
        cout << "Store belief map as image:"<<endl;
        seeker->getAutoPlayer()->storeMapBeliefAsImage("test.png",sPos,NULL,hPos,NULL);
        cout << "String map output:"<<endl;
        cout << seeker->getAutoPlayer()->getMapBeliefProbAsString(map->rowCount(),map->colCount())<<endl;



        cout << "Calc action ..."<<endl;
        //action = hsmomdp->getNextAction(x,y,isVis!=0);

        sPos.set(seekerPos[0],seekerPos[1]);
        if (visib) {
            //hider pos
            hPos.set(mulHiderPos[0][0], mulHiderPos[0][1]);
        }

        int status = seeker->getWinState(sPos,hPos,visib);

        if (status>HSGlobalData::GAME_STATE_RUNNING) {
            stop = true;
            cout << "Game finished in "<<GAME_STATES[status+1].toStdString()<<endl;
        } else {
            try {

                actVec = seeker->getNextMultiplePosesForMultipleObs(seekerPos,mulHiderPos,nHPos,newSeekerPos,&winState);

                /*
                if (mulHiderPos) {
                    if (visib)
                        //action = seeker->getNextPose(seekerPos,hiderPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePosesForMultipleObs(seekerPos,hiderPos,3,newSeekerPos,&winState);
                    else
                        //action = seeker->getNextPose(seekerPos,emptyPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePosesForMultipleObs(seekerPos,emptyPos,3,newSeekerPos,&winState);
                } else {
                    if (visib)
                        //action = seeker->getNextPose(seekerPos,hiderPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePoses(seekerPos,hiderPos,3,newSeekerPos,&winState);
                    else
                        //action = seeker->getNextPose(seekerPos,emptyPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePoses(seekerPos,emptyPos,3,newSeekerPos,&winState);
                }*/

                cout<<endl << "Action: "<<actVec[0] << " ("<<actions[actVec[0]]<<")"<<endl<<endl;


                cout << "new seeker pos: "<<newSeekerPos[0]<<","<<newSeekerPos[1]<<"; win state="<<winState<<endl;

                if (winState!=HSGlobalData::GAME_STATE_RUNNING) {
                    cout<<"end of game"<<endl;
                    break;
                }

                cout << "next actions: ";
                for(vector<int>::size_type i=1; i<actVec.size(); i++) {
                    cout << actions[actVec[i]] << " ("<<newSeekerPos[i*3]<<","<<newSeekerPos[i*3+1]<<","<<newSeekerPos[i*3+2]<<"º) ";
                }
                cout <<endl;
            } catch(CException& ce) {
                cout << "CEException: "<<ce.what()<<endl;
                cout <<"TRY AGAIN!"<<endl;
            }

            cout << "now enter pos of hider,seeker and if visible, enter <0 to quit"<<endl<<endl;


            cout <<"Seeker init (row,col): ";
            cin >> seekerPos[0];
            cin >> seekerPos[1];

            //if (multipleHiderPos) {
            cout << "Num of hider pos: "<<flush;
            cin >> nHPos;
            //}

            mulHiderPos.resize(nHPos);
            for(int i=0; i<nHPos; i++) {
                mulHiderPos[i].resize(3);
                cout <<"Hider init (row,col) (-1 if hidden): ";
                cin >> mulHiderPos[i][0];
                /*visib = hiderPos[0]>=0;
                if (visib)*/
                cin >> mulHiderPos[i][1];
                cout << " q: ";
                cin >> mulHiderPos[i][2];
            }
            //if (mulHiderPos.size()==0) visib=false;
            visib=(mulHiderPos.size()>0);


            /*cout <<"Hider init (row,col) (-1 if hidden): ";
            cin >> hiderPos[0];
            visib = hiderPos[0]>=0;
            if (visib)
                cin >> hiderPos[1];*/
        }

    }
}

#ifdef old_test
void testSeekerHS(SeekerHS* seeker) {
    cout << " TEST SeekerHS " << endl;

    GMap* map = seeker->getMap();
    //Player *p = new Player(map);
    bool visib;

    cout << "pre init: "<<endl;

    map->printMap();

    int multipleHiderPosInt = 0;
    cout << "Multiple hider pos (1=true,0=false): "<<flush;
    cin >> multipleHiderPosInt;
    bool multipleHiderPos = (multipleHiderPosInt==1);
    if (multipleHiderPos) cout << "Multiple hider pos.: "<< (multipleHiderPos?"true":"false")<<endl;

    vector<double> seekerPos,hiderPos,emptyPos,newSeekerPos;
    vector< vector<double> > mulHiderPos;
    seekerPos.resize(2);
    hiderPos.resize(2);
    emptyPos.resize(0);
    newSeekerPos.resize(3);

    cout << "--------" <<endl<<"init belief:"<<endl;
    //bool ok = hsmomdp->initBelief(&map,inits,inith,visib);

    bool ok=false;
    int nHPos = 1;

    while (!ok) {
        try {
            cout <<"Seeker init (row,col): ";
            cin >> seekerPos[0];
            cin >> seekerPos[1];

            if (multipleHiderPos) {
                cout << "Num of hider pos: "<<flush;
                cin >> nHPos;
            }

            hiderPos.resize(nHPos);
            for(int i=0; i<nHPos; i++) {
                cout <<"Hider init (row,col) (-1 if hidden): ";
                cin >> hiderPos[i*2 + 0];
                visib = hiderPos[0]>=0;
                if (visib)
                    cin >> hiderPos[i*2 + 1];
            }
            if (hiderPos.size()==0) visib=false;

            cout << "init s:"<<seekerPos[0]<<","<<seekerPos[1]<<" h:";

            if (visib) {
                cout <<hiderPos[0]<<","<<hiderPos[1]<<" visib:"<<endl;

                if (multipleHiderPos) {
                    seeker->initMultipleObs(seekerPos,hiderPos);
                } else {
                    seeker->init(seekerPos,hiderPos);
                }

            } else {
                cout <<" hidden:"<<endl;
                if (multipleHiderPos) {
                    seeker->initMultipleObs(seekerPos,hiderPos);
                } else {
                    seeker->init(seekerPos,emptyPos);
                }
            }
            cout<<"ok"<<endl;
            ok = true;

        } catch(CException& ce) {
            cout << "CEException: "<<ce.what()<<endl;
            cout <<"TRY AGAIN!"<<endl;
        }
    }

    cout << "ok loaded"<<endl;

    cout << "--------" <<endl<<"first action:"<<endl;

    // loop run
    //HALT_ACT; N_ACT; NE_ACT; E_ACT; SE_ACT; S_ACT; SW_ACT; W_ACT; NW_ACT
    //const string actions[5] = {"h","n","s","e","w"};
    const string actions[9] = {"h","n","ne", "e", "se","s","sw","w","nw"};

    //Pos x,y;
    //int isVis;
    //int unseenI = map.numFreeCells();
    int action = 0;
    bool stop=false;
    //string cmd;
    int winState;
    Pos sPos,hPos;
    vector<int> actVec;



    //TODO CHECK WITH A POMDP!!!

    while (!stop) {
        cout << "Calc action ..."<<endl;
        //action = hsmomdp->getNextAction(x,y,isVis!=0);

        sPos.row = seekerPos[0];
        sPos.col = seekerPos[1];
        hPos.row = hiderPos[0];
        hPos.col = hiderPos[1];

        int status = seeker->getWinState(sPos,hPos,true);

        if (status==SeekerHS::STATE_TIE || status==SeekerHS::STATE_WIN_HIDER || status==SeekerHS::STATE_TIE) {
            stop = true;
            cout << "Game finished in "<<GAME_STATES[status+1].toStdString()<<endl;
        } else {
            try {

                if (multipleHiderPos) {
                    if (visib)
                        //action = seeker->getNextPose(seekerPos,hiderPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePosesForMultipleObs(seekerPos,hiderPos,3,newSeekerPos,&winState);
                    else
                        //action = seeker->getNextPose(seekerPos,emptyPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePosesForMultipleObs(seekerPos,emptyPos,3,newSeekerPos,&winState);
                } else {
                    if (visib)
                        //action = seeker->getNextPose(seekerPos,hiderPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePoses(seekerPos,hiderPos,3,newSeekerPos,&winState);
                    else
                        //action = seeker->getNextPose(seekerPos,emptyPos,newSeekerPos,&winState);
                        actVec = seeker->getNextMultiplePoses(seekerPos,emptyPos,3,newSeekerPos,&winState);
                }

                cout<<endl << "Action: "<<actVec[0] << " ("<<actions[actVec[0]]<<")"<<endl<<endl;


                cout << "new seeker pos: "<<newSeekerPos[0]<<","<<newSeekerPos[1]<<"; win state="<<winState<<endl;

                if (winState!=SeekerHS::STATE_PLAYING) {
                    cout<<"end of game"<<endl;
                    break;
                }

                cout << "next actions: ";
                for(int i=1; i<actVec.size(); i++) {
                    cout << actions[actVec[i]] << " ("<<newSeekerPos[i*3]<<","<<newSeekerPos[i*3+1]<<","<<newSeekerPos[i*3+2]<<"º) ";
                }
                cout <<endl;
            } catch(CException& ce) {
                cout << "CEException: "<<ce.what()<<endl;
                cout <<"TRY AGAIN!"<<endl;
            }

            cout << "now enter pos of hider,seeker and if visible, enter <0 to quit"<<endl<<endl;


            cout <<"Seeker init (row,col): ";
            cin >> seekerPos[0];
            cin >> seekerPos[1];
            cout <<"Hider init (row,col) (-1 if hidden): ";
            cin >> hiderPos[0];
            visib = hiderPos[0]>=0;
            if (visib)
                cin >> hiderPos[1];
        }

    }
}
#endif

void showHelp(string error, int exitCode, char* arg) {
    if (!error.empty()) {
        cout << "Error: " << error;
        if (arg == NULL) {
            cout << endl;
        } else {
            cout << " " << arg << endl;
        }
        cout << endl;
    }

    cout << "hsmomdp [-t | -ts | -tm | -tc | -tC | -tg | -tp | -h | -v] [ -Gh | -Gf[2] ] [-so | -sl | -slc | -sm | -ss | -sM | -sc[c|d|p] | -sf[e] | -sC[c|h|H] | -sh ] [-Rf | -Rc | -Rt | -Rff | -Rfd ] [-D discount] [-gt | -gb | -gk [k] | -g[r|c][x] "
         << "[d a b h]] [-m momdp-file] [-p policy-file] [-a[s|p] map-file] [-A map-id] [-b row col] [-u username] [-s server port] [-PP | -PA] "
         << "[-oh | -or | -ow | -os | -ov | -oa | -oA | -ox | -of [file]] -dc [-L log-prefix] [-um | -uf] [-pt target-prec] [-pi target-init-fac] "
         << "[-pd] [-pp] [-rs|-ra|-rn|-rx] [-d max_depth] [-M max-mem] [-w win-dist] [-ni num_init_belief_points "
         << "-ns num_sim x expl_const -e expand_cnt -d max_search_depth [-hsr -hsb -hss]] [-dc] [-sss[a|x|n]] [-ssm max_cells] "
         << "[-I[r|b|d|f] [dist | r c]] [ -rp[r|s] ] [-MS max_steps | -MT max-time-sec] [-ud] [-k[r|s|x] [n] | -kf file] [-f[w|t]] [-S[n] [num-steps] "
         << "[-d[w|l] file] [-us steps] [-c comments] [-Ns std-dev | -Nfn false-neg-prob | -Nfp false-pos-prob] [-Sp start-pos-file] "
         << "[-ppf dest-file | -pph horiz-time] | -bi belief-img | -ew utilw distw bel_w | -em expl_max_r | -en expl_n_bel | -mo own_obs_p | -md pos-small-d] [-bf] "
         << "| [-fd follow-dist] | [-nc] | [-ld]" << endl
         << "   -t                test manual input of data" << endl
         << "   -ts [belief-file] test segmentation, if belief-file not given than random number used" << endl
         << "   -tm               test map, show the map" << endl
         << "   -t[c|C]           test POMCP simulator / MCTS"<<endl
         << "   -tp               test people prediction"<<endl
         << "   -tg               show Tree GUI"<<endl
         << "   -t[S|D|N]         S/D: use SeekerHS (for testing); D: manually add tracks; N: don't send second seeker"<<endl
         << "   -h                this help" << endl
         << "   -G[h|f[2]]        game of hide-and-seek or find-and-follow (-Gf2: for 2 seekers)"<<endl
         << "   -P[P|A]           path planner: propagation (default) / A*"<<endl
         << "   -v                shows version and build" << endl
         << "   -so               solver: uses offline learned policy" << endl
         << "   -sl               solver: layered MOMDP" << endl
         << "   -slc              solver: layered MOMDP, but compare with offline" << endl
         << "   -sm               solver: MCVI (offline) [not implemented yet]"<<endl
         << "   -sc[d|c|p]        solver: POMCP (online) uses MCTS (discrete/continuous states/continuous with predictor)"<<endl
         << "   -ss               smart seeker"<<endl
         << "   -sf[e]            follower / exact: when not visible, goes to last point"<<endl
         << "   -sh[d|c|p]        solver: follower of highest belief of POMCP (discrete/continuous states/continous with prediction)"<<endl
         << "   -sC               solver: combi POMCP and follower"<<endl
         << "   -sCc              solver: combi POMCP and follower, with continuous states"<<endl
         << "   -sCh              solver: combi highest POMCP belief follower and follower (cont. states)"<<endl
         << "   -sCH              solver: combi highest POMCP belief follower and follower (cont. states and person prediction)"<<endl
         << "   -sM[c|d]          solver: muti (2) robot highest POMCP belief explorer (c/d: continuous/discrete)"<<endl
         << "   -W map_file       write map to map file"<<endl
         << "   -R[f|c|t]         Rewards: f=final, c=final and crossing, t=triangle"<<endl
         << "   -Rf[f|d]          Rewards Find&Follow: fs=simple, fd=rev. distance"<<endl
         << "   -D discount       discount factor (gamma)"<<endl
         << "   -bl               use best action look ahead"<<endl
         << "   -m momdp-file     MOMDP file or map name (without extension - should be on server!)" << endl
         << "   -p policy-file    policy file" << endl
         << "   -a[s] file        load map file, -as: send map to server" << endl
         << "   -ap file zoom-f   load map PGM file, using zoom-factor"<<endl
         << "   -ay file cell-sz  load map PGM file, passing cell size (m) and map size of rowD x colD m"<<endl
         << "       rowD colD     "<<endl
         << "   -A [map-id | ?]   map ID (? shows list of maps and IDs)" << endl
         << "   -b row col        base (row,col), overwrites base in file" << endl
         << "   -u user           username of the player in the game" << endl
         << "   -s server port    server and port (if 0 then no server is used; default: localhost:" << SeekerHSParams::DEFAULT_SERVER_PORT <<")" << endl
         << "   -o[h|r|s|v|a|w]   opponent: Human/Random/Smart hider/Very Smart Hider/All knowing hider/Random walker (has goals)" << endl
         << "     -o[A|2|T]       All know. Very Smart H./Very Smart H. v2/All know. Very Sm. H. v2" << endl
         << "     -ol act-file    Action list with file name (no path)" <<endl
         << "     -of pos-file    Position file"<<endl
         << "     -ox             opponent pos random fixed"<<endl
         << "   -L log-file-pre   log file prefix"<<endl<<endl
         << "                     The folowing parameters are for the online solver only:" << endl
         << "   -T timeout        solver time out in seconds (default: 0, not taken into account)" << endl
         << "   -Tr               set top reward (if not then uses average of bottom)" << endl
         << "   -Tf               set top final state (if not uses average of bottom)" << endl
         << "   -gb | -gk [k] |   segmenter: basic (3 segments: <0,=0,>0) / k-means (default k based on map size and #obstacles) /" <<endl
         << "   -gr [d a b h]     robot centered with 'circles'/'squares' of a certain distance d and crossing with certian angle a," <<endl
         << "                     center node has radius b (0=only center node), and h is de radius from which no segmentation is done"<<endl
         << "                     (i.e. high resolution cells are taken)"<<endl
         << "   -gc [d a b h]     combines robot centered and base centered, same params as -gr"<<endl
         << "   -grx [d a b h]    same as -gr, but for the X states (i.e. robot's position)"<<endl
         << "   -gvr|-gvb|-gvbr   segment value: reward only / belief only / belief*reward" <<endl
         << "   -gt               segmenter test (uses same 'segmentation' as map)"<<endl
         << "   -um | -uf         initializer upper bound (see APPL): (Q)MDP (-um) or FIB (-uf, default)" <<endl
         #ifndef DO_NOT_USE_MOMDP
         << "   -pt target-prec   set the target precision for the MOMDP solver (default=" << SolverParams::DEFAULT_TARGET_PRECISION <<")"<<endl
         << "   -pi init-t-fac    init target procesion factor for calculating upper and lower bounds (default="<<SolverParams::CB_INITIALIZATION_PRECISION_FACTOR<<")"<<endl
         #endif
         << "   -pd               pruning disabled, i.e. no pruning (for debug purpuse)"  <<endl
         << "   -pp               print (APPL solver) parameters before solving" <<endl
         << "   -rs|-ra|-rn|-rx   top reward aggregation: sum / average / min / max"<<endl
         << "   -d max-depth      maximum depth to search belief tree"<<endl
         << "   -M max-memory-MB  maximum memory to be used by SARSOP (MB)"<<endl
         << "   -w win_dist       maximum distance to hider for seeker to win (default: 0)"<<endl
         << "   -ni numb_init_bel number of init beliefs, if 0 (default) all belief points of init will be used"<<endl
         << "   -ns numb_sims     number of simulations"<<endl
         << "   -x expl_constant  exploration constant"<<endl
         << "   -e expand_count   expand count (after so many counts node is expanded)"<<endl
         << "   -dc               disable consistency check / allow inconcistencies"<<endl
         << "   -ssa[a|x|n]       smart seeker score type for actions opponents: average/max/min"<<endl
         << "   -ssh[a|x|n]       smart seeker score type for hidden positions: average/max/min"<<endl
         << "   -ssm max_cells    smart seeker max. cells to calculate when hidden (random choice)"<<endl
         << "   -hs[r|b|s] [rp]   hider simulator: random/directly to base/smart, rp=prob. of using random action"<<endl
         << "   -sci              set init node value for POMCP (default false)"   <<endl
         << "   -E[s|n]           expected reward count: sum (default) or normalized"<<endl
         << "   -I[r|b]           set initial position to be random for both, or random for the hider and seeker for the base"<<endl
         << "   -Id dist          set initial to be random at a given distance d from the base"   <<endl
         << "   -If r c           set the initial position to be fixed: (r,c)" << endl
         << "   -rp[r|s]          roll-out policy: random/smart(seeker)" << endl
         << "   -csn[s|h]         std_dev continuous std. dev. for next step seeker / hider (default = "<<SeekerHSParams::CONT_NEXT_SEEKER_STATE_STDDEV<<" / "<<SeekerHSParams::CONT_NEXT_HIDER_STATE_STDDEV<<")" << endl
         << "   -cso[s|h]         std_dev continuous std. dev. for observation seeker / hide (default = "<<SeekerHSParams::CONT_SEEKER_OBS_STDDEV<<" / "<<SeekerHSParams::CONT_HIDER_OBS_STDDEV<<")" << endl
         << "   -cpf[p|n]         probability for false positive / negative of detecting a person"<<endl
         << "   -cpip             probability for incorrect positive of detecting a person (default = "<<SeekerHSParams::CONT_INCOR_POS_PROB<<")" << endl
         << "   -cppn             next hider halt probability (default = "<<SeekerHSParams::CONT_NEXT_HIDER_HALT_PROB<<")" << endl
         << "   -cppp             use prediction step for next pos. probability (default = "<<SeekerHSParams::CONT_USE_HIDER_PRED_STEP_PROB<<")" << endl
         << "   -cpiu             probability for belief update to accept inconsistent (with obs) steps (default = "<<SeekerHSParams::CONT_UPD_INCONSIST_ACCEPT_PROB<<")"<<endl
         << "   -C                continuous space" << endl
         << "   -t[S|D]           Use SeekerHS to test it, D: debug, enter tracks manually"<<endl
         << "   -MS max-steps     max. steps"<<endl
         << "   -MT max-time-sec  max. time in seconds"<<endl
         << "   -ud               use deduced action"<<endl
         << "   -k[r|s|g|x] [n]   automated walkers/dyn. obstacles: random / social force model / random goal / fixed pos; n walkers (default 1)"<<endl
         << "   -kf file          automated file reading data from file"<<endl
         << "   -f[w|t]           filter all possible hider poses: weighted score / tag only"<<endl
         << "   -fs num-it        filter input tracks at maximum num-it iterations (using observation score and distance)"<<endl
         << "   -S[n] num-steps   stop after num-steps (0 is default, not stopping); -Sn: do not stop after win"<<endl
         << "   -d[w|l] file      write/load distance file"<<endl
         << "   -bz zoom-fac      belief zoom factor"<<endl
         << "   -us update-steps  number of steps after which the highest belief is checked and goal is updated"<<endl
         << "   -c comments       comments, use quotes (\") when writing spaces"<<endl
         << "   -Ns std-dev       noise std.dev. for osbervation of hider's pos. sent to seeker"<<endl
         << "   -Nfn false-neg-p. false negative prob. for obs. of hider's pos sent to seeker"<<endl
         << "   -Nfp false-pos-p. false positive prob. for obs. of hider's pos sent to seeker"<<endl
         << "   -Sp start-posfile start pos file (made with autohider file type "<<endl
         << "   -ppf dest-file    destination file for people preditor"<<endl
         << "   -pph t-horizon    time-horizon for people predictor"<<endl
         << "   -bi belief-image  image file of belief (png, default: out.png)"<<endl
         << "   -ew uw dw bw      set weights for explorer formula: util, distance, belief weight"<<endl
         << "   -em expl-max-r    explorer max range"<<endl
         << "   -en expl-n-belp   explorer use n belief points"<<endl
         << "   -md pos-small-d   multi: small distance for selecting poses"<<endl
         << "   -mo own-obs-p     multi: own observation probability"<<endl
         << "   -bf               filter belief in update retrieving from root (important for multi)" <<endl
         << "   -fd follow-dist   follow distance (default: "<<SeekerHSParams::FOLLOW_PERSON_DIST<<")"<<endl
         << "   -mr min-rob-dist  min. robot distance (default: "<<SeekerHSParams::MIN_DIST_BETW_ROBOTS<<")"<<endl
         << "   -nc               no communication, i.e. don't use other's observations"<<endl
         << "   -ld               disable learning for POMCP, only belief update"<<endl //TODO!!
         //<<
         << endl;

    if (exitCode != -1) {
        exit(exitCode);
    }
}

string inline showOnOff(bool b) {
    return (b?"on":"off");
}

void showVersion() {
    cout << "Hide&Seek client v"<<HS_VERSION<<endl<<endl;
    cout << "Build: "<<__DATE__<<" "<<__TIME__<<endl<<endl;
    cout << "Compiled with: "<<endl;
    #ifndef DO_NOT_USE_MOMDP
    cout << " - " << SolverParams::getAPPLVersion() <<endl;
    #endif
    cout << " - Qt " << QT_VERSION_STR <<endl;
    cout << " - OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION<<endl<<endl;
    cout << "GNU compiler version: " <<__VERSION__ << " (" << _MACHINEBITS << " bits)" <<endl;//<<__GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__<<endl;
    if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
    else if( __cplusplus == 199711L ) std::cout << "C++98\n" ;
    else std::cout << "pre-standard C++\n" ;
    cout << " (C++ version: "<<__cplusplus<<")"<<endl;
#if __cplusplus>=201103
    cout <<"New C++, 2011"<<endl;
#else
    cout <<"Old C++"<<endl;
#endif
    cout << endl << "Debug flags: ";
    DEBUG_HS(cout <<"debug (DEBUG_HS); ";);
    DEBUG_HS_INIT(cout <<"init debug (DEBUG_HS_INIT); ";);
    DEBUG_HS1(cout << "detailed debug (DEBUG_HS1); ";);
    DEBUG_MAP(cout << "map debugging (DEBUG_MAP); ";);
    DEBUG_SEGMENT(cout << "segment debug (DEBUG_SEGMENT); ";);
    //cout << "Show transition probabilities: "<<showOnOff(DEBUG_SHOW_TRANS)<<endl;
    #ifdef DO_NOT_USE_MOMDP
    cout<<"not using MOMDP (DO_NOT_USE_MOMDP);";
    #else
    cout << "APPL debug: " << showOnOff(SolverParams::isDebugOn()) << "; debug trace: "<<showOnOff(SolverParams::isDebugTraceOn()) <<endl;
    #endif
    cout << endl;
    cout << "Random (1-10): "<<random(1,10)<<endl;
}

#ifndef DO_NOT_USE_MOMDP
void testSegmentation(char* mapFile,Segmenter* segmenter, int row, int col, char* beliefTestFile, SeekerHSParams* params) {
    cout << "TEST MODE segmentation"<<endl;
    GMap gmap(mapFile,params);
    cout << "Map:"<<endl;
    gmap.printMap();
    int freeCells = gmap.numFreeCells();
    cout << "--------" <<endl
         << freeCells << " free cells"<<endl;

    //new vector
    vector<double> vecnew; //gmap.countFreeCells());

    //seed randomizer
    srand ( time(NULL) );

    if (beliefTestFile == NULL) {
        cout << "Generating random values:"<<endl;

        //gen vector:
        for (int r=0; r<gmap.rowCount(); r++) {
            for (int c=0; c<gmap.colCount(); c++) {
                if (gmap.isObstacle(r,c)) {
                    cout << "X";
                } else {
                    double d = /*2.0*rand()/RAND_MAX*/ randomDouble(0,2.0) - 1;
                    if (d==0) {
                        cout <<"0";
                    } else if (d<0) {
                        cout << "-";
                    } else {
                        cout <<"+";
                    }
                    vecnew.push_back(d);
                }
            }
            cout << endl;
        }
    } else {
        //read from file
        QFile bFile(beliefTestFile);
        if (!bFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            cout << "Could not open the belief file: "<<beliefTestFile<<endl;
            return;
        }

        //read all data
        QString data;
        data = bFile.readAll();
        bFile.close();
        DEBUG_MAP(cout <<"ok"<<endl<<"parsing size ... "<<flush;);

        //parse string
        QStringList splitList = data.split(',');


        //splitList = data.split('\n'); //AG-NOTE: didn't work using : QChar::LineSeparator); //'\n');
        QStringList::iterator splitListIt;
        //int r=0;
        double sum = 0;
        //loop lines of map
        for (splitListIt=splitList.begin(); splitListIt!=splitList.end(); splitListIt++) {
            QString bStr = *splitListIt;
            bool ok = false;
            double b = bStr.toDouble(&ok);
            if (!ok) {
                cout << "incorrect belief value: "<< bStr.toStdString()<<endl;
                return;
            }
            vecnew.push_back(b);
            sum += b;
        }
        //print belief map
        int i = 0;
        for (int r=0; r<gmap.rowCount(); r++) {
            for (int c=0; c<gmap.colCount(); c++) {
                if (gmap.isObstacle(r,c)) {
                    //print obst
                    cout << "[XXXXX]";
                } else {
                    //print others, check for hider/seeker/base
                    double p = vecnew[i]; // (*b->bvec)(i);
                    char s1='[',s2=']';

                    if (gmap.isBase(r,c)) {
                        if ((s1=='[' && s2==']') || s1==s2) {
                            s2 = 'B';
                            if (s1=='[') s1='B';
                        } else {
                            s1=s1-('A'-'a');
                            s2=s2-('A'-'a');
                        }
                    }
                    if (p>0) {
                        cout <<s1<<setprecision(3)<< setw(4)<<fixed<< p << s2;
                    } else {
                        cout <<s1<< "     " << s2;
                    }

                    i++;
                }
            }
            cout << endl;
        }
        //last line and undo print settings
        cout << endl << resetiosflags(ios_base::fixed) << setw(-1) <<setprecision(-1);

        if (abs(sum)>0.001) cout<<"Warning: sum of belief is not 1.0, but: "<<sum<<endl;
    }

    cout << "resulting vector size: " << vecnew.size()<<endl;
    if ((int)vecnew.size() != freeCells) {
        cout << "Expected items: "<< (freeCells)<<endl;
        return;
    }

    //set map
    segmenter->setGMap(&gmap);

    //set position
    segmenter->setPosition(row, col);

    cout << "segmenting..."<<endl;
    int segCount;
    vector<int>* regVec = segmenter->segment(&vecnew, segCount);
    cout << " done segmentation, created "<<segCount<<" segments"<<endl;
    segmenter->showMap(regVec);

    cout<<endl<<"map details: size="<<gmap.rowCount()<<"x"<<gmap.colCount()<<", #free="<<gmap.numFreeCells()<<", #obstacles="<<gmap.numObstacles()<<endl;
    //", #connected obst="<<segmenter->countConnectedObstacles()<<endl;

}
#endif

void showMaps() {
    cout << "Showing the list of " << HSGlobalData::MAPS.size() <<" maps:"<<endl;
    QString mapsDir("maps/"); //TODO: make syst seperator flex
    int notFoundCount=0;

    for(int i=0; i<HSGlobalData::MAPS.size(); i++) {
        cout << "   "<<i<<") "<<HSGlobalData::MAPS[i].toStdString()<<flush;
        //test if exists
        QFile gameF(HSGlobalData::MAPS[i]);
        bool exists = gameF.exists();
        if (!exists) {
            gameF.setFileName(mapsDir+HSGlobalData::MAPS[i]);
            exists = gameF.exists();
        }
        if (exists) {
            cout << " [ok]"<<endl;
        } else {
            cout << " [NOT FOUND]"<<endl;
            notFoundCount++;
        }
    }
    cout << endl;

    if (notFoundCount>0) {
        cout << "Note: "<<notFoundCount<<" of the "<<HSGlobalData::MAPS.size()<<" maps have not been found in the current nor the subdirectory maps/"<<endl;
    }
}


//AG130301: was used to check old and new visibility check method
/*
void testVisibInMap(char* mapfile) {
    GMap map(mapfile);
    Player player(&map);
    cout << "Testing visibility algo comparing old against new for map "<<mapfile<<"..."<<endl;
    long pcnt = 0, vcntNew = 0, vcntOld = 0, n=0;

    FOR(r1,map.rowCount()) {
        FOR(c1,map.colCount()) {

            if (map.isObstacle(r1,c1))
                continue;

            player.setcurpos(r1,c1);
            //map.setInitials(r1,c1);
            //player.calculatevisible(); //AG130228: disable

            FOR(r2,map.rowCount()) {
                FOR(c2,map.colCount()) {

                    if (map.isObstacle(r2,c2))
                        continue;

                    //bool oldb = map.isVisible(r2,c2);
                    bool newb = map.isVisibleMat(r1,c1,r2,c2);

                    //if (oldb) vcntOld++;
                    if (newb) vcntNew++;

                    if (oldb!=newb) {
                        cout <<"p1=("<<r1<<","<<c1<<") p2=("<<r2<<","<<c2<<") old="<<oldb<<",new="<<newb<<endl;
                        pcnt++;
                    }

                    n++;
                }
            }
        }
    }

    cout << "Found "<<pcnt<<" inconsistensies ("<< (1.0*pcnt/(map.rowCount()*map.colCount()))<<"%)"<<endl;
    cout << "Total tests: "<<n<<", visible count: old = "<<vcntOld<<", new="<<vcntNew<<endl;

}*/



void showMap(string mapFile, bool usePomcpSim, SeekerHSParams* params) {

    cout << "Showing map: " << mapFile << endl;
    GMap gmap(mapFile,params);
    gmap.setUsingContActions(params->useContinuousPos);
    cout << "Size: "<<gmap.rowCount()<<"x"<<gmap.colCount()<<endl;
    cout << "Using continuous pos: "<<params->useContinuousPos<<endl;

    gmap.printMap();

    //cout <<"!!:"<<gmap.distance(0,3,5,7)<<endl;

    if (params->mapDistanceMatrixFile.length()>0) {
        cout <<"get pathplanner: ";
        gmap.createPathPlanner();
        PathPlanner* p = gmap.getPathPlanner();
        //cout<<"dist:"<<gmap.distance(0,0,1,2)<<"X";
        cout<<"ok, now testing speed: "<<endl;
        long ptime_us=0,ftime_us=0;
       Timer timer;
        int tid = timer.startTimer();
        for(int r1=0;r1<gmap.rowCount();r1++) {
            for(int c1=0;c1<gmap.colCount();c1++) {
                for(int r2=0;r2<gmap.rowCount();r2++) {
                    for(int c2 =0;c2<gmap.colCount();c2++) {
                        gmap.distance(r1,c1,r2,c2);
                    }
                }
            }
        }
        ptime_us = timer.getTime_us(tid);
        cout << "->Time: "<<(ptime_us/10.0e6)<<" s"<<endl;

        cout <<endl<<"read map distance file.."<<endl;
        gmap.readDistanceMatrixToFile(params->mapDistanceMatrixFile.c_str());
        cout <<"dist file read, test time:"<<endl;

        timer.restartTimer(tid);
        for(int r1=0;r1<gmap.rowCount();r1++) {
            for(int c1=0;c1<gmap.colCount();c1++) {
                for(int r2=0;r2<gmap.rowCount();r2++) {
                    for(int c2 =0;c2<gmap.colCount();c2++) {
                        gmap.distance(r1,c1,r2,c2);
                    }
                }
            }
        }
        ftime_us = timer.getTime_us(tid);
        cout << "->Time: "<<(ftime_us/10.0e6)<<" s"<<endl;


        cout <<"testing:"<<endl;
        ulong c=0,t=0;
        double maxDiff=0;
        for(int r1=0;r1<gmap.rowCount();r1++) {
            for(int c1=0;c1<gmap.colCount();c1++) {
                for(int r2=0;r2<gmap.rowCount();r2++) {
                    for(int c2 =0;c2<gmap.colCount();c2++) {
                        double dp = p->distance(c1,r1,c2,r2);
                        double df = gmap.distance(r1,c1,r2,c2);
                        double diff = abs(df-dp);
                        if (diff>maxDiff) maxDiff=diff;
                        if (diff>1e-3) {
                            cout << "r"<<r1<<"c"<<c1<<"->r"<<r2<<"c"<<c2<<": dp="<<dp<<",df="<<df<<",diff="<<diff<<endl;
                            c++;
                        }
                        t++;
                    }
                }
            }
        }

        cout <<"Path plan and distance file test done: "<<c<<"/"<<t<<" errors found with "<<maxDiff
            <<" as max diff, and stored from file was "<<((ptime_us-ftime_us)/10.0e6)<<" s faster, i.e. "
           <<(1.0*ptime_us/ftime_us)<<"x faster."<<endl<<endl;
    }


    //testing
    /*gmap.testVisibility();
    gmap.testDistance();*/



    //hssimulator
    pomcp::HSSimulator* sim = NULL;
    if (usePomcpSim) {
        cout << "Using sim"<<endl;
        sim = new pomcp::HSSimulator(&gmap, params);
    }

    bool stop=false;
    Pos x,y,x2,y2;
    double obs1p;


    //TODO: variable pathplanner
    //gmap.createPropDistPlanner();


    Player player(&gmap);
    PathPlanner* pathPlanner = gmap.getPathPlanner();

    while (!stop) {
        cout << "now enter start and pos for path planning, enter <0 to quit"<<endl<<endl;

        double row,col;
        cout << "Start/seeker row: ";
        cin >> row;
        //STOPQRY(x.x);
        if (row<0) break; //return 0;
        cout << " col: ";
        cin >> col;
        testx(col);
        x.set(row,col);

        cout << endl << "Goal/hider row: ";
        cin >> row;
        testx(row);
        cout << " col: ";
        cin >> col;
        testx(col);
        y.set(row,col);

        //AG150126: test 2 seekers
        if (params->gameHas2Seekers()) {
            cout << "Start/seeker 2 row: ";
            cin >> row;
            //STOPQRY(x.x);
            if (row<0) break; //return 0;
            cout << " col: ";
            cin >> col;
            testx(col);
            x2.set(row,col);

            cout << endl << "Goal/hider (seen from s2) row: ";
            cin >> row;
            testx(row);
            cout << " col: ";
            cin >> col;
            testx(col);
            y2.set(row,col);

            cout << "Prob. of choosing obs1: :";
            cin >> obs1p;
        }

        gmap.printMap(x);


        cout << " distance ["<<x.toString()<<"->"<<y.toString()<<"]: " << gmap.distance(x,y)<<endl;//", cont: "<<gmap.distanceCont(x,y)<<endl;

//gmap.distanceCont(x,y);

        player.setCurPos(x);

        //player.calculatevisible(); //AG130228: disable

        //cout << "Old isVisible: "<< gmap.isVisible(y.x,y.y)<<endl;
        cout << "New isVisible: "<< gmap.isVisible(x,y,false)<<endl;

        cout << "Path from "<<x.toString()<<" to "<<y.toString()<<endl;
        int r = x.row();
        int c = x.col();
        int d = 0;
        if (pathPlanner==NULL) pathPlanner = gmap.getPathPlanner();
        while (!y.equalsInt(r,c)) {
            pathPlanner->nextStep(c,r,y.col(),y.row(),&c,&r);
            d++;
            cout << " "<<d<<": r"<<r<<"c"<<c<<endl;
        }
        cout <<"--"<<endl;
//return;


        cout << "Invisible from pos 1:"<<endl;
        vector<Pos> invisPosVec = gmap.getInvisiblePoints(x,false);
        FOR(i,invisPosVec.size()) {
            cout << " "<<i<<"] " << invisPosVec[i].toString() <<endl;
        }
        cout <<endl;

        cout <<"Trying actions:"<<endl;
        for(int a=0; a<HSGlobalData::NUM_ACTIONS; a++) {
            cout << " - "<<ACTION_COUT(a)<<": "<<flush;
            Pos q=gmap.tryMove(a,x);
            cout << q.toString()<<endl;
        }
        /*char abc;
        cin>>abc;*/

        if (sim!=NULL) {
            cout << " == POMCP Sim Tests =="<<endl;
            cout << " -- genInitState --"<<endl;

            bool v = gmap.isVisible(x,y,false);
            cout << "visiblity: (s->h): "<<v<<endl;

            //AG131104: use obs
            //sim->setSeekerHiderPos(x,y,v);
            pomcp::HSState obs(x,y);
            if (!v) obs.hiderPos.clear();
            cout << "obs1: "<<obs.toString()<<endl;

            pomcp::HSState* obs2 = NULL;
            if (params->gameHas2Seekers()) {
                bool v2 = gmap.isVisible(x2,y2,false);
                cout << "visiblity: (s2->h2): "<<v2<<endl;
                obs2 = new pomcp::HSState(x2,y2);
                if (!v2) obs2->hiderPos.clear();
                cout<<"obs2: "<<obs2->toString()<<endl;
            }

            cout<<"10 init states: ";
            FOR(i,10) {
                pomcp::State* s = sim->genInitState(&obs,obs2,obs1p);
                cout << s->toString()<<" ";
                delete s;
            }
            cout << endl;

            cout<<"10 init all states: ";
            vector<pomcp::State*> vecAllinitStates = sim->genAllInitStates(&obs,10,obs2,obs1p);
            for (pomcp::State* s : vecAllinitStates) {
                cout << s->toString()<<" ";
                delete s;
            }
            cout<<endl;

            //cout << " -- genRandomState --"<<endl;
            pomcp::HSState hsstate(x,y);
            /*cout<<"10 next random states from "<< hsstate.toString() <<": ";
            //sim->setSeekerHiderPos(x,y,v);
            FOR(i,10) {
                pomcp::State* s = sim->genRandomState(&hsstate,NULL);
                cout << s->toString()<<" ";
                delete s;
            }
            cout << endl;*/


            cout << " -- getActions --"<<endl;
            cout<<"possible actions from "<< hsstate.toString() <<": ";
            vector<int> actVec;
            sim->getActions(&hsstate,NULL,actVec);
            FOREACH(int,a,actVec) {
                cout << *a<<" ";
            }
            cout << endl;


            cout << " -- getPossibleNextPos --"<<endl;
            cout<<"possible pos from "<< x.toString() <<": ";
            vector<Pos> posVec;
            sim->getPossibleNextHiderPos(x,posVec);
            FOREACHnc(Pos,p,posVec) {
                cout << p->toString()<<" ";
            }
            cout << endl;


            cout << " -- setInitialNodeValue --"<<endl;
            cout<<"set init node value from "<< hsstate.toString() <<": "<<flush;
            pomcp::Node node(sim,NULL);
            for(int a=-1; a<HSGlobalData::NUM_ACTIONS; a++) {
                sim->setInitialNodeValue(&hsstate,NULL,&node,a);   //(x,posVec);
                cout << " a="<<a<<" count="<<node.getCount()<<" v="<<node.getValue()<<"; ";
            }
            cout << endl;


            cout << " -- step --"<<endl;
            cout<<"step from "<< hsstate.toString() <<": ";
            for(int a=0; a<HSGlobalData::NUM_ACTIONS; a++) {
                double r=0;
                pomcp::State* o = NULL;
                pomcp::State* nextS = sim->step(&hsstate,a,o,r);
                cout << " a="<<a<<"->next="<<nextS->toString()<<",o="<<(o==NULL?"NULL":o->toString())<<",r="<<r<<"; ";
                delete nextS;
            }
            cout << endl;

            cout << " -- step with obs2 --"<<endl;
            cout<<"step from "<< hsstate.toString() <<": ";
            for(int a=0; a<HSGlobalData::NUM_ACTIONS; a++) {
                double r=0;
                pomcp::State* o = NULL;
                pomcp::State* nextS = sim->step(&hsstate,a,o,r,true,&obs,obs2,obs1p);
                cout << " a="<<a<<"->next="<<nextS->toString()<<",o="<<(o==NULL?"NULL":o->toString())<<",r="<<r<<"; ";
                delete nextS;
            }
            cout << endl;


            cout << " -- isFinal --"<<endl;
            cout << hsstate.toString()<<": "<< sim->isFinal(&hsstate)<<endl;

            cout << "-- immediateReward --"<<endl;
            cout << hsstate.toString()<<": "<< sim->getImmediateReward(&hsstate)<<endl;

            cout << " -- reward --"<<endl;
            cout << sim->getImmediateReward(&hsstate) << endl;

            if (obs2!=NULL) delete obs2;

        } // if sim!=NULL

    } //while


    if (sim!=NULL) delete sim;
}



void testPOMCP(string mapFile, SeekerHSParams* params, bool testMCTS) {
    /*GMap map(5,5);
    map.addObstacle(2,1);
    map.setUsingContActions(true);
    map.setMapFixed();
    Pos pos(2.5,2.5);
    //for (int i=0;i<5;i++)
        map.printMap(pos);*/

    cout << "Test POMCP; map: " << mapFile << endl;
    GMap gmap(mapFile,params);
    cout << "Size: "<<gmap.rowCount()<<"x"<<gmap.colCount()<<endl;
    gmap.printMap();

    cout<<endl;

    pomcp::HSSimulator* sim = NULL;

    switch (params->pomcpSimType) {
        case  SeekerHSParams::POMCP_SIM_DISCR: { //SeekerHSParams::SOLVER_POMCP: {
            pomcp::HSSimulator* hssim = new pomcp::HSSimulator(&gmap, params);
            sim = hssim;
            //sim.testAllFunctions();
            break;
        }
        case SeekerHSParams::POMCP_SIM_CONT: { //SOLVER_POMCP_CONT: {
            pomcp::HSSimulatorCont* hssimcon = new pomcp::HSSimulatorCont(&gmap, params);
            sim = hssimcon;
            //sim.testAllFunctions();
            break;
        }
        case SeekerHSParams::POMCP_SIM_PRED: {
            pomcp::HSSimulatorPred* hssimpred = new pomcp::HSSimulatorPred(&gmap, params);
            //sim.testAllFunctions();
            sim = hssimpred;
            break;
        }
        default:
            cout << "Unknown solver type!"<<endl;
            break;
    }

    if (sim != NULL) {
        if (testMCTS) {
            pomcp::MCTS mcts(params, sim, NULL);
            mcts.testNodes(&gmap);
        } else {
            sim->testAllFunctions();
        }
        cout << "deleting simulator:"<<flush;
        delete sim;
        cout << "ok"<<endl;
    }

}

void writeMap(string mapFile, string mapFileOut, Pos base, int zoomOutFac, double cellSize, double rowDist_m,
              double colDist_m, bool isYalm, SeekerHSParams* params) {
    cout << "Write map mode"<<endl;
    cout << endl<<"Loading map ... "<<endl;
    if (isYalm) {
        GMap gmap(mapFile, base, cellSize, rowDist_m, colDist_m ,params);
        cout << "done loading"<<endl;
        gmap.printMap();
        cout<<"Writing "<<mapFileOut<<"..."<<endl;
        gmap.writeMapFile(mapFileOut);
    } else {
        GMap gmap(mapFile, base, zoomOutFac,params);
        cout << "done"<<endl<<"Writing "<<mapFileOut<<"..."<<endl;
        gmap.writeMapFile(mapFileOut);
    }
    cout << "done"<<endl;
}

void writeDistMat(SeekerHSParams* params) {
    cout << "Write dist. map"<<endl;
    cout << endl<<"Loading map ... "<<endl;
    GMap gmap(params->mapFile,params);
    //cout<<"!!:"<<gmap.distance(0,3,5,7)<<endl;
    cout << "done"<<endl<<"Writing "<<params->mapDistanceMatrixFile<<"..."<<endl;
    gmap.writeDistanceMatrixToFile(params->mapDistanceMatrixFile.c_str());
    cout << "done"<<endl;
}

//AG121113: get map id from file name: first get n
// assume file to be of shape:  /abc/def/name.extension
// the name 'name' will be get and searched in the list of MAPS to get its idea, -1 will be returned otherwise
int getMapIDFromFileName(string file) {
    QString fname = QString::fromStdString(file);
    //search last / or \ for start of file
    int si = fname.lastIndexOf(QDir::separator());
    if (si==-1)
        si = 0;
    else
        si++;
    //index of dot / end of name
    int ei = fname.lastIndexOf(".");
    if (ei==-1)
        ei = fname.length();

    //get name
    QString name = fname.mid(si,ei-si);

    //check index
    int i = HSGlobalData::MAPS.indexOf(name);
    if (i==-1) i = HSGlobalData::MAPS.indexOf(name + ".txt");


    return i;
}

void printDoubleVectorToCout(const vector<double>& v) {
    for(double d:v) {
        cout <<d<<",";
    }
}

void testPeoplePrediction(SeekerHSParams *params) {
#ifdef DO_USE_PEOPLEPREDICTION //AG120121: disabled because not finished and not used
    cout<<"Testing PeoplePrediction"<<endl<<endl<<"Creating PeoplePredictionWrapper: "<<endl;
    PeoplePredictionWrapper wrapper(params);
    cout<<"PeoplePredictionWrapper created"<<endl;

    vector<vector<double>> meanVec, covVec;

    double x = 1, y = 1, vx = 1, vy = 0;

    uint n = 10;
    cout << "Now test "<<n<<" iterations:"<<endl;
    for(uint i=0; i<n; i++){
        cout <<">***[it "<<i<<"]***<"<<endl;
        cout<<">Input obs: (x,y)=("<<x<<","<<y<<"), (vx,vy)=("<<vx<<","<<vy<<") (no vel obs) "<<endl;
        wrapper.update(x,y,0,0); //,vx,vy);
        cout<<">Update done"<<endl<<">Get predictions"<<endl;
        if (wrapper.predict(meanVec, covVec)) {
            cout << ">Predictions received successfully"<<endl<<">Predictions:"<<endl;
            assert(meanVec.size()==covVec.size());
            for(size_t i=0; i<meanVec.size();i++) {
                cout<<">    "<<i<<") mean: ";
                printDoubleVectorToCout(meanVec[i]);
                cout<<"; cov: ";
                printDoubleVectorToCout(covVec[i]);
                cout<<endl;
            }
        } else {
            cout << ">Predictions received failed"<<endl;
        }

        //update x,y
        x += vx;
        y += vy;
    }

    cout <<">***FINISHED***<"<<endl;
#endif
}

