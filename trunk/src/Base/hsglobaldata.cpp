
#include "Base/hsglobaldata.h"

#include <limits>
#include <cmath>


/*const double HSGlobalData::INFTY_NEG_DBL = -1e100;// std::numeric_limits<double>::min();
const double HSGlobalData::INFTY_POS_DBL = 1e100;//std::numeric_limits<double>::max();
const float HSGlobalData::INFTY_NEG_FLT = -1e30; //std::numeric_limits<float>::min();
const float HSGlobalData::INFTY_POS_FLT = 1e30; //std::numeric_limits<float>::max();*/


#ifdef USE_QT

const std::string HSGlobalData::DEFAULT_SERVER_IP = "localhost";

const QStringList HSGlobalData::MAPS = QStringList()
                           << "map1_6x5_0o.txt"<<"map2_6x5_1o.txt"<<"map3_6x5_2o.txt"
                           << "map4_6x5_2o.txt"<<"map5_6x5_4o.txt"<< "map1_40x40_1o.txt"
                           << "map2_40x40_2o.txt"<< "map3_40x40_3o.txt"<< "map4_40x40_4o.txt"
                           << "map5_40x40_5o.txt"<< "testmapbig1.txt" << "testmapbig2.txt"
                           << "map1_10x10_1o.txt" << "map2_10x10_2o.txt" << "map3_10x10_3o.txt"
                           << "map4_10x10_4o.txt" << "map5_10x10.txt" << "map6_10x10.txt"
                           << "map7_10x10.txt"  << "mapbcn1.txt" << "mapbcn2.txt"
                           << "mapbcn3.txt" << "mapbcn1a.txt" << "mapbcn1b.txt"
                           << "mapbcn1c.txt" << "map1_20x20_0o.txt" << "map2_20x20_1o.txt"
                           << "map3_20x20_2o.txt" << "map4_20x20_3o.txt" << "map5_20x20_4o.txt"
                           << "map1_3x3_0o.txt" << "map2_3x3_1o.txt" << "map3_3x3_2o.txt"
                           << "map1_5x5_0o.txt" << "map2_5x5_1o.txt" << "map3_5x5_2o.txt"
                           << "map1_7x7_0o.txt" << "map2_7x7_1o.txt" << "map3_7x7_2o.txt"
                           // FROM HERE copied
                           << "map1_40x40.txt"
                           << "map2_40x40.txt"
                           << "map3_40x40.txt"
                           << "map4_40x40.txt"
                           << "map5_40x40.txt"
                           << "map1_20x20.txt"
                           << "map2_20x20.txt"
                           << "map3_20x20.txt"
                           << "map4_20x20.txt"
                           << "map5_20x20.txt"
                           << "map1_17x17.txt"
                           << "map2_17x17.txt"
                           << "map3_17x17.txt"
                           << "map4_17x17.txt"
                           << "map1_15x15.txt"
                           << "map2_15x15.txt"
                           << "map3_15x15.txt"
                           << "map4_15x15.txt"
                           << "map1_12x12.txt"
                           << "map2_12x12.txt"
                           << "map3_12x12.txt"
                           << "map4_12x12.txt"
                           << "map1_10x10.txt"
                           << "map2_10x10.txt"
                           << "map3_10x10.txt"
                           << "map4_10x10.txt"
                           << "map1_7x7.txt"
                           << "map2_7x7.txt"
                           << "map3_7x7.txt"
                           << "map1_6x5.txt"
                           << "map2_6x5.txt"
                           << "map3_6x5.txt"
                           << "map4_6x5.txt"
                           << "map5_6x5.txt"
                           << "map1_5x5.txt"
                           << "map2_5x5.txt"
                           << "map3_5x5.txt"
                           << "map1_3x3.txt"
                           << "map2_3x3.txt"
                           << "map3_3x3.txt"
                           << "fme_place/map1_10x12.txt"
                           << "fme_place/map1_12x14.txt"
                           << "fme_place/map2_10x12.txt"
                           << "fme_place/map2_12x14.txt"
                           << "fme_place/map3_10x12.txt"
                           << "fme_place/map3_12x14.txt"
                           << "fme_place/map_fmep1_10x12.txt"
                           << "fme_place/map_fmep1_12x14.txt"
                           << "fme_place/map_fmep2_10x12.txt"
                           << "fme_place/map_fmep2_12x14.txt"
                           << "fme_place/map_fmep3_10x12.txt"
                           << "fme_place/map_fmep3_12x14.txt"
                           << "map1_10x12.txt"
                           << "map1_12x14.txt"
                           << "map2_10x12.txt"
                           << "map2_12x14.txt"
                           << "map3_10x12.txt"
                           << "map3_12x14.txt"
                           << "fme_place/map1_9x10.txt"
                           << "fme_place/map2_9x10.txt"
                           << "fme_place/map3_9x10.txt"
                           << "map1_9x10.txt"
                           << "map2_9x10.txt"
                           << "map3_9x10.txt"
                           << "map0_8x6.txt"
                           << "map1_8x6.txt"
                           << "map0_6x8.txt"
                           << "map1_6x8.txt"
                           << "map0_7x9.txt"
                           << "map1_7x9.txt"
                           << "map2_7x9.txt"
                           << "map3_7x9.txt"
                           << "mapbcn_full.txt"
                           << "fme_place/map1_9x12.txt"
                           << "fme_place/map2_9x12.txt"
                           << "map1_9x12.txt"
                           << "map2_9x12.txt"
                           << "mapbcn4.txt"
                           << "mapbcn5.txt"
                           << "mapbcn6.txt"
                           << "bcn_lab/fme2014_map4.txt"
                           << "brl/brl29a.txt"
                           << "brl/master29e.txt"
                           << "brl/master29f.txt"
                           << "bcn_lab/upc_campus_1m.txt"
                           << "ext/nsh.txt"
                           << "ext/museum.txt"
                           << "brl/master2015s.txt"
                           << "brl/master2015s2.txt"
                           << "brl/master2016full.txt"
                           << "brl/master2016med.txt"
                           << "brl/master2016.txt";


