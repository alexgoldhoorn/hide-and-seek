#include <QCoreApplication>

//#include "gameconnectorhider.h"
#include "Base/gameconnectorclient.h"

#include "Base/hsconfig.h"
#include "Base/hsglobaldata.h"

#include "AutoHider/randomhider.h"
#include "AutoHider/randomwalker.h"
#include "AutoHider/randomlisthider.h"
#include "AutoHider/smarthider.h"
#include "AutoHider/verysmarthider.h"
#include "AutoHider/verysmarthider2.h"
#include "AutoHider/fromlisthider.h"
#include "AutoHider/randomfixedhider.h"

#include <iostream>
#include <opencv/cv.h>

#include "Utils/generic.h"

#include "HSGame/gmap.h"

#include "exceptions.h"

using namespace std;
using namespace hsutils;


static const int FIRST_AUTOHIDER_PARAM_INDEX = 5;

//localhost 1120 11 0 /home/agoldhoorn/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/build/map3_10_steps.txt


void showHelp() {
    cout<<"HSAutoHider: error in the arguments while calling automated player, expecting: "<<endl
       << "     ip port hiderType [initPosDistToBase] [file | score_rand_std score_dhs_factor score_std_less_random_std [score_type_hidden score_type_action D2 f1 f2 k f3]]" << endl
       << " ip:port should be the server address" <<endl
       << " the hiderType should be: "<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM         <<": random"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM_WALKER  <<": random walker (random goals)"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_SMART          <<": smart"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST    <<": from file (extra parameter: actions file required)" <<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_SEEKER               <<": N/A" <<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART      <<": very smart"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING     <<": all knowning (smart)"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING    <<": all knowning (very smart)"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART2     <<": very smart 2"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_V2ALLKNOWING   <<": all knowning (very smart 2)"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_FILE           <<": from file (positions, extra parameter: positions file required)" <<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_FIXED_RAND_POS <<": hider fixed random pos" <<endl
       << endl << "score types:"<<endl
       <<"- "<<(int)VerySmartHider::SCORE_AVG                        <<": average (default)"<<endl
       <<"- "<<(int)VerySmartHider::SCORE_MIN                        <<": min"<<endl
       <<"- "<<(int)VerySmartHider::SCORE_MAX                        <<": max"<<endl
       <<endl<<"Version: -v"<<endl
       <<endl<<"initPosDistToBase: the init position distance to the base"<< endl
       <<endl<<"To generate a file that contains the positions, use the following parameters:"<<endl
       << "     -create hiderType map-file out-pos-file num-hiders num-steps C/D [other-start-pos-file V/H/n]"<<endl
       << " The hiderType can only be random ("<<HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM<<"), Random Walker/goal ("<<
          HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM_WALKER<<") or Fixed Random Pos ("<<
          HSGlobalData::OPPONENT_TYPE_HIDER_FIXED_RAND_POS<<")" <<endl
       << " The output file (out-pos-file) will take num-steps with num-hiders positions on each line"<<endl
       << " C/D: continuous/discrete"<<endl
       << " seeker-start-pos-file: the start position of the seeker."
       << " V/H/n: hider should start visible/hidden/at 1/2/3 cells of the seeker's position"
       <<endl<<"To test a positions file:"<<endl
       << "     -test pos-file map-file C/D"<<endl
       <<endl;
}


void setParams(int argc, char *argv[], SmartHider* smartHider) {
    double scoreRandStd = SmartHider::SCORE_DEFAULT_RAND_STD;
    double scoreDhsFactor = SmartHider::SCORE_DEFAULT_DHS_FACTOR;
    double scoreStdLessRandomDist = SmartHider::SCORE_DEFAULT_LESS_RAND_DIST;

    //get params
    if (argc>FIRST_AUTOHIDER_PARAM_INDEX) {
        QString s = QString::fromLatin1( argv[FIRST_AUTOHIDER_PARAM_INDEX] );
        bool ok = false;
        double v = s.toDouble(&ok);

        if (!ok ) {
            cout << "Expected a number (score rand. std.): "<<s.toStdString()<<endl;
            exit(-1);
        }
        scoreRandStd = v;

        if (argc>FIRST_AUTOHIDER_PARAM_INDEX+1) {
            s = QString::fromLatin1( argv[FIRST_AUTOHIDER_PARAM_INDEX+1] );
            v = s.toDouble(&ok);

            if (!ok ) {
                cout << "Expected a number (score d_hs factor): "<<s.toStdString()<<endl;
                exit(-1);
            }
            scoreDhsFactor = v;

            if (argc>FIRST_AUTOHIDER_PARAM_INDEX+2) {
                QString s = QString::fromLatin1( argv[FIRST_AUTOHIDER_PARAM_INDEX+2] );
                ok = false;
                v = s.toDouble(&ok);

                if (!ok ) {
                    cout << "Expected a number (score std. less random dist.): "<<s.toStdString()<<endl;
                    exit(-1);
                }
                scoreStdLessRandomDist = v;
            }
        }
    }

    smartHider->setParameters(scoreRandStd, scoreDhsFactor, scoreStdLessRandomDist);

    cout << "Parameters: "<<endl
         << "Score random std.dev. (n_max): "<<scoreRandStd<<endl
         << "Score d_hs factor    : "<<scoreDhsFactor<<endl
         << "Score std.dev. less distance (c_less): " << scoreStdLessRandomDist<<endl<<endl;
}


