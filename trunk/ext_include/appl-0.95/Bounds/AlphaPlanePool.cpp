#include "AlphaPlanePool.h"
#include "BeliefCache.h"
#include "SARSOP.h"
#include <exception>
#include <stdexcept>
using namespace std;


AlphaPlanePool::AlphaPlanePool(Backup<AlphaPlane> *_backupEngine) {
    this->setBackupEngine(_backupEngine);
}

AlphaPlanePool::~AlphaPlanePool(void) {
}

SharedPointer<AlphaPlane> AlphaPlanePool::getValueAlpha(SharedPointer<Belief>& belief) {
    return getBestAlphaPlane(belief, -1);
}

void AlphaPlanePool::addAlphaPlane(SharedPointer<AlphaPlane> plane) {
    planes.push_back(plane);
}



SharedPointer<AlphaPlane> AlphaPlanePool::getBestAlphaPlane( SharedPointer<belief_vector>& b, int index ) { //  belief, belief index
    DEBUG_TRACE( cout << "getBestAlphaPlane b" << endl; );
    DEBUG_TRACE( b->write(cout) << endl; );

    double val,maxval = -99e+20;
    SharedPointer<AlphaPlane> ret = NULL;
    int lastTimeStamp, maxTimeStamp, bestTimeStamp;
    lastTimeStamp = maxTimeStamp = bestTimeStamp = -1;

    //assert(this->sval == cn.s->sval);

    SARSOP* sarsopSolver = (SARSOP *)solver;

    DEBUG_TRACE( cout << "index " << index << endl; );

    if(index != -1) {
        maxval = beliefCache->getRow(index)->LB;

        lastTimeStamp = dataTable->get(index).ALPHA_TIME_STAMP;
        maxTimeStamp = lastTimeStamp;
        list<SharedPointer<AlphaPlane> >* alphas = dataTable->get(index).ALPHA_PLANES;
        //initialize best plane
        DEBUG_TRACE( cout << "alphas->size()" << alphas->size() << endl; );
        if(alphas->size()>0) {
            ret = alphas->front();
        }
        //for debugging purpose
        else {
            DEBUG_TRACE( cout << "no alpha plane presents" << endl; );
        }
    }


    DEBUG_TRACE( cout << "alphaPlanePool->planes.size() " << this->planes.size() << endl; );
    DEBUG_TRACE( cout << "maxval " << maxval << endl; );

    LISTFOREACH(SharedPointer<AlphaPlane>, pr,  this->planes) {
        SharedPointer<AlphaPlane> al = *pr;

        DEBUG_TRACE( cout << "al->timeStamp" << al->timeStamp  << endl; );
        DEBUG_TRACE( cout << "lastTimeStamp" << lastTimeStamp << endl; );

        if(al->timeStamp > lastTimeStamp) {
#if USE_MASKED_ALPHA
            if (!mask_subset( b, al->mask )) continue;
#endif
            val = inner_prod(*al->alpha, *b );
            DEBUG_TRACE( cout << "val = inner_prod(al->alpha, b ); alpha:" << endl );
            DEBUG_TRACE( al->alpha->write( cout ) << endl );

            DEBUG_TRACE( cout << "val " << val << endl; );
            DEBUG_TRACE( cout << "maxval " << maxval << endl; );
            //DEBUG_TRACE( cout << "al->alpha "; );
            //DEBUG_TRACE( al->alpha->write(cout) << endl; );
            //DEBUG_TRACE( cout << "b "; );
            //DEBUG_TRACE( b->write(cout) << endl; );

            if (val >= maxval - 0.0000000000001) {
                maxval = val;
                ret = al;
            }
            if (al->timeStamp > maxTimeStamp) {
                maxTimeStamp = al->timeStamp;
            }
        }
    }

    DEBUG_TRACE( cout << "maxTimeStamp" << maxTimeStamp << endl; );
    //cout << "finished with alphaPlanePool->planes.size() " << alphaPlanePool->planes.size() << endl;

    if(index != -1) {
        if(ret!= NULL) {
            //reset best alpha parameters if a better one is found
            dataTable->set(index).ALPHA_TIME_STAMP = maxTimeStamp;
            beliefCache->getRow( index)->LB = maxval;


            list<SharedPointer<AlphaPlane> >* alphas = dataTable->get(index).ALPHA_PLANES;
            if(alphas->size()>0) {
                SharedPointer<AlphaPlane> frontAlpha = alphas->front();
                SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)frontAlpha->solverData;
                tempTuple->certed --;
                alphas->pop_front();
            }

            //update value of certs to avoid wrong delete
            ((SARSOPAlphaPlaneTuple *)ret->solverData)->certed ++;
            alphas->push_front(ret);
        }
    }

    if(ret == NULL) {
        //cout << "calling getBestAlphaPlane1(b) " << endl;
        return getBestAlphaPlane1(b);
    }

    assert(ret!=NULL);

    return ret;
}
SharedPointer<AlphaPlane> AlphaPlanePool::getBestAlphaPlane( BeliefTreeNode& cn) {
    SharedPointer<belief_vector>  b = cn.s->bvec;
    int index = cn.cacheIndex.row;
    return getBestAlphaPlane(b, index ) ;
}