//ACTIONS = [HALT; N_ACT; NE_ACT; E_ACT; SE_ACT; S_ACT; SW_ACT; W_ACT; NW_ACT];
const QStringList HSGlobalData::ACTIONS = QStringList()
        << "none" << "halt" << "north" << "north-east" << "east" << "south-east" << "south" << "south-west" << "west" << "north-west";

const QStringList HSGlobalData::ACTIONS_SHORT = QStringList()
        << "h" << "n" << "ne" << "e" << "se" << "s" << "sw" << "w" << "nw";

const QStringList HSGlobalData::WINSTATE_LIST = QStringList()
        << "running" << "seeker won" << "hider won" << "tie";

//AG150121: names of enum MessageSendTo
const QStringList HSGlobalData::MESSAGESENDTO_NAMES = QStringList()
        << "Hider"<<"Seeker 1"<<"Seeker 2"<<"All"<<"All Clients"<<"All Seekers"<<"Server";


const QStringList HSGlobalData::PLAYER_TYPE_NAMES = QStringList()
        << "Hider"<<"Seeker"; //<<"Seeker 2";

#else

std::string HSGlobalData::getActionName(int actionID) {
    std::string actName = "?";
    switch (actionID) {
    case HSGlobalData::ACT_H:
    actName="h";
    break;
    case HSGlobalData::ACT_N:
    actName="n";
    break;
    case HSGlobalData::ACT_NE:
    actName="ne";
    break;
    case HSGlobalData::ACT_E:
    actName="e";
    break;
    case HSGlobalData::ACT_SE:
    actName="se";
    break;
    case HSGlobalData::ACT_S:
    actName="s";
    break;
    case HSGlobalData::ACT_SW:
    actName="sw";
    break;
    case HSGlobalData::ACT_W:
    actName="w";
    break;
    case HSGlobalData::ACT_NW:
    actName="nw";
    break;
    }
    return actName;
}

#endif

double HSGlobalData::getActionDirection(char action) {
    if (action==ACT_DIR_H) {
        //halt action has no direction
        return 0;
    } else {
        //use index of action, and the fact that each next action is 45º more
        return (action-1)*M_PI_4;
    }
}