void setParams2(int argc, char *argv[], VerySmartHider2* smartHider, bool allKnowing) {
    setParams(argc,argv,smartHider);

    char scoreTypeHidden = VerySmartHider::SCORE_MIN;
    char scoreTypeAct = VerySmartHider::SCORE_MIN;
    double D2 = VerySmartHider2::SCORE_DEFAULT_D2;
    double f1 = VerySmartHider2::SCORE_DEFAULT_F1;
    double f2 = VerySmartHider2::SCORE_DEFAULT_F2;
    double k  = VerySmartHider2::SCORE_DEFAULT_K;
    double f3 = VerySmartHider2::SCORE_DEFAULT_F3;

    //ip port hiderType [actionFile | score_rand_std score_dhs_factor score_std_less_random_std
    // 7
    //[score_type_hidden score_type_action D2 f1 f2 k]]

    //now read params
    if (argc>FIRST_AUTOHIDER_PARAM_INDEX+3) {
        scoreTypeHidden = argv[FIRST_AUTOHIDER_PARAM_INDEX+3][0] - '0';

        if (argc>FIRST_AUTOHIDER_PARAM_INDEX+4) {
            scoreTypeAct = argv[FIRST_AUTOHIDER_PARAM_INDEX+4][0] - '0';

            if (argc>FIRST_AUTOHIDER_PARAM_INDEX+5) {
                QString s = QString::fromLatin1(argv[FIRST_AUTOHIDER_PARAM_INDEX+5]);
                bool ok = false;
                D2 = s.toDouble(&ok);
                if (!ok) {
                    cout << "Expected number for D2: "<<s.toStdString()<< endl;
                    exit(-1);
                }

                if (argc>FIRST_AUTOHIDER_PARAM_INDEX+6) {
                    s = QString::fromLatin1(argv[FIRST_AUTOHIDER_PARAM_INDEX+6]);
                    ok = false;
                    f1 = s.toDouble(&ok);
                    if (!ok) {
                        cout << "Expected number for f1: "<<s.toStdString()<< endl;
                        exit(-1);
                    }

                    if (argc>FIRST_AUTOHIDER_PARAM_INDEX+7) {
                        s = QString::fromLatin1(argv[FIRST_AUTOHIDER_PARAM_INDEX+7]);
                        ok = false;
                        f2 = s.toDouble(&ok);
                        if (!ok) {
                            cout << "Expected number for f2: "<<s.toStdString()<< endl;
                            exit(-1);
                        }

                        if (argc>FIRST_AUTOHIDER_PARAM_INDEX+8) {
                            s = QString::fromLatin1(argv[FIRST_AUTOHIDER_PARAM_INDEX+8]);
                            ok = false;
                            k = s.toDouble(&ok);
                            if (!ok) {
                                cout << "Expected number for k: "<<s.toStdString()<< endl;
                                exit(-1);
                            }

                            if (argc>FIRST_AUTOHIDER_PARAM_INDEX+9) {
                                s = QString::fromLatin1(argv[FIRST_AUTOHIDER_PARAM_INDEX+9]);
                                ok = false;
                                f3 = s.toDouble(&ok);
                                if (!ok) {
                                    cout << "Expected number for f3: "<<s.toStdString()<< endl;
                                    exit(-1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    smartHider->setParameters2(D2,f1,f2,k,f3,allKnowing,scoreTypeHidden,scoreTypeAct);


    cout //<< "Parameters: "<<endl
         << "Score type hidden:      "<<(int)scoreTypeHidden<<endl
         << "Score type actions:     "<<(int)scoreTypeAct<<endl
         << "Score bonus (D2):       " << D2<<endl
         << "Score factor Dhs 1 (f1): "<<f1<<endl
         << "Score factor Dhs 2 (f2): "<<f2<<endl
         << "Min dist. seeker (k):  : "<<k<<endl
         << "Score Dhb factor 3 (f3): "<<f3<<endl<<endl;
}


void showVersion() {
    cout << "Hide&Seek AutoHider v"<<HS_VERSION<<endl<<endl;
    cout << "Build: "<<__DATE__<<" "<<__TIME__<<endl<<endl;
    cout << "Compiled with: "<<endl;
    cout << " - Qt " << QT_VERSION_STR <<endl;
    cout << " - OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION<<endl<<endl;
    cout << "xGNU compiler version: " <<__VERSION__ << " (" << _MACHINEBITS << " bits)" <<endl;//<<__GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__<<endl;
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
    DEBUG_AUTOHIDER(cout << "auto hider debug (DEBUG_AUTOHIDER); ";);
    cout << endl;
    cout << "Random (1-10): "<<random(1,10)<<endl;
}

void testPosList(QString mapFile, QString posFile, SeekerHSParams* params) {
    //AG150602: TODO fix this for use of Multi agent code
    cout << "Error: test function has to be adapted for multi agent code"<<endl;
    exit(1);
#ifdef OLD_CODE
    cout <<"Opening map file: "<<mapFile.toStdString()<<endl;
    GMap gmap(mapFile.toStdString(), params);
    gmap.printMap();

    cout << "Using continuous: "<<(params->useContinuousPos?"yes":"no")<<endl;

    cout<<"Creating fromListHider.. "<<endl;
    FromListHider fromListHider(params, posFile.toStdString());
    AutoWalker* autoWalker = &fromListHider;
    cout <<" Num hiders: "<<fromListHider.getNumberOfHiders()<<endl<<" Num steps: "<<fromListHider.getNumberOfSteps()<<endl;
    uint numSteps = fromListHider.getNumberOfSteps();
    uint numHiders = fromListHider.getNumberOfHiders();

    //AG150602: add player info
    //PlayerInfo pInfo;

    Pos p;
    cout <<"Init belief (not used): "<<flush;
    //AG150602: change params
    autoWalker->initBelief(&gmap, &pInfo); // (&gmap,p,p,false);
    cout<<"ok"<<endl;

    cout<<"Reading all poses:"<<endl;
    for(uint i=0; i<numSteps; i++) {
        vector<IDPos> posVec = /*fromListHider.*/ autoWalker->getAllNextPos(p,p);
        cout <<" step "<<i<<": ";
        for(uint j=0; j<numHiders; j++) {
            cout<<posVec[j].toString()<<"; ";
        }
        cout<<endl;
    }

    cout << endl<<"Now as autohider:";
    FromListHider fromListHider2(params, posFile.toStdString());
    AutoHider* autoHider = &fromListHider2;
    cout<<"Reading all poses:"<<endl;
    IDPos idpos(0,0,0);

    autoHider->initBelief(gmap, &pInfo); // (&gmap,p,idpos,false);
    vector<int> actions;
    for(uint i=0; i<numSteps; i++) {
        p = /*fromListHider.*/ autoHider->getNextPos(p, idpos, false, actions);
        cout <<" step "<<i<<": "<<p.toString()<<endl;
    }

    cout <<endl<<"END OF TEST"<<endl;
#endif
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SeekerHSParams params;

    //ag140521: reset locale after Qt sets to system's local (http://qt-project.org/doc/qt-5/QCoreApplication.html#locale-settings)
    setlocale(LC_NUMERIC,"C");
    //ag120208: set locale to en-US such that the atof uses decimal dots and not comma
    setlocale(LC_NUMERIC,"en_US");

    //return result
    int retRes = 0;

    try
    {
        //check params
        if (argc>1 && argv[1][0]=='-') {
            if (argv[1][1]=='v') {
                showVersion();
                return 0;
            }
        }
       if(argc < 4) {
            showHelp();
            exit(-1);
        }

       bool writeFile = false;
       int hiderType = 0;
       //AG150602: depricated -> use SeekerHSParams
       //int randomPosDistToB = -1;
       uint numHiders = 0;

       QString firstParamStr = QString::fromLatin1(argv[1]);
       if (firstParamStr.compare("-create",Qt::CaseInsensitive)==0 || firstParamStr.compare("-write",Qt::CaseInsensitive)==0) {
           cout << "Create a positions file"<<endl;
           //<< "     -create hiderType map-file out-pos-file num-hiders num-steps"<<endl
           writeFile = true;

           QString hiderTStr = QString::fromLatin1(argv[2]);
           bool ok = false;
           hiderType = hiderTStr.toInt(&ok);
           if (!ok) {
               cout << "hidertype not an int: "<<hiderTStr.toStdString() <<endl;
               exit(-1);
           }

           QString numHiderStr = QString::fromLatin1(argv[5]);
           numHiders = numHiderStr.toUInt(&ok);
           if (!ok) {
               cout << "Number of hiders is not an uint"<<endl;
               exit(-1);
           }

       } else if (firstParamStr.compare("-test",Qt::CaseInsensitive)==0) {
           //<< "     -test pos-file map-file C/D"<<endl
           QString posFile = QString::fromLatin1(argv[2]);
           QString mapFile = QString::fromLatin1(argv[3]);
           QString useContStr = QString::fromLatin1(argv[4]);
           //cout << "STRING USECONT: "<<useContStr.toStdString()<<endl;
           params.useContinuousPos = (useContStr.compare("c",Qt::CaseInsensitive)==0);

           testPosList(mapFile, posFile, &params);

           return 0;

       } else {
            //ip, port,hidertype
            params.serverIP = firstParamStr.toStdString();
            QString portStr = QString::fromLatin1(argv[2]);
            QString hiderTStr = QString::fromLatin1(argv[3]);
            bool ok = false;
            params.serverPort = portStr.toInt(&ok);
            if (!ok) {
                cout << "Port not an int: "<<portStr.toStdString()<<endl;
                exit(-1);
            }
            hiderType = hiderTStr.toInt(&ok);
            if (!ok) {
                cout << "hidertype not an int: "<<hiderTStr.toStdString()<<endl;
                exit(-1);
            }

            //random pos dist to base
            if (argc > 4 ) {
                QString randPDStr = QString::fromLatin1(argv[4]);
                //AG150602: depricated -> use SeekerHSParams
                params.randomPosDistToBase = randPDStr.toInt(&ok);
                if (!ok) {
                    cout << "randomPosDistToBase is not an int"<<endl;
                    exit(-1);
                }
            }

            cout << "Automated Hider "<<endl<<endl
                 << "Automated hider Type:   "<<(int)hiderType<<endl
                 << "Server:                 "<<params.serverIP<<":"<<params.serverPort<<endl
                 << "Random pos. dist. to b.:" << params.randomPosDistToBase <<endl<<endl;
        }

        AutoHider* autoHider = NULL;
        AutoWalker* autoWalker = NULL;
        switch (hiderType) {
            case HSGlobalData::OPPONENT_TYPE_HIDER_SMART: {
                SmartHider* smartHider = new SmartHider(&params);
                autoHider = smartHider;
                setParams(argc,argv,smartHider);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING: {
                SmartHider* smartHider = new SmartHider(&params,true);
                autoHider = smartHider;
                setParams(argc,argv,smartHider);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM: {
                RandomHider* randomHider = new RandomHider(&params, (size_t)numHiders);
                autoHider = randomHider;
                autoWalker = randomHider;
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM_WALKER: {
                RandomWalker* randomWalker = new RandomWalker(&params, (size_t)numHiders);
                autoHider = randomWalker;
                autoWalker = randomWalker;
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_FILE:
            case HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST: {
                DEBUG_HS(cout << "getting actionfile "<<endl;);
                if (argc<=FIRST_AUTOHIDER_PARAM_INDEX) {
                    cout << "Action file required"<<endl;
                    exit(-1);
                }
                QString file = QString::fromLatin1(argv[FIRST_AUTOHIDER_PARAM_INDEX]);

                if  (hiderType==HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST) {
                    RandomListHider* randomListHider = new RandomListHider(&params,file.toStdString());
                    autoHider = randomListHider;
                } else  if  (hiderType==HSGlobalData::OPPONENT_TYPE_HIDER_FILE) {
                    FromListHider* fromListHider = new FromListHider(&params, file.toStdString());
                    autoHider = fromListHider;
                }
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART: {
                VerySmartHider *verySmartHider = new VerySmartHider(&params);
                autoHider = verySmartHider;
                setParams(argc,argv,verySmartHider);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING: {
                VerySmartHider *verySmartHider = new VerySmartHider(&params,true);
                autoHider = verySmartHider;
                setParams(argc,argv,verySmartHider);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART2: {
                VerySmartHider2 *verySmartHider2 = new VerySmartHider2(&params);
                autoHider = verySmartHider2;
                setParams2(argc,argv,verySmartHider2,false);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_V2ALLKNOWING: {
                VerySmartHider2 *verySmartHider2 = new VerySmartHider2(&params,true);
                autoHider = verySmartHider2;
                setParams2(argc,argv,verySmartHider2,true);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_FIXED_RAND_POS: {
                RandomFixedHider *fixedHider = new RandomFixedHider(&params, (size_t)numHiders);
                autoHider = fixedHider;
                autoWalker = fixedHider;
                break;
            }
            default:
                //not hider
                cout << "ERROR: unknown hider"<<endl;
                assert(false);
                break;
        }

        cout << "Starting "<<autoHider->getName()<<" ..."<<endl;

        if (writeFile) {
            //<< "     -create hiderType map-file out-pos-file num-hiders num-steps C/D"<<endl
            QString mapFile = QString::fromLatin1(argv[3]);
            QString outFile = QString::fromLatin1(argv[4]);
            QString numStepsStr = QString::fromLatin1(argv[6]);
            QString useContStr = QString::fromLatin1(argv[7]);
            //AG160128: use of seeker location to start hider hidden / visible
            QString otherLocFileStr;
            bool startVisibFromSeeker = false;
            double seekerDist = -1.0;

            bool ok = false;
            uint numSteps = numStepsStr.toUInt(&ok);
            if (!ok) {
                cout << "Number of steps not an int"<<endl;
                exit(-1);
            }
            params.useContinuousPos = (useContStr.compare("c",Qt::CaseInsensitive)==0);

            if (argc>8) {
                //use seeker distance
                otherLocFileStr = QString::fromLatin1(argv[8]);

                if (argc>9) {
                    QString seekerDistVisibStr = QString::fromLatin1(argv[9]);
                    if (seekerDistVisibStr.compare("v",Qt::CaseInsensitive)==0) {
                        startVisibFromSeeker = true;
                    } else if (seekerDistVisibStr.compare("h",Qt::CaseInsensitive)==0) {
                        startVisibFromSeeker = false;
                    } else {
                        //start visib
                        startVisibFromSeeker = true;
                        //should be a number
                        seekerDist = seekerDistVisibStr.toDouble(&ok);
                        if (!ok) {
                            cout << "Number expected for distance to seeker, but this is not a double."<<endl;
                            exit(-1);
                        }
                    }
                }
            }

            cout << "Automated Hider "<<endl<<endl
                 << "Automated hider Type:   "<<(int)hiderType<<endl
                 << "Map: "<<mapFile.toStdString()<<endl
                 << "Out action file: " << outFile.toStdString()<<endl
                 << "Continuous movements: "<<(params.useContinuousPos?"yes":"no") <<endl
                 << "Use seeker pos. as reference: ";

            if (otherLocFileStr.isEmpty()) {
                cout << "no"<<endl;
            } else {
                cout << "yes"<<endl
                     << "Other pos. file: "<<otherLocFileStr.toStdString()<<endl
                     << "Auto player should start: "<<(startVisibFromSeeker?"visible":"hidden")<<endl
                     << "Distance to other: ";
                if (seekerDist<0)
                    cout << "none"<<endl;
                else
                    cout <<seekerDist<<endl;

            }

            cout <<endl;

            GMap gmap(mapFile.toStdString(), &params);
            gmap.printMap();

            // write pos. file
            FromListHider::writePosFile(autoWalker, &gmap, outFile.toStdString(), numHiders, numSteps, params.useContinuousPos,
                                        otherLocFileStr.toStdString(), startVisibFromSeeker, seekerDist);

        } else {
            //GameConnectorHider gc(ip, port, autoHider);
            //AG140528: set all in params, such that the SAME struct is used
            params.userName = autoHider->getName();
            params.mapID = 0;
            params.opponentType = 0;
            params.isSeeker = false;

            //open game connector
            QString comments = "hsautohider ";
            comments += QString::fromStdString(autoHider->getName());
            GameConnectorClient gc(&params, argsToQString(argc,argv), comments, autoHider);
            gc.setExitOnDisconnect(true);
            bool ok = gc.connectToServer();

            if (!ok ) {
                cout << "Failed to connect to the server"<<endl;
            } else {
                //everything OK, now just wait for
                retRes = a.exec();
            }
        }

    }
    catch(CException &ce) {
        cout <<"CException: "<<ce.what()<<endl;
    }
    catch(exception &e)
    {
        cout << "Exception: " << e.what() << endl ;

    }


    return retRes;
}
