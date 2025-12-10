#include <QtCore/QCoreApplication>
#include <QDir>
#include <QDateTime>

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


#include "HSGame/gplayer.h" //world.h"

#include "gameconnector.h"

#include "Solver/hsmomdp.h"
#include "Solver/hsmomdp_layered.h"
#include "Solver/hsmomdp_runpolicy.h"

#include "Segment/segment.h"
#include "Segment/basesegmenter.h"
#include "Segment/kmeanssegmenter.h"
#include "Segment/robotcenteredsegmentation.h"
#include "Segment/combinecenteredsegmentation.h"
#include "Segment/testsegmenter.h"

#include "Utils/timer.h"
#include "Utils/generic.h"

#include "hsconfig.h"
#include "hsglobaldata.h"

#include "seekerhs.h"
#include "Smart/smartseeker.h"


#include "POMCP/hssimulator.h"
#include "POMCP/hsstate.h"
//#include "mcvi/hideseekDiscrete/hideseekmcvi.h"
//#include <cfloat>

//#include "Utils/readwriteimage.h"


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

/*static const char SOLVER_NOT_SET = 0;
static const char SOLVER_OFFLINE = 1;
static const char SOLVER_LAYERED = 2;
static const char SOLVER_LAYERED_COMPARE = 3;

static const char SEGMENTER_NOT_SET = 0;
static const char SEGMENTER_BASIC = 1;
static const char SEGMENTER_KMEANS = 2;
static const char SEGMENTER_ROBOT_CENTERED = 3;
static const char SEGMENTER_TEST = 4;
static const char SEGMENTER_CENTERED_COMBINED = 5;


static const char UPPERBOUND_INIT_FIB = 0;
static const char UPPERBOUND_INIT_MDP = 1;*/

QStringList RUN_MODES = QStringList()  << "Run Game"
                                       << "Test Mode"
                                       << "Test segmentation"
                                       << "Show Maps list"
                                       << "Show Map"
                                       << "Test POMCP"
                                       << "Write Map";

QStringList SOLVERS = QStringList()    << "[not set]"
                                       << "Offline"
                                       << "Online layered"
                                       << "Online layered (compare with offline)";

QStringList OPP_TYPES = QStringList()  << "Human"
                                       << "Random Hider"
                                       << "Smart Hider"
                                       << "Action List Hider"
                                       << "Seeker(?)"
                                       << "Very Smart Hider"
                                       << "All Knowing (Smart) Hider"
                                       << "All Knowing Very Smart Hider";

QStringList INITUB_TYPES = QStringList() << "FIB (Fast Informed Bound)"
                                         << "(Q)MDP";

/* FROM Player:
static const int SET_HIDER_TYPE_HUMAN = 0; //??
static const int SET_HIDER_TYPE_RANDOM_HIDER = 1; //??
static const int SET_HIDER_TYPE_SMART_HIDER = 2; //??
*/

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

void testHSMOMDP(HSMOMDP* hsmomdp, char* mapFile);
void testSegmentation(char* mapFile,Segmenter* segmenter, int row, int col, char* beliefTestFile);
void showMaps();
void showMap(const char* mapFile, bool usePomcpSim, SeekerHSParams* params);
void testPOMCP(const char* mapFile, SeekerHSParams* params);
void showHelp(string error="", int exitCode=EXIT_FAILURE, char* arg=NULL);
void showVersion();
int getMapIDFromFileName(string file);
void testSeekerHS(SeekerHS* seeker);
void writeMap(const char* mapFile, string mapFileOut, Pos base, int zoomOutFac);