SharedPointer<AlphaPlane> AlphaPlanePool::getBestAlphaPlane( SharedPointer<belief_vector>& b) {
    int index = beliefCache->getBeliefRowIndex(b);
    return getBestAlphaPlane(b, index ) ;
}


//AG121010: return all calculated values for each action in an array
SharedPointer<AlphaPlane> AlphaPlanePool::getBestAlphaPlane1( SharedPointer<belief_vector>& b, REAL_VALUE* maxValue, bool showActions, vector<double>* maxValPerAction) {
    //AG120914: added max value as out param
    //AG121010: show all actions and its reward
#ifdef DEBUG_AG_POL2
    vector<double>* maxValPerA = NULL;

    if (maxValue!=NULL || showActions) {
        //prepare vector for max value per action
        maxValPerA = new vector<double>(9);
        FORs(i,9) {
            (*maxValPerA)[i] = -99e+20;
        }

        if (maxValPerAction!=NULL) maxValPerAction->resize(9);
        if (showActions) cout << " APPL, best act (a,v): "; ///AG
    }
#endif    //<ag

    double val, maxval = -99e+20;
    SharedPointer<AlphaPlane> ret = NULL;

    LISTFOREACH(SharedPointer<AlphaPlane>, pr, this->planes) {
        SharedPointer<AlphaPlane> al = *pr;
        val = inner_prod( *(al->alpha), *b );

#ifdef DEBUG_AG_POL2
        //if (maxValue != NULL) cout << "("<<al->action<<","<<val<<") "; ///AG
        //set max val in vector per action
        if (maxValPerA != NULL)
            if (val > (*maxValPerA)[al->action]) (*maxValPerA)[al->action] = val;
#endif    //<ag

        if (val > maxval) {
            maxval = val;
            ret = al;
        }
    }


#ifdef DEBUG_AG_POL2
    //show max val per action and put in out vector
    if (maxValPerA != NULL) {
        FORs(i,9) {
            //show
            if (showActions) {
                cout << "("<<i<<",";
                if ((*maxValPerA)[i]<-98e+20)
                    cout << "min";
                else
                    cout << (*maxValPerA)[i];
                cout <<") ";
            }

            //put in param vector
            if (maxValPerAction!=NULL) (*maxValPerAction)[i] = (*maxValPerA)[i];
        }

        if (showActions) cout<<endl;///AG

        delete maxValPerA;
    }
#endif    //<ag

    //AG120914: max value passed as out param
    if (maxValue != NULL) *maxValue = maxval;

    return ret;
}


REAL_VALUE AlphaPlanePool::getValue(SharedPointer<Belief>& belief) {
    double v = inner_prod( *getBestAlphaPlane(belief)->alpha, *belief );
    return v;
}
double AlphaPlanePool::getValue( SharedPointer<belief_vector>& belief, SharedPointer<AlphaPlane>* resultBestAlpha) {
    SharedPointer<AlphaPlane>bestAlpha = getBestAlphaPlane(belief);
    *resultBestAlpha = bestAlpha;
    double v = inner_prod( *(bestAlpha->alpha), *belief );
    return v;
}


SharedPointer<AlphaPlane> AlphaPlanePool::backup(BeliefTreeNode * node) {
    SharedPointer<AlphaPlane> result = backupEngine->backup(node);
    for(size_t i = 0 ; i < onBackup.size(); i++) {
        (*onBackup[i])(solver, node, result);
    }
    throw runtime_error("Not finished...");
    return result;
}



void AlphaPlanePool::setProblem(SharedPointer<MOMDP> p) {
    problem = p;
}
void AlphaPlanePool::setSolver(PointBasedAlgorithm *p) {
    //solver = p;
}

void AlphaPlanePool::setBeliefCache(BeliefCache *p) {
    beliefCache = p;
}


void AlphaPlanePool::setDataTable(IndexedTuple<AlphaPlanePoolDataTuple> *p) {
    dataTable = p;
}

