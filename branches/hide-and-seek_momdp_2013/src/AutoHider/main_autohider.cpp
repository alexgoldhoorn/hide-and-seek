#include <QCoreApplication>
#include "gameconnectorhider.h"
#include <iostream>
#include "hsconfig.h"
#include "hsglobaldata.h"

#include "AutoHider/randomhider.h"
#include "AutoHider/randomlisthider.h"
#include "AutoHider/smarthider.h"
#include "AutoHider/verysmarthider.h"

using namespace std;


void showHelp() {
    cout<<"HSAutoHider: error in the arguments while calling automated player, expecting: "<<endl
       << "     ip port hiderType [actionFile | score_rand_std score_dhs_factor score_std_less_random_std]" << endl
       << " ip:port should be the server address" <<endl
       << " the hiderType should be: "<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM         <<": random"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_SMART          <<": smart"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST    <<": from file (extra parameter: actionFile required)" <<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_SEEKER               <<": N/A" <<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART      <<": very smart"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING     <<": all knowning (smart)"<<endl
       <<"- "<<HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING    <<": all knowning (very smart)"<<endl
       <<endl;
}


void setParams(int argc, char *argv[], SmartHider* smartHider) {
    double scoreRandStd = SmartHider::SCORE_DEFAULT_RAND_STD;
    double scoreDhsFactor = SmartHider::SCORE_DEFAULT_DHS_FACTOR;
    double scoreStdLessRandomDist = SmartHider::SCORE_DEFAULT_LESS_RAND_DIST;

    //get params
    if (argc>4) {
        QString s = QString::fromAscii( argv[4] );
        bool ok = false;
        double v = s.toDouble(&ok);

        if (!ok ) {
            cout << "Expected a number (score rand. std.): "<<s.toStdString()<<endl;
            exit(-1);
        }
        scoreRandStd = v;

        if (argc>5) {
            s = QString::fromAscii( argv[5] );
            v = s.toDouble(&ok);

            if (!ok ) {
                cout << "Expected a number (score d_hs factor): "<<s.toStdString()<<endl;
                exit(-1);
            }
            scoreDhsFactor = v;

            if (argc>6) {
                QString s = QString::fromAscii( argv[6] );
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
         << "Score random std.dev.: "<<scoreRandStd<<endl
         << "Score d_hs factor    : "<<scoreDhsFactor<<endl
         << "Score std.dev. less distance: " << scoreStdLessRandomDist<<endl<<endl;
}

int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    //tSingle
    QCoreApplication a(argc, argv);

    try
    {


        if(argc < 4) {
            showHelp();
            exit(-1);
        }

        //(QString ip, int port, int hiderType, QString actionFile) {
        QString ip = QString::fromAscii(argv[1]);

        QString portStr = QString::fromAscii(argv[2]);
        QString hiderTStr = QString::fromAscii(argv[3]);
        bool ok = false;
        int port = portStr.toInt(&ok);
        if (!ok) {
            cout << "Port not an int"<<endl;
            exit(-1);
        }
        int hiderType = hiderTStr.toInt(&ok);
        if (!ok) {
            cout << "hidertype not an int"<<endl;
            exit(-1);
        }

        AutoHider* autoHider = NULL;
        switch (hiderType) {
            case HSGlobalData::OPPONENT_TYPE_HIDER_SMART: {
                SmartHider* smartHider = new SmartHider();
                autoHider = smartHider;
                setParams(argc,argv,smartHider);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING: {
                SmartHider* smartHider = new SmartHider(true);
                autoHider = smartHider;
                setParams(argc,argv,smartHider);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM: {
                autoHider = new RandomHider();
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST: {
                DEBUG_HS(cout << "getting actionfile "<<endl;);
                if (argc<5) {
                    cout << "Action file required"<<endl;
                    exit(-1);
                }
                QString actionFile = QString::fromAscii(argv[4]);

                RandomListHider* randomListHider = new RandomListHider(actionFile.toStdString());
                autoHider = randomListHider;
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART: {
                VerySmartHider *verySmartHider = new VerySmartHider();
                autoHider = verySmartHider;
                setParams(argc,argv,verySmartHider);
                break;
            }
            case HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING: {
                VerySmartHider *verySmartHider = new VerySmartHider(true);
                autoHider = verySmartHider;
                setParams(argc,argv,verySmartHider);
                break;
            }
            default:
                //not hider
                cout << "ERROR: unknown hider"<<endl;
                assert(false);
                break;
        }


        cout << "Starting "<<autoHider->getName()<<" ..."<<endl;


        GameConnectorHider gc(ip, port, autoHider);



    }
    catch(exception &e)
    {
        cout << "Exception: " << e.what() << endl ;
    }


    return a.exec();
}