int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);

    //ag120208: set locale to en-US such that the atof uses decimal dots and not comma
    setlocale(LC_NUMERIC,"en_US");

    try
    {        
        cout <<endl
            << "+-----------------------------------+" << endl
            << "| Automatic Seeker Hide&Seek Client |" << endl
            << "+-----------------------------------+" << endl<<endl;

        SeekerHSParams params;

        char* beliefTestFile = NULL;
        char runMode = MODE_RUN_GAME;
        char* logFilePrefix = NULL;
        bool showVer = false;
        int robotRow = -1, robotCol = -1;
        bool sendMap = false;
        bool sendMapPGM = false;
        Pos basePos;
        int zoomOutFac = 1;
        string mapFileOut;

        //AG120914: use solverparams to pass params
        SolverParams* solverParams = &GlobalResource::getInstance()->solverParams;
        solverParams->useLookahead = false;


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
                        runMode = MODE_TEST_SEGMENT; //segmentation
                        if (i+1<argc && argv[i+1][0]!='-') {
                            beliefTestFile = argv[++i];

                            if (i+2<argc && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
                                QString n = QString::fromAscii(argv[++i]);
                                bool ok = false;
                                robotRow = n.toInt(&ok);
                                if (!ok) showHelp("expected a number for position row at",EXIT_FAILURE,arg);

                                n = QString::fromAscii(argv[++i]);
                                ok = false;
                                robotCol = n.toInt(&ok);
                                if (!ok) showHelp("expected a number for position col at",EXIT_FAILURE,arg);
                            }
                        }
                        break;
                    case 'm':
                        runMode = MODE_SHOW_MAP;
                        break;
                    case 'c':
                        runMode = MODE_TEST_POMCP;
                        break;
                    default:
                        showHelp("unknown parameter",EXIT_FAILURE,arg);
                        break;
                    }
                } else {
                    runMode = MODE_TEST;
                }
                break;
            case 'H':
            case '?':
            case 'h': // help
                showHelp("",EXIT_SUCCESS);
                break;
            case 's': //server or solver
                if (strlen(arg)==2) { //server: "-s"
                    if (i+2>=argc) showHelp("server expecting two parameters: ip port");
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
                            case 's': //-sss: score type
                                if (strlen(arg)==5) {
                                    switch(arg[4]) {
                                    case 'a':
                                        params.smartSeekerScoreType = SmartSeeker::SCORE_AVG;
                                        break;
                                    case 'x':
                                        params.smartSeekerScoreType = SmartSeeker::SCORE_MAX;
                                        break;
                                    case 'n':
                                        params.smartSeekerScoreType = SmartSeeker::SCORE_MIN;
                                        break;
                                    default:
                                        showHelp("unknown smart seeker score type",EXIT_FAILURE,arg);
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
                    case 'c':
                        params.solverType=SeekerHSParams::SOLVER_POMCP;
                        break;
                    default:
                        showHelp("unknown parameter/solver", EXIT_FAILURE, arg);
                        break;
                    }
                }
                break;
            case 'm': // momdp file
                if (i+1>=argc) showHelp("expected MOMDP file");
                params.pomdpFile = argv[++i];
                break;
            case 'p': //policy file or precision
                if (strlen(arg)==2) { // policy file
                    if (i+1>=argc) showHelp("expected policy file");
                    params.policyFile = argv[++i];
                } else if (arg[2]=='d') {
                    //AG121008: disable pruning
                    params.doPruning = false;
                } else if (arg[2]=='p') {
                    //AG121016: print params
                    params.showParams = true;
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
                    if (arg[2]!='s' && arg[2]!='p') {
                        showHelp("expected map send type",EXIT_FAILURE,arg);
                    }
                    if (arg[2]=='p') {
                        sendMapPGM = true;                        
                    }
                }

                if (i+1>=argc) showHelp("expected map file");
                params.mapFile = argv[++i];
                //cout << "map:"<<params.mapFile<<endl<<"next:"<<(argc>i+1?argv[i+1]:"[n/a]")<<endl<<"argc="<<argc<<endl<<"sendmappgm:"<<sendMapPGM<<endl<<"i="<<i<<endl;
                if (sendMapPGM && i+1<argc && argv[i+1]!='-') {
                    QString zmStr(argv[++i]);
                    bool ok=false;
                    zoomOutFac = zmStr.toInt(&ok);
                    if (!ok) showHelp("expected zoomoutfactor",EXIT_FAILURE,argv[i]);
                    //cout << "zoomoutf="<<zoomOutFac<<endl;
                    //cout <<"i="<<i<<endl;
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
                    case 'l': //AG120904: opponent with action list
                        params.opponentType = HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST;

                        if (i+1>=argc) showHelp("expected action list file");
                        params.oppActionFile = argv[++i];
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
            case 'g': //segmenter type
                {
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
            case 'b': // best action option                
                if (strlen(arg)==2) {
                    //base
                    if (i+2>=argc) showHelp("expected base row col",EXIT_FAILURE,arg);
                    QString rStr(argv[++i]);
                    bool ok = false;
                    basePos.row = rStr.toInt(&ok);
                    if (ok) {
                        QString cStr(argv[++i]);
                        basePos.col = cStr.toInt(&ok);
                    }
                    if (!ok) showHelp("expected base row col",EXIT_FAILURE,arg);

                } else if (strlen(arg)!=3) {  //expecting "-b?"
                    showHelp("unknown parameter / opponent type: ",EXIT_FAILURE, arg);
                } else {
                    if (arg[2]=='l') {
                        params.useLookahead = true;
                    } else {
                        showHelp("unknown parameter / opponent type: ",EXIT_FAILURE, arg);
                    }
                }
                break;
            case 'r': // top reward option
                if (strlen(arg)!=3) {  //expecting "-r?"
                    showHelp("unknown parameter / top reward type: ",EXIT_FAILURE, arg);
                } else {
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
                    default:
                        showHelp("unknown parameter / top reward type: ",EXIT_FAILURE, arg);
                        break;
                    }
                }
                break;
            case 'R': // reward type
                if (strlen(arg)!=3) {  //expecting "-R?"
                    showHelp("unknown parameter / reward type: ",EXIT_FAILURE, arg);
                } else {
                    switch(arg[2]) {
                    case 'f':
                        params.rewardType = SeekerHSParams::REWARD_FINAL;
                        break;
                    case 'c':
                        params.rewardType = SeekerHSParams::REWARD_FINAL_CROSS;
                        break;
                    case 't':
                        params.rewardType = SeekerHSParams::REWARD_TRIANGLE;
                        break;
                    default:
                        showHelp("unknown parameter / top reward type: ",EXIT_FAILURE, arg);
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
                    default:
                        showHelp("unknown parameter",EXIT_FAILURE,arg);
                        break;
                    }
                }
                break;
            }
            case 'M': { // max mem
                if (i+1>=argc) showHelp("expected max memory");
                QString maxmemStr(argv[++i]);
                bool ok = false;
                params.memoryLimit = maxmemStr.toInt(&ok);
                if (!ok) showHelp("expected max memory instead of ",EXIT_FAILURE, argv[i]);
                params.memoryLimit *= 1024*1024;
                break;
            }
            case 'w': { //win dist
                if (i+1>=argc) showHelp("expected win distance");
                QString winDStr(argv[++i]);
                bool ok = false;
                params.winDist = winDStr.toInt(&ok);
                if (!ok) showHelp("expected win dist instead of ",EXIT_FAILURE,argv[i]);
                break;
            }
            case 'n':
                if (strlen(arg)!=3) {  //expecting "-n?"
                    showHelp("unknown parameter / number of ..: ",EXIT_FAILURE, arg);
                } else {
                    if (i+1>=argc) showHelp("expected number distance");
                    QString numbStr(argv[++i]);
                    bool ok = false;
                    unsigned int numb = numbStr.toUInt(&ok);
                    if (!ok) showHelp("expected number instead of ",EXIT_FAILURE,argv[i]);

                    switch(arg[2]) {
                    case 'i':
                        params.numInitBeliefStates = numb;
                        break;
                    case 's':
                        params.numSim = numb;
                        break;
                    default:
                        showHelp("unknown parameter / number of ..: ",EXIT_FAILURE, arg);
                        break;
                    }
                }
                break;
            case 'e': {
                if (i+1>=argc) showHelp("expand count");
                QString expCountStr(argv[++i]);
                bool ok = false;
                params.expandCount = expCountStr.toUInt(&ok);
                if (!ok) showHelp("expand count instead of ",EXIT_FAILURE,argv[i]);
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
        /*QString unameStr;
        if (username==NULL) {
            unameStr = "autohsUser";
        } else {
            unameStr = username;
        }*/
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
        cout    << "Run Mode:               " << RUN_MODES[runMode].toStdString() <<endl;
        params.printVariables(true);

        //check params
        if (runMode==MODE_TEST || runMode==MODE_TEST_SEGMENT || runMode==MODE_SHOW_MAP) {
            if (params.mapFile.length()==0)
                showHelp("map file required");
        }

        if (runMode == MODE_RUN_GAME) {
            if (params.serverIP.length()==0 || params.serverPort<=0)
                showHelp("server IP and port have to be set");
            if (!sendMap && ( params.mapID<0 || params.mapID>=HSGlobalData::MAPS.size() ) )
                showHelp("give a map ID, use '-A ?' for a list of the maps");
            if (params.opponentType<0 || params.opponentType>=OPP_TYPES.size())
                showHelp("give a legal opponent type");
            if (sendMap && params.mapFile.length()==0)
                showHelp("a map file is expected when sending the map");
            if (sendMap && sendMapPGM && !basePos.isSet())
                showHelp("a base is required when loading a pgm map");
        }


        switch (runMode) {
        case MODE_RUN_GAME: {            
            cout << "===--- Starting at "<< QDateTime::currentDateTime().toString().toStdString() << " ---===" << endl;

            GMap* gmap = NULL;
            if (sendMap) { //load map to send
                if (sendMapPGM) {
                    cout << "Zoomoutfactor: "<<zoomOutFac<< endl;
                    gmap = new GMap(params.mapFile.c_str(), basePos,zoomOutFac);
                    gmap->printMap();
                } else {
                    gmap = new GMap(params.mapFile);
                    if (basePos.isSet()) {
                        gmap->setBase(basePos);
                    }
                }
            }

            //GameConnector gameConnector(ip,port,2,hsmomdp,mapID,opponentType,username,1,oppActionFile);
            GameConnector gameConnector(params.serverIP, params.serverPort, 2, seekerHS->getAutoPlayer(), seekerHS, // hsmomdp,
                                        params.mapID, params.opponentType, params.userName, 1, params.oppActionFile, params.winDist, gmap);

            cout << "===--- Stopped at "<< QDateTime::currentDateTime().toString().toStdString() << " ---===" << endl;

            break;
        }
        case MODE_TEST: {
            testSeekerHS(seekerHS);
            break;
        }        
        case MODE_TEST_SEGMENT:
            //testSegmentation(mapFile,segmenter, robotRow, robotCol, beliefTestFile);
            break;

        case MODE_SHOW_MAP_LIST:
            showMaps();
            break;

        case MODE_SHOW_MAP:            
            showMap(params.mapFile.c_str(), params.solverType==SeekerHSParams::SOLVER_POMCP, &params);
            break;
        case MODE_TEST_POMCP:
            testPOMCP(params.mapFile.c_str(), &params);
            break;
        case MODE_WRITE_MAP:
            writeMap(params.mapFile.c_str(), mapFileOut, basePos, zoomOutFac);
            break;
        }

        if (seekerHS!=NULL)
            delete seekerHS;


    }
    catch(bad_alloc &e) {
        if(GlobalResource::getInstance()->solverParams.memoryLimit == 0)
        {
            cout << "Memory allocation failed. Exit." << endl;
        }
        else
        {
            cout << "Memory limit reached. Please try increase memory limit" << endl;
        }

    }
    catch(CException &ce) {
        cout << "CException: " << ce.what() << endl ;
    }
    catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }


    return 0; //a.exec();
}



void testHSMOMDP(HSMOMDP* hsmomdp, char* mapFile) {
    cout << " TEST mode " << endl;

    GMap map(mapFile);
    Player *p = new Player(&map);

    cout << "Loading map "<<endl;
    //map->readMapFile(mapFile);
    p->sethidertype(-1);
    p->setType(2);
    p->setcurpos(0,0);
    p->setoppos(3,3);
    map.setInitialh(0,0);
    map.setInitials(3,3);


    Pos inith = map.getInitialh();
    Pos inits = map.getInitials();
    bool visib = map.isVisible(inits,inith);

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

        cout << "Calc action ..."<<endl;
        action = hsmomdp->getNextAction(x,y,isVis!=0);
        cout<<endl << "Action: "<<action << " ("<<actions[action]<<")"<<endl<<endl;


        cout << "now enter pos of hider,seeker and if visible, enter <0 to quit"<<endl<<endl;

        cout << "Seeker row: ";
        cin >> x.row;
        //STOPQRY(x.x);
        if (x.row<0) break; //return 0;
        cout << " col: ";
        cin >> x.col;
        testx(x.col);

        cout << endl << "Hider row: ";
        cin >> y.row;
        testx(y.row);
        cout << " col: ";
        cin >> y.col;
        testx(y.col);

        cout << endl << "Hider visible? (1=true,0=false): ";
        cin >> isVis;
        testx(isVis);

    }
}


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
            for(int i=0;i<nHPos; i++) {
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
        if (visib) {
            //hider pos
            hPos.row = mulHiderPos[0][0];
            hPos.col = mulHiderPos[0][1];
        }

        int status = seeker->getWinState(sPos,hPos,visib);

        if (status==SeekerHS::STATE_TIE || status==SeekerHS::STATE_WIN_HIDER || status==SeekerHS::STATE_TIE) {
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

                if (winState!=SeekerHS::STATE_PLAYING) {
                    cout<<"end of game"<<endl;
                    break;
                }

                cout << "next actions: ";
                for(int i=1;i<actVec.size();i++) {
                    cout << actions[actVec[i]] << " ("<<newSeekerPos[i*3]<<","<<newSeekerPos[i*3+1]<<","<<newSeekerPos[i*3+2]<<"º) ";
                } cout <<endl;
            }
            catch(CException& ce) {
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
            for(int i=0;i<nHPos; i++) {
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
            for(int i=0;i<nHPos; i++) {
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
                for(int i=1;i<actVec.size();i++) {
                    cout << actions[actVec[i]] << " ("<<newSeekerPos[i*3]<<","<<newSeekerPos[i*3+1]<<","<<newSeekerPos[i*3+2]<<"º) ";
                } cout <<endl;
            }
            catch(CException& ce) {
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

    cout << "hsmomdp [-t | -ts | -tm | -tc | -h | -v] [-so | -sl | -slc | -sm | -ss] [-Rf | -Rc | -Rt] [-gt | -gb | -gk [k] | -g[r|c][x] "
         << "[d a b h]] [-m momdp-file] [-p policy-file] [-a[s|p] map-file] [-A map-id] [-b row col] [-u username] [-s server port] "
         << "[-oh | -or | -os | -ov | -oa | -oA] -dc [-L log-prefix] [-um | -uf] [-pt target-prec] [-pi target-init-fac] "
         << "[-pd] [-pp] [-rs|-ra|-rn|-rx] [-d max_depth] [-M max-mem] [-w win-dist] [-ni num_init_belief_points "
         << "-ns num_sim x expl_const -e expand_cnt -d max_search_depth] [-dc] [-sss[a|x|n]] [-ssm max_cells]" << endl
         << "   -t                test manual input of data" << endl
         << "   -ts [belief-file] test segmentation, if belief-file not given than random number used" << endl
         << "   -tm               test map, show the map" << endl
         << "   -tc               test POMCP"<<endl
         << "   -h                this help" << endl
         << "   -v                shows version and build" << endl
         << "   -so               solver: uses offline learned policy" << endl
         << "   -sl               solver: layered MOMDP" << endl
         << "   -slc              solver: layered MOMDP, but compare with offline" << endl
         << "   -sm               solver: MCVI (offline) [not implemented yet]"<<endl
         << "   -sc               solver: POMCP (online) uses MCTS"<<endl
         << "   -ss               smart seeker"<<endl
         << "   -W map_file       write map to map file"<<endl
         << "   -R[f|c|t]         Rewards: f=final, c=final and crossing, t=triangle"<<endl
         << "   -bl               use best action look ahead"<<endl
         << "   -m momdp-file     MOMDP file" << endl
         << "   -p policy-file    policy file" << endl
         << "   -a[s|p] map-file  map file, -as: send map to server; -ap: send map to server, PMG map" << endl
         << "   -A [map-id | ?]   map ID (? shows list of maps and IDs)" << endl
         << "   -b row col        base (row,col), overwrites base in file" << endl
         << "   -u user           username of the player in the game" << endl
         << "   -s server port    server and port (default: localhost:" << SeekerHSParams::DEFAULT_SERVER_PORT <<")" << endl
         << "   -o[h|r|s|v|a|A]   opponent: Human/Random/Smart hider/Very Smart Hider/All knowing hider/All know. Very Smart H. " << endl
         << "     -ol act-file    Action list with file name (no path)" <<endl
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
         << "   -pt target-prec   set the target precision for the MOMDP solver (default=" << SolverParams::DEFAULT_TARGET_PRECISION <<")"<<endl
         << "   -pi init-t-fac    init target procesion factor for calculating upper and lower bounds (default="<<SolverParams::CB_INITIALIZATION_PRECISION_FACTOR<<")"<<endl
         << "   -pd               pruning disabled, i.e. no pruning (for debug purpuse)"  <<endl
         << "   -pp               print (APPL solver) parameters before solving" <<endl
         << "   -rs|-ra|-rn|-rx   top reward aggregation: sum / average / min / max"<<endl
         << "   -d max-depth      maximum depth to search belief tree"<<endl
         << "   -M max-memory-MB  maximum memory to be used by SARSOP (MB)"<<endl
         << "   -w win_dist       maximum distance to hider for seeker to win (default: 0)"<<endl
         << "   -ni numb_init_bel number of init beliefs"<<endl
         << "   -ns numb_sims     number of simulations"<<endl
         << "   -x expl_constant  exploration constant"<<endl
         << "   -e expand_count   expand count (after so many counts node is expanded)"<<endl
         << "   -dc               disable consistency check / allow inconcistencies"<<endl
         << "   -sss[a|x|n]       smart seeker score type: average/max/min"<<endl
         << "   -ssm max_cells    smart seeker max. cells to calculate when hidden (random choice)"<<endl
         <<endl   ;

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
    cout << " - " << SolverParams::getAPPLVersion() <<endl;
    cout << " - Qt " << QT_VERSION_STR <<endl;
    cout << " - OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION<<endl<<endl;
    cout << "GNU compiler version: " <<__VERSION__<<endl;//<<__GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__<<endl;
    cout << endl << "Debug flags: ";
    DEBUG_HS(cout <<"debug (DEBUG_HS); ";);
    DEBUG_HS_INIT(cout <<"init debug (DEBUG_HS_INIT); ";);
    DEBUG_HS1(cout << "detailed debug (DEBUG_HS1); ";);
    DEBUG_MAP(cout << "map debugging (DEBUG_MAP); ";);
    DEBUG_SEGMENT(cout << "segment debug (DEBUG_SEGMENT); ";);
    cout << "Show transition probabilities: "<<showOnOff(DEBUG_SHOW_TRANS)<<endl;

    cout << "APPL debug: " << showOnOff(SolverParams::isDebugOn()) << "; debug trace: "<<showOnOff(SolverParams::isDebugTraceOn()) <<endl;
    cout << endl;
}

void testSegmentation(char* mapFile,Segmenter* segmenter, int row, int col, char* beliefTestFile) {
   cout << "TEST MODE segmentation"<<endl;
   GMap gmap(mapFile);
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
                   double d = 2.0*rand()/RAND_MAX - 1;
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



void showMap(const char* mapFile, bool usePomcpSim, SeekerHSParams* params) {

    cout << "Showing map: " << mapFile << endl;
    GMap gmap(mapFile);
    cout << "Size: "<<gmap.rowCount()<<"x"<<gmap.colCount()<<endl;
    gmap.printMap();

    //testing
    gmap.testVisibility();
    gmap.testDistance();

    //hssimulator
    pomcp::HSSimulator* sim = NULL;
    if (usePomcpSim) {
        cout << "Using sim"<<endl;
        sim = new pomcp::HSSimulator(&gmap, params);
    }

    bool stop=false;
    Pos x,y;


    //TODO: variable pathplanner
    //gmap.createPropDistPlanner();


    Player player(&gmap);

    while (!stop) {
        cout << "now enter start and pos for path planning, enter <0 to quit"<<endl<<endl;

        cout << "Start/seeker row: ";
        cin >> x.row;
        //STOPQRY(x.x);
        if (x.row<0) break; //return 0;
        cout << " col: ";
        cin >> x.col;
        testx(x.col);

        cout << endl << "Goal/hider row: ";
        cin >> y.row;
        testx(y.row);
        cout << " col: ";
        cin >> y.col;
        testx(y.col);

        cout << " distance: " << gmap.distance(x.row,x.col,y.row,y.col)<<endl;

        player.setcurpos(x.row,x.col);

        //player.calculatevisible(); //AG130228: disable

        gmap.printMap(x.row,x.col);

        //cout << "Old isVisible: "<< gmap.isVisible(y.x,y.y)<<endl;
        cout << "New isVisible: "<< gmap.isVisible(x.row,x.col,y.row,y.col)<<endl;

        cout << "Invisible from pos 1:"<<endl;
        vector<Pos> invisPosVec = gmap.getInvisiblePoints(x.row,x.col);
        FOR(i,invisPosVec.size()) {
            cout << " "<<i<<"] r" << invisPosVec[i].row << "c"<<invisPosVec[i].col<<endl;
        }
        cout <<endl;

        if (sim!=NULL) {

            cout << " -- genInitState --"<<endl;
            bool v = gmap.isVisible(x,y);
            cout << "visiblity: (s->h): "<<v<<endl;
            cout<<"10 init states: ";
            sim->setSeekerHiderPos(x,y,v);
            FOR(i,10) {
                pomcp::State* s = sim->genInitState();
                cout << s->toString()<<" ";
                delete s;
            }
            cout << endl;


            cout << " -- genRandomState --"<<endl;
            pomcp::HSState hsstate(x,y);
            cout<<"10 next random states from "<< hsstate.toString() <<": ";
            //sim->setSeekerHiderPos(x,y,v);
            FOR(i,10) {
                pomcp::State* s = sim->genRandomState(&hsstate,NULL);
                cout << s->toString()<<" ";
                delete s;
            }
            cout << endl;


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
            sim->getPossibleNextPos(x,posVec);
            FOREACHnc(Pos,p,posVec) {
                cout << p->toString()<<" ";
            }
            cout << endl;


            cout << " -- setInitialNodeValue --"<<endl;
            cout<<"set init node value from "<< hsstate.toString() <<": "<<flush;
            pomcp::Node node(sim,NULL);
            for(int a=-1;a<HSGlobalData::NUM_ACTIONS;a++) {
                sim->setInitialNodeValue(&hsstate,NULL,&node,a);   //(x,posVec);
                cout << " a="<<a<<" count="<<node.getCount()<<" v="<<node.getValue()<<"; ";
            }
            cout << endl;


            cout << " -- step --"<<endl;
            cout<<"step from "<< hsstate.toString() <<": ";
            for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
                double r=0;
                int o=-1;
                pomcp::State* nextS = sim->step(&hsstate,a,o,r);
                cout << " a="<<a<<"->next="<<nextS->toString()<<",o="<<o<<",r="<<r<<"; ";
                delete nextS;
            }
            cout << endl;


            cout << " -- isFinal --"<<endl;
            cout << hsstate.toString()<<": "<< sim->isFinal(&hsstate)<<endl;

            cout << "-- immediateReward --"<<endl;
            cout << hsstate.toString()<<": "<< sim->getImmediateReward(&hsstate)<<endl;

        } // if sim!=NULL

    } //while




    if (sim!=NULL) delete sim;
}



void testPOMCP(const char* mapFile, SeekerHSParams* params) {
    cout << "Test POMCP; map: " << mapFile << endl;
    GMap gmap(mapFile);
    cout << "Size: "<<gmap.rowCount()<<"x"<<gmap.colCount()<<endl;
    gmap.printMap();

    cout<<endl;
    pomcp::HSSimulator sim(&gmap, params);

    sim.testAllFunctions();
}

void writeMap(const char* mapFile, string mapFileOut, Pos base, int zoomOutFac) {
    cout << "Write map mode"<<endl;
    cout << endl<<"Loading map ... "<<endl;
    GMap gmap(mapFile, base, zoomOutFac);
    cout << "done"<<endl<<"Writing "<<mapFileOut<<"..."<<endl;
    gmap.writeMapFile(mapFileOut.c_str());
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



