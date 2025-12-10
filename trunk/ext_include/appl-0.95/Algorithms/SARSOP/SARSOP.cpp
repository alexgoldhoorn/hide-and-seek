#include <cerrno>
#include <cstring>
#include <iomanip>

#include "SARSOP.h"
#include "SARSOPPrune.h"
#include "MOMDP.h"
#include "BeliefValuePairPoolSet.h"
#include "AlphaPlanePoolSet.h"
#include "BeliefTreeNode.h"
#include "CPTimer.h"
#include "BlindLBInitializer.h"
#include "FastInfUBInitializer.h"
#include "BackupBeliefValuePairMOMDP.h"
#include "BackupAlphaPlaneMOMDP.h"


void printSampleBelief(list<cacherow_stval>& beliefNStates) {
    cout << "SampledBelief" <<  endl;
    for(list<cacherow_stval>::iterator iter =beliefNStates.begin(); iter != beliefNStates.end() ; iter ++) {
        cout <<  "[ " <<(*iter).row << " : " << (*iter).sval << " ] ";
    }
    cout <<  endl;
}


SARSOPSolveState::SARSOPSolveState () {
    elapsedTime=precision=lowBound=upBound=-1;
    numTrials=numBackups=numAlpha=numBeliefs=-1;
    lowBoundBackupTime = upBoundBackupTime = -1;
    upBoundBackupCountSinceLast = lowBoundBackupCountSinceLast = -1;
    sampleTime = pruneTime = -1;
    stopped = true;

    step0Time = step1Time = step2Time = step3Time = step4Time = step5Time = step6Time = 0;
}


void SARSOP::progressiveIncreasePolicyInteval(int& numPolicies) {
    if (numPolicies == 0) {
        this->solverParams->interval *= 10;
        numPolicies++;

    } else {
        if (numPolicies == 5) {
            this->solverParams->interval *= 5;
        } else if (numPolicies == 10) {
            this->solverParams->interval *= 2;
        } else if (numPolicies == 15) {
            this->solverParams->interval *= 4;
        }

        numPolicies++;
    }
}

SARSOP::SARSOP(SharedPointer<MOMDP> problem, SolverParams * solverParams) {
    this->problem = problem;
    this->solverParams = solverParams;
    beliefForest = new BeliefForest();
    sampleEngine = new SampleBP();
    ((SampleBP*)sampleEngine)->setup(problem, this);
    beliefForest->setup(problem, this->sampleEngine, &this->beliefCacheSet);
    numBackups = 0;

    //AG120316: log stream
    openLogStream();
}

SARSOP::~SARSOP(void) {
    //AG120316: log stream
    //AG120426: delete
    if (beliefForest!=NULL) delete beliefForest;
    if (sampleEngine!=NULL) delete sampleEngine;

}


void SARSOP::onLowerBoundBackup (PointBasedAlgorithm *solver, BeliefTreeNode * node, SharedPointer<AlphaPlane> backupResult) {
    // updating certs, etc
}

void SARSOP::onUpperBoundBackup (PointBasedAlgorithm *solver, BeliefTreeNode * node, SharedPointer<BeliefValuePair> backupResult) {
}



//AG120316: log stream
void SARSOP::openLogStream() { //SARSOP * sarsop){
    if (!_logStream.is_open()) {

        string logFile = "SARSOP_time_log.txt";

        //AG120418: check if file open
        fstream fileCheck;
        fileCheck.open(logFile.c_str(),ios_base::in);
        bool logExists = fileCheck.is_open();
        if (logExists) { //close
            fileCheck.close();
        }
        //open log for writing
        _logStream.open("SARSOP_time_log.txt",ios_base::app | ios_base::out);

        if (!logExists) {
            //write header
            _logStream  << "Time,What,col1,col2,col3,col4,col5,col6,col7,col8,col9,col10,col11,col12,col13,col14,col15,col16,col17"<<endl
                        << "[time],OPEN LOG"<<endl
                        << "[time],START, #X, #Y, #O, #A"<<endl
                        << "[time],SOLVE INIT,upper bound(s),lower bound(s),init bounds(s),init sample engine(s),init sarsop prune(s),init other,init belief tree(s),total init time" << endl
                        << "[time],SOLVE START,#iteration,elapsed time,#trials,#backups,lower bound,upper bound,precision,#alphas,#beliefs,total time" << endl
                        << "[time],SOLVE,#iteration,elapsed time,#trials,#backups,lower bound,upper bound,precision,#alphas,#beliefs,low bound backup time,up bound backup time,#low bound backups,#up bound backups,sample time,prune time,step 0(s),step 1(s),step 2(s),step 3(s),step 4(s),step 5(s),step 6(s),stop,total time" << endl;
        }

        printLogTime();
        _logStream <<"OPEN LOG"<<endl;


    }

}

void SARSOP::printLogTime() {
    char buf[256];
    struct tm * timeinfo;
    time_t tt;
    time(&tt);
    timeinfo = localtime(&tt);

    //http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    strftime(buf, 256,"%Y/%m/%d %H:%M:%S", timeinfo);
    //strcpy(buf,ctime(&tt));
    //buf[strlen(buf)-1]='\0';

    _logStream << "[" << buf  << "],";
}

/*ofstream* SARSOP::logStream(){
    if (_logStream==NULL || !_logStream->is_open()) {

        cout << "ERROR @ SARSOP.logStream: no log stream initialized, requires init with SARSOP object"<<endl;
        exit(EXIT_FAILURE);
    }
    return _logStream;
}*/




void SARSOP::solve(SharedPointer<MOMDP> problem) {



    try {
        //AG120316: log stream
        //openLogStream(problem);
        printLogTime();
        (_logStream) << "START,"
                     << problem->XStates->size()<<","<<problem->YStates->size()<<","<<problem->observations->size()
                     <<","<<problem->getNumActions()<<endl;
        printLogTime();
        _logStream<<"SOLVE INIT,";//<<endl;

        bool skipSample = false;	// ADDED_24042009 flag for when all roots have ended their last trial and precision gap  <= 0


        //struct tms now;
        int policyIndex, checkIndex;//index for policy output file, and index for checking whether to output policy file
        bool stop;
        //AGc: list of index of beliefstate in tree
        std::vector<cacherow_stval> currentBeliefIndexArr; //int currentBeliefIndex;
        // modified for parallel trials
        //cacherow_stval currentBeliefIndex; //int currentBeliefIndex;
        //AGc: for all X hold a belief state in the tree (different trees??)
        currentBeliefIndexArr.resize(problem->initialBeliefX->size());
        //AGc: init all
        FOR(r,currentBeliefIndexArr.size()) {
            currentBeliefIndexArr[r].sval = -1;
            currentBeliefIndexArr[r].row = -1;
        }

        //AGc: list of row and state
        list<cacherow_stval> sampledBeliefs;  //modified for factored, prevly: list<int> sampledBeliefs;
        cacherow_stval lastRootBeliefIndex; //24092008 added to keep track of root chosen for each new trial
        lastRootBeliefIndex.row = -1;
        lastRootBeliefIndex.sval = -1;

        int numPolicies = 0; //SYLADDED 07082008 temporary

        //start timing
        //times(&start);
        runtimeTimer.start();
        cout << "\nSARSOP initializing ..." << endl;

        //AGc: init-> create upper + lower bound + 'learn' them
        initialize(problem);

        //AG120316: log
        //AG130214: moved to check location
        CPTimer t;
        t.start();


        if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices()) {
            // only POMDPX parser can generates 2 sets of matrices, therefore, only release the second set if it is using POMDPX parser and the second set is generated
            problem->deletePOMDPMatrices();
        }
        if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices()) {
            // only POMDPX parser can generates 2 sets of matrices, therefore, only release the second set if it is using POMDPX parser and the second set is generated
            problem->deletePOMDPMatrices();
        }
        if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices()) {
            // only POMDPX parser can generates 2 sets of matrices, therefore, only release the second set if it is using POMDPX parser and the second set is generated
            problem->deletePOMDPMatrices();
        }
        GlobalResource::getInstance()->getInstance()->solving = true;

        //cout << "finished calling initialize() in SARSOP::solve()" << endl;

        //initialize parameters
        stop = false;

        //AG120316: log
        _logStream<<t.elapsed()<<",";
        t.restart();

        //ADD SYLTAG - need to expand global root and all the roots for sampling
        //?=  root of trees (?)
        //AG: should be root of all beliefs that are sampled
        BeliefForest& globalroot = *(sampleEngine->getGlobalNode());

        beliefForest->globalRootPrepare();//do preparation work for global root

        // cycle through all the roots and do preparation work
        FOR(r, globalroot.sampleRootEdges.size()) {
            //		FOR(r, sampleEngine->globalRoot->sampleRootEdges.size()) {
            if (NULL != globalroot.sampleRootEdges[r]) {
                BeliefTreeNode& thisRoot = *(globalroot.sampleRootEdges[r]->sampleRoot);
                sampleEngine->samplePrepare(thisRoot.cacheIndex);//do preparation work for this root
            }
        }

        // TODO:: sampleEngine->dumpData = dumpData;//dump data
        // TODO:: sampleEngine->dumpPolicyTrace = dumpPolicyTrace;//dump datadone
        policyIndex = 0;
        checkIndex = 0;

        lapTimer.start();
        elapsed = runtimeTimer.elapsed();
        printf("  initialization time : %.2fs\n", elapsed);

        //AG120316: log
        t.pause();
        _logStream<<t.elapsed()<<","<<elapsed<<endl;

        DEBUG_LOG(logFilePrint(policyIndex-1););

        // paused timer for writing policy
        double currentElapsed = lapTimer.elapsed();
        lapTimer.pause();
        runtimeTimer.pause();

        //write out INITIAL policy

        DEBUG_LOG(writeIntermediatePolicyTraceToFile(0, 0.0, this->solverParams->outPolicyFileName, this->solverParams->problemName ); );
        DEBUG_LOG(cout << "Initial policy written" << endl;);

        printHeader();

        lapTimer.resume();
        runtimeTimer.resume();

        policyIndex++;
        elapsed += currentElapsed;

        //times(&last);//renew 'last' time flag
        lapTimer.restart();

        //AG121120: set to 0 otherwise -1 in logs
        lastSolverStats.lowBoundBackupCountSinceLast = lastSolverStats.upBoundBackupCountSinceLast = 0;
        lastSolverStats.lowBoundBackupTime = lastSolverStats.upBoundBackupTime = 0;

        string logsi = alwaysPrint(false);

        //AG120316: log
        printLogTime();
        _logStream<<"SOLVE START,-1,"<<logsi<<t.elapsed()<<endl;


        DEBUG_LOG( logFilePrint(policyIndex-1); );


        //create while loop where:
        int lastTrial = ((SampleBP*)sampleEngine)->numTrials;

        // no root assigned as active at the beginning
        int activeRoot = -1;


        DEBUG_LOG( cout << "Now staring real learning"<<endl;);

        //AG120316: log time iter
        int iter=0;

        if (!this->solverParams->doPruning) {
            cout << "Skipping pruning!"<<endl;
        }


        //AG121120: init times to 0
        lastSolverStats.pruneTime = lastSolverStats.sampleTime = 0;

        while(!stop) {
            //AG120316: log
            //printLogTime();
            //_logStream<<"SOLVE,"<<iter<<",";

            CPTimer oneLoopTimer;
            oneLoopTimer.start();

            //AG130214: to measure steps
            t.restart();

            int numTrials = ((SampleBP*)sampleEngine)->numTrials;
            if ( this->solverParams->targetTrials > 0 && numTrials > this->solverParams->targetTrials ) {
                //    target number of trials reached
                break;
            }

            //0. IF this is the start of a new trial,
            // backup the list of nodes in sampledBeliefs, then
            // decide on which root to sample from
            //	(choose the root which has the largest weighted excess uncertainty)
            //   ELSE, do a regular backup of just one node

            if (activeRoot == -1) {
                FOR (r, globalroot.getGlobalRootNumSampleroots()) {
                    SampleRootEdge* eR = globalroot.sampleRootEdges[r];

                    if (NULL != eR) {
                        BeliefTreeNode & sn = *eR->sampleRoot;
                        sampledBeliefs.clear();
                        sampledBeliefs.push_back(sn.cacheIndex);

                        DEBUG_TRACE( printSampleBelief(sampledBeliefs); );

                        currentBeliefIndexArr[r] =  backup(sampledBeliefs);
                    }
                }
                sampledBeliefs.clear();

            } else {

                if (((SampleBP *)sampleEngine)->newTrialFlagArr[activeRoot] == 1) {
                    // backup the list of nodes in sampledBeliefs
                    DEBUG_TRACE( printSampleBelief(sampledBeliefs); );
                    currentBeliefIndexArr[activeRoot] = backup(sampledBeliefs);
                    lastRootBeliefIndex = currentBeliefIndexArr[activeRoot];
                    sampledBeliefs.clear();
                    // backup at all root nodes except for the root node that we had just backedup
                    FOR (r, globalroot.getGlobalRootNumSampleroots()) {
                        SampleRootEdge* eR = globalroot.sampleRootEdges[r];

                        if (NULL != eR) {
                            BeliefTreeNode & sn = *eR->sampleRoot;
                            // check if we had just done backup at this root,
                            if( !((sn.cacheIndex.row == lastRootBeliefIndex.row) && (sn.cacheIndex.sval == lastRootBeliefIndex.sval)) ) {

                                // ADDED_24042009 - dont do LB backup if precision gap <= 0
                                // check if the precision gap for this root is already zero
                                double lbVal = beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
                                double ubVal = beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;

                                if (!((ubVal - lbVal) <= 0)) {
                                    // else, do backup at this root
                                    sampledBeliefs.clear();
                                    sampledBeliefs.push_back(sn.cacheIndex);

                                    DEBUG_TRACE( cout << "LB backup only " << endl; );
                                    DEBUG_TRACE( printSampleBelief(sampledBeliefs); );

                                    backupLBonly(sampledBeliefs);
                                    //ofsol1710d: backup(sampledBeliefs);
                                }
                            }
                        }
                    }
                    sampledBeliefs.clear();

                    ((SampleBP *)sampleEngine)->newTrialFlagArr[activeRoot]  = 0;

                } else {
                    DEBUG_TRACE( printSampleBelief(sampledBeliefs); );
                    currentBeliefIndexArr[activeRoot] = backup(sampledBeliefs);

                }

            }

            //AG130214: step timing
            lastSolverStats.step0Time = t.elapsed();
            t.restart();

            //[1.?] go to next valid activeRoot here
            if (activeRoot == -1) { // set to the first valid root
                // cycle through all roots till we find a valid one
                FOR (r, globalroot.getGlobalRootNumSampleroots()) {
                    SampleRootEdge* eR = globalroot.sampleRootEdges[r];
                    if (NULL != eR) {
                        activeRoot = r;
                        break;
                    }
                }
            } else {		// set to the next valid root
                int currActiveRoot = activeRoot; 	// ADDED_24042009
                bool passedcurrActiveRoot = false;	// ADDED_24042009
                while(true) {

                    // ADDED_24042009
                    if ((activeRoot == currActiveRoot) && passedcurrActiveRoot) {
                        // i.e. this is the second time that activeRoot == currActiveRoot, the while loop has cycled through all roots and not found one that passes the tests below
                        skipSample = true;		// flag to indicate dont call sample()
                        break;
                    }

                    if (activeRoot == currActiveRoot) passedcurrActiveRoot = true;

                    if (activeRoot == (globalroot.getGlobalRootNumSampleroots()-1))
                        activeRoot = 0;
                    else activeRoot++;

                    if (globalroot.sampleRootEdges[activeRoot] != NULL) {

                        // ADDED_24042009 - dont go to this root if this root is about to start a new trial
                        // and the precision gap for the root is already zero
                        cacherow_stval currCacheIndex = globalroot.sampleRootEdges[activeRoot]->sampleRoot->cacheIndex;
                        double lbVal = beliefCacheSet[currCacheIndex.sval]->getRow(currCacheIndex.row)->LB;
                        double ubVal = beliefCacheSet[currCacheIndex.sval]->getRow(currCacheIndex.row)->UB;
                        if (!((((SampleBP *)sampleEngine)->trialTargetPrecisionArr[activeRoot] == -1)&&((ubVal - lbVal) <= 0) )) {
                            break;
                        }
                    }
                }
            }

            //AG130214: step timing
            lastSolverStats.step1Time = t.elapsed();
            t.restart();

            //2. sample
            //  samples the next belief to do backup
            //  a. if haven't reached target depth, search further
            //  b. if target depth has been reached, go back to root

            if (!skipSample) {
                CPTimer sTimer;
                sTimer.start(); //AG121120: record sample time
                // ADDED_24042009
                sampledBeliefs = sampleEngine->sample(currentBeliefIndexArr[activeRoot], activeRoot);
                lastSolverStats.sampleTime = sTimer.elapsed(); //AG121120: sample time
            }

            //AG130214: step timing
            lastSolverStats.step2Time = t.elapsed();
            t.restart();

            //3. prune
            //	decide whether needs pruning at this moment, if so,
            //  prune off the unnecessary nodes

            //DEBUG_TRACE (beliefForest->print(););

            if (this->solverParams->doPruning) { //AG121008: allow to disable pruning (for debug)
                CPTimer pTimer;
                pTimer.start(); //AG121120: record prune time
                pruneEngine->prune();
                lastSolverStats.pruneTime = pTimer.elapsed(); //AG121120: prune time
            }

            //AG130214: step timing
            lastSolverStats.step3Time = t.elapsed();
            t.restart();

            //4. write out policy file if interval time reached
            // check time every CHECK_INTERVAL backups
            ///AG TODO: MAYBE LOWER THIS CHECK!!!!!
            if(this->solverParams->interval > 0 || this->solverParams->timeoutSeconds > 0) {
                //only do this if required
                if((numBackups/CHECK_INTERVAL) >= checkIndex) {
                    //do check every CHECK_INTERVAL(50) backups
                    //times(&now);
                    checkIndex++;//for next check

                    //check and write out policy file periodically
                    if (this->solverParams->interval > 0) {
                        double currentElapsed = lapTimer.elapsed();
                        if(currentElapsed > this->solverParams->interval) {
                            //write out policy and reset parameters

                            // paused timer for writing policy
                            lapTimer.pause();
                            runtimeTimer.pause();

                            writeIntermediatePolicyTraceToFile(numTrials, runtimeTimer.elapsed(), this->solverParams->outPolicyFileName, this->solverParams->problemName );

                            lapTimer.resume();
                            runtimeTimer.resume();

                            policyIndex++;
                            elapsed += currentElapsed;
                            cout << "Intermediate policy written(interval: "<< this->solverParams->interval <<")" << endl;

                            // reset laptime so that next interval can start
                            lapTimer.restart();


                            DEBUG_LOG(logFilePrint(policyIndex-1););
                            DEBUG_LOG( progressiveIncreasePolicyInteval(numPolicies); );


                        }
                    }//end write out policy periodically

                    else if(this->solverParams->timeoutSeconds >0) {
                        double currentElapsed = runtimeTimer.elapsed();
                        elapsed = currentElapsed;
                    }
                }//end check periodically for policy write out and elapsed time update
            }

            //AG130214: step timing
            lastSolverStats.step4Time = t.elapsed();
            t.restart();

            //5. do printing for current precision
            string logS = print();

            //AG130214: step timing
            lastSolverStats.step5Time = t.elapsed();
            t.restart();

            //6. decide whether to stop here
            stop = stopNow(activeRoot); //AG121212: to decide stop based on depth

            //AG130214: step timing
            lastSolverStats.step6Time = t.elapsed();
            t.pause();

            //AG120316: log
            oneLoopTimer.pause();
            if (!logS.empty()) {
                printLogTime();
                _logStream<<"SOLVE,"<<iter<<","<<logS<<stop<< ","<< oneLoopTimer.elapsed()<<endl;
            }

            iter++;

        }

    }

    catch(bad_alloc &e) {
        // likely bad_alloc exception
        // should we remove the last alpha vector?
        cout << "Memory limit reached, trying to write out policy" << endl;

    }

    if (this->solverParams->doPruning) { //AG121008: allow to disable pruning (for debug)
        //prune for the last time
        FOR (stateidx, lowerBoundSet->set.size()) {
            lowerBoundSet->set[stateidx]->pruneEngine->prunePlanes();
        }
    } /*else {
        cout << "Skipping pruning!"<<endl;
    }*/


    printHeader();
    alwaysPrint();
    printDivider();
    DEBUG_LOG(logFilePrint(-1););

    //now output policy to the outfile
    cout << endl << "Writing out policy ..." << endl;
    cout << "  output file : " << this->solverParams->outPolicyFileName << endl;
    writePolicy(this->solverParams->outPolicyFileName, this->solverParams->problemName);
}

//Function: print
//Functionality:
//	print the necessary info for help  understanding current situation inside
//	solver
//ag120316: return string with log string
string SARSOP::print() {
    string res;//ag
    if(numBackups/CHECK_INTERVAL>printIndex) {
        printIndex++;
        //print time now
        res= alwaysPrint();
    }
    return res;
}

//Function: print
//Functionality:
//    print the necessary info for help  understanding current situation inside
//    solver
//ag120316: return string with log string
string SARSOP::alwaysPrint(bool printStepTime) {
    //struct tms now;
    //float utime, stime;
    //long int clk_tck = sysconf(_SC_CLK_TCK);

    //print time now
    //times(&now);
    double currentTime =0;
    if(this->solverParams->interval >0) {
        currentTime = elapsed + lapTimer.elapsed();
    } else {
        currentTime = runtimeTimer.elapsed();
    }
    //printf("%.2fs ", currentTime);
    cout.precision(6);
    cout <<" ";
    cout.width(8);
    cout << left << currentTime;

    //print current trial number, num of backups
    int numTrials = ((SampleBP*)sampleEngine)->numTrials;
    //printf("#Trial %d ",numTrials);
    cout.width(7);
    cout << left  <<numTrials << " ";
    //printf("#Backup %d ", numBackups);
    cout.width(8);
    cout << left << numBackups << " ";
    //print #alpha vectors
    //print precision

    //ADD SYLTAG
    //assume we can estimate lb and ub at the global root
    //by cycling through all the roots to find their bounds
    double lb = 0, ub = 0, width = 0;

    BeliefForest& globalRoot  = *(sampleEngine->getGlobalNode());
    FOR (r, globalRoot.getGlobalRootNumSampleroots()) {
        SampleRootEdge* eR = globalRoot.sampleRootEdges[r];
        if (NULL != eR) {
            BeliefTreeNode & sn = *eR->sampleRoot;
            double lbVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
            double ubVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
            lb += eR->sampleRootProb * lbVal;
            ub += eR->sampleRootProb * ubVal;
            width += eR->sampleRootProb * (ubVal - lbVal);
        }
    }

    //REMOVE SYLTAG
    //cacherow_stval rootIndex = sampleEngine->getRootNode()->cacheIndex;
    //double lb = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->LB;
    //double ub = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->UB;

    //printf("[%f,%f],", lb, ub);
    cout.width(10);
    cout << left << lb<< " ";
    cout.width(10);
    cout << left << ub<< " ";

    //print precision
    double precision = width; // ub - lb;   //MOD SYLTAG
    //printf("%f, ", precision);
    cout.width(11);
    cout << left << precision << " ";
    int numAlphas = 0;
    FOR (setIdx, beliefCacheSet.size()) {
        numAlphas += (int)lowerBoundSet->set[setIdx]->planes.size();
    }

    //printf("#Alphas %d ", numAlphas);			//SYLTEMP FOR EXPTS
    cout.width(9);
    cout << left << numAlphas;

    //print #belief nodes
    //printf("#Beliefs %d", sampleEngine->numStatesExpanded);
    cout.width(9);
    cout << left << sampleEngine->numStatesExpanded;

    //printf("#alphas %d", (int)bounds->alphaPlanePool->planes.size());
    printf("\n");

    //ag120308: store solver stats
    lastSolverStats.elapsedTime = runtimeTimer.elapsed();
    lastSolverStats.numTrials = numTrials;
    lastSolverStats.numBackups = numBackups;
    lastSolverStats.lowBound = lb;
    lastSolverStats.upBound = ub;
    lastSolverStats.precision = width;
    lastSolverStats.numAlpha = numAlphas;
    lastSolverStats.numBeliefs = sampleEngine->numStatesExpanded;

    //AG120316: log
    stringstream logs;
    logs << lastSolverStats.elapsedTime << "," << numTrials << "," << numBackups << ","
         << lb << "," << ub << "," << width << "," << numAlphas << "," << lastSolverStats.numBeliefs << ","
         << lastSolverStats.lowBoundBackupTime<<","<<lastSolverStats.upBoundBackupTime<<","
         << lastSolverStats.lowBoundBackupCountSinceLast<<","<<lastSolverStats.upBoundBackupCountSinceLast<<","
         << lastSolverStats.sampleTime<<","<<lastSolverStats.pruneTime<<","
         << lastSolverStats.step0Time<<","<< lastSolverStats.step1Time<<","<< lastSolverStats.step2Time<<"," //AG130214: step times
         << lastSolverStats.step3Time<<","<< lastSolverStats.step4Time<<","<< lastSolverStats.step5Time<<","
         << lastSolverStats.step6Time<<",";



    //ag121113: set times to 0
    lastSolverStats.lowBoundBackupCountSinceLast = lastSolverStats.upBoundBackupCountSinceLast = 0;
    lastSolverStats.lowBoundBackupTime = lastSolverStats.upBoundBackupTime = 0;


    return logs.str();
}

//SYL ADDED FOR EXPTS
//Function: print
//Functionality:
//    print the necessary info for help  understanding current situation inside
//    solver

void SARSOP::logFilePrint(int index) {
    //struct tms now;
    //float utime, stime;
    //long int clk_tck = sysconf(_SC_CLK_TCK);

    //print time now
    //times(&now);

    FILE *fp = fopen("solve.log", "a");
    if(fp==NULL) {
        cerr << "can't open logfile\n";
        exit(1);
    }


    fprintf(fp,"%d ",index);

    //print current trial number, num of backups
    int numTrials = ((SampleBP*)sampleEngine)->numTrials;
    //int numBackups = numBackups;
    fprintf(fp,"%d ",numTrials); 			//SYLTEMP FOR EXPTS
    //printf("#Trial %d, #Backup %d ",numTrials, numBackups);

    //print #alpha vectors
    int numAlphas = 0;
    FOR (setIdx, beliefCacheSet.size()) {
        //cout << " p : " << setIdx << " : " << 	 (int)bounds->boundsSet[setIdx]->alphaPlanePool->planes.size();
        numAlphas += (int)lowerBoundSet->set[setIdx]->planes.size();
    }

    fprintf(fp, "%d ", numAlphas);			//SYLTEMP FOR EXPTS

    double currentTime =0;
    if(this->solverParams->interval >0) {
        //utime = ((float)(now.tms_utime-last.tms_utime))/clk_tck;
        //stime = ((float)(now.tms_stime-last.tms_stime))/clk_tck;
        //currentTime = elapsed+utime+stime;
        currentTime = elapsed + lapTimer.elapsed();
        fprintf(fp, "%.2f ", currentTime);			//SYLTEMP FOR EXPTS
        //printf("<%.2fs> ", currentTime);
    } else {
        //utime = ((float)(now.tms_utime-start.tms_utime))/clk_tck;
        //stime = ((float)(now.tms_stime-start.tms_stime))/clk_tck;
        //currentTime = utime+stime;
        currentTime = runtimeTimer.elapsed();
        fprintf(fp, "%.2f ", currentTime);		//SYLTEMP FOR EXPTS
        //printf("<%.2fs> ", currentTime);
    }

    fprintf(fp,"\n");

    fclose(fp);
}

//ADD SYLTAG
bool SARSOP::stopNow(int activeRoot) { //AG121212: added activeRoot to decide stop based on depth
    bool stop = false;

    double width = 0;
    BeliefForest& globalRoot  = *(sampleEngine->getGlobalNode());

    //find the weighted excess uncertainty at the global root
    //cycle through all the roots to find their bounds
    FOR (r, globalRoot.getGlobalRootNumSampleroots()) {
        SampleRootEdge* eR = globalRoot.sampleRootEdges[r];
        if (NULL != eR) {
            BeliefTreeNode & sn = *eR->sampleRoot;
            double lbVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
            double ubVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
            width += eR->sampleRootProb * (ubVal - lbVal);
        }
    }

    if(GlobalResource::getInstance()->userTerminatedG) {
        stop = true;
    }

    if ((width) < this->solverParams->targetPrecision) {
        alwaysPrint();
        printDivider();
        printf("\nSARSOP finishing ...\n");
        printf("  target precision reached\n");
        printf("  target precision  : %f\n", this->solverParams->targetPrecision);
        printf("  precision reached : %f \n", width);

        stop = true;
    }
    if (this->solverParams->timeoutSeconds > 0) {
        if (elapsed > this->solverParams->timeoutSeconds ) {
            printDivider();
            printf("\nSARSOP finishing ...\n");
            printf("  Preset timeout reached\n");
            printf("  Timeout     : %fs\n",  this->solverParams->timeoutSeconds );
            printf("  Actual Time : %fs\n", elapsed);
            stop = true;
        }
    }

    //AG121212: filter on depth .. if reached max depth-> stop (OR: DO BACKTRACKING...)
    if (activeRoot>=0 && solverParams->maxTreeDepth>0) {
        int depth =  ( (SampleBP*)sampleEngine )->getDepth(activeRoot);
        if (depth >= solverParams->maxTreeDepth) {
            stop = true;
        }
    }


    //ag120308: store solver stats
    lastSolverStats.stopped = stop;

    return stop;
}


//REMOVE SYLTAG
/*	bool SARSOP::stopNow(){
bool stop = false;
cacherow_stval rootIndex = sampleEngine->getRootNode()->cacheIndex;
//int rootIndex = sampleEngine->getRootNode()->cacheIndex;

//decide whether to stop or not depend on current root precision
double lb = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->LB;
double ub = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->UB;
#if USE_DEBUG_PRINT
printf("targetPrecision is %f, precision is %f\n", targetPrecision, ub-lb);
#endif
if(GlobalResource::getInstance()->userTerminatedG)
{
stop = true;
}
if ((ub-lb)<targetPrecision){
alwaysPrint();
printf("Target precision reached: %f (%f)\n\n", ub-lb, targetPrecision);
stop = true;
}
if (timeout > 0){
if (elapsed > timeout){
printf("Preset timeout reached %f (%fs)\n\n", elapsed, timeout);
stop = true;
}
}
return stop;
}
*/

void SARSOP::writeIntermediatePolicyTraceToFile(int trial, double time, const string& outFileName, string problemName) {
    stringstream newFileNameStream;
    string outputBasename = GlobalResource::parseBaseNameWithPath(outFileName);
    newFileNameStream << outputBasename << "_" << trial << "_" << time << ".policy";
    string newFileName = newFileNameStream.str();
    cout << "Writing policy file: " << newFileName << endl;
    writePolicy(newFileName, problemName);
}


BeliefTreeNode& SARSOP::getMaxExcessUncRoot(BeliefForest& globalroot) {

    double maxExcessUnc = -99e+20;
    int maxExcessUncRoot = -1;
    double width;
    double lbVal, ubVal;

    FOR (r, globalroot.getGlobalRootNumSampleroots()) {
        SampleRootEdge* eR = globalroot.sampleRootEdges[r];
        if (NULL != eR) {
            BeliefTreeNode & sn = *eR->sampleRoot;
            lbVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
            ubVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
            width =	eR->sampleRootProb * (ubVal - lbVal);

            if (width > maxExcessUnc) {
                maxExcessUnc = width;
                maxExcessUncRoot = r;
            }
        }

    }

    return *(globalroot.sampleRootEdges[maxExcessUncRoot]->sampleRoot);

}

void SARSOP::backup(BeliefTreeNode* node) {
    upperBoundSet->backup(node);
    lowerBoundSet->backup(node);
}

void SARSOP::initialize(SharedPointer<MOMDP> problem) {
    printIndex = 0; // reset printing counter

    int xStateNum = problem->XStates->size();
    beliefCacheSet.resize(xStateNum);
    lbDataTableSet.resize(xStateNum);
    ubDataTableSet.resize(xStateNum);

    for(States::iterator iter = problem->XStates->begin(); iter != problem->XStates->end(); iter ++ ) {
        beliefCacheSet[iter.index()] = new BeliefCache();
        lbDataTableSet[iter.index()] = new IndexedTuple<AlphaPlanePoolDataTuple>();
        ubDataTableSet[iter.index()] = new IndexedTuple<BeliefValuePairPoolDataTuple>();
    }

    //AG120316: log
    CPTimer t;
    t.start();

    //ag: only initilizes the sets
    initializeUpperBound(problem);
    upperBoundSet->setBeliefCache(beliefCacheSet);
    upperBoundSet->setDataTable(ubDataTableSet);

    //AG120316: log
    t.pause();
    _logStream<<t.elapsed()<<",";
    t.restart();

    //ag: only initilizes the sets
    initializeLowerBound(problem);
    lowerBoundSet->setBeliefCache(beliefCacheSet);
    lowerBoundSet->setDataTable(lbDataTableSet);

    //AG120316: log
    t.pause();
    _logStream<<t.elapsed()<<",";
    t.restart();

    //ag: real bound initialization
    initializeBounds(this->solverParams->targetPrecision);

    //AG120316: log
    t.pause();
    _logStream<<t.elapsed()<<",";
    t.restart();

    //cout <<" init samples"<<endl;
    initSampleEngine(problem);
    //cout<<"init prune"<<endl;

    //AG120316: log
    t.pause();
    _logStream<<t.elapsed()<<",";
    t.restart();

    pruneEngine = new SARSOPPrune(this);

    //AG120316: log
    t.pause();
    _logStream<<t.elapsed()<<",";

}
void SARSOP::initSampleEngine(SharedPointer<MOMDP> problem) {
    sampleEngine->appendOnGetNodeHandler(&SARSOP::onGetNode);
    binManagerSet = new BinManagerSet(upperBoundSet);
    ((SampleBP*)sampleEngine)->setBinManager(binManagerSet);
    ((SampleBP*)sampleEngine)->setRandomization(solverParams->randomizationBP);

}
void SARSOP::initializeUpperBound(SharedPointer<MOMDP> problem) {
    upperBoundSet = new BeliefValuePairPoolSet(upperBoundBackup);
    upperBoundSet->setProblem(problem);
    upperBoundSet->setSolver(this);
    upperBoundSet->initialize();
    upperBoundSet->appendOnBackupHandler(&SARSOP::onUpperBoundBackup);
    ((BackupBeliefValuePairMOMDP*)upperBoundBackup)->boundSet = upperBoundSet;
}
void SARSOP::initializeLowerBound(SharedPointer<MOMDP> problem) {
    lowerBoundSet = new AlphaPlanePoolSet(lowerBoundBackup);
    lowerBoundSet->setProblem(problem);
    lowerBoundSet->setSolver(this);
    lowerBoundSet->initialize();
    lowerBoundSet->appendOnBackupHandler(&SARSOP::onLowerBoundBackup);
    lowerBoundSet->appendOnBackupHandler(&SARSOPPrune::onLowerBoundBackup);
    ((BackupAlphaPlaneMOMDP* )lowerBoundBackup)->boundSet = lowerBoundSet;
}

void SARSOP::initializeBounds(double _targetPrecision) {
    //AG120423: set the target precision to a fraction of the by the user set target precision.
    //          by default is 1e-3
    double targetPrecision = _targetPrecision * solverParams->cb_initialization_precision_factor;//CB_INITIALIZATION_PRECISION_FACTOR;

    DEBUG_LOG( cout << "initialize bounds, given target precision="<<setprecision(6)<<_targetPrecision
               << ", precision factor: " << solverParams->cb_initialization_precision_factor << ", init target factor: " << targetPrecision
               <<endl<<" blb.initialize(targetPrecision) ... "<<flush;);

    //lower bound
    CPTimer heurTimer;
    heurTimer.start(); 	// for timing heuristics
    BlindLBInitializer blb(problem, lowerBoundSet);
    blb.initialize(targetPrecision);
    elapsed = heurTimer.elapsed();

    DEBUG_LOG( cout << fixed << setprecision(2) << elapsed << "s done" << endl; );

    //upper bound
    heurTimer.restart();
    //ag120420: MDP or QMDP solution
    FastInfUBInitializer fib(problem, upperBoundSet, !solverParams->FIB_UB_Init); //ag120420: tell if onlymdp or also fib
    fib.initialize(targetPrecision);
    elapsed = heurTimer.elapsed();
    DEBUG_LOG(cout << fixed << setprecision(2) << elapsed << "s fib.initialize(targetPrecision) done" << endl;);

    FOR (state_idx, problem->XStates->size()) {
        upperBoundSet->set[state_idx]->cornerPointsVersion++;	// advance the version by one so that next time get value will calculate rather than skip
    }

    numBackups = 0;
    //ag121113: set times to 0
    lastSolverStats.lowBoundBackupCountSinceLast = lastSolverStats.upBoundBackupCountSinceLast = 0;
    lastSolverStats.lowBoundBackupTime = lastSolverStats.upBoundBackupTime = 0;
}//end method initialize

cacherow_stval SARSOP::backup(list<cacherow_stval> beliefNStates) {
    //decide the order of backups
    cacherow_stval rowNState, nextRowNState;
    nextRowNState.row = -1;

    //for each belief given, we perform backup for it
    LISTFOREACH(cacherow_stval, beliefNState,  beliefNStates) {
        //get belief
        rowNState = *beliefNState;
        nextRowNState = backup(rowNState);
    }//end FOR_EACH

    // prevly:
    //for each belief given, we perform backup for it
    /* LISTFOREACH(int, belief,  beliefs) {
    //get belief
    row = *belief;
    nextRow = backup(row);
    }//end FOR_EACH */



    if(nextRowNState.row== -1) {
        printf("Error: backup list empty\n");
        cout << "In SARSOP::backup( )" << endl;
    }
    return nextRowNState;
}//end method: backup


cacherow_stval SARSOP::backupLBonly(list<cacherow_stval> beliefNStates) {
    //decide the order of backups
    cacherow_stval rowNState, nextRowNState;
    nextRowNState.row = -1;

    //for each belief given, we perform backup for it
    LISTFOREACH(cacherow_stval, beliefNState,  beliefNStates) {
        //get belief
        rowNState = *beliefNState;
        nextRowNState = backupLBonly(rowNState);
    }//end FOR_EACH


    if(nextRowNState.row== -1) {
        printf("Error: backup list empty\n");
        cout << "In SARSOP::backupLBonly( )" << endl;
    }
    return nextRowNState;
}//end method: backup

//Function: backup
//Functionality:
//	do backup at a single belief
//Parameters:
//	row: the row index of the to-be-backuped belief in BeliefCache
//Returns:
//	row index of the belief as the starting point for sampling
cacherow_stval SARSOP::backup(cacherow_stval beliefNState) {

    unsigned int stateidx = beliefNState.sval;
    int row = beliefNState.row;

    //cout << "in SARSOP::backup(), beliefNState.sval : " << beliefNState.sval << " beliefNState.row : " << beliefNState.row << endl;

    //do belief propogation if the belief is a fringe node in tree
    BeliefTreeNode* cn = beliefCacheSet[stateidx]->getRow(row)->REACHABLE;
    //should we use BeliefTreeNode or should we use row index?
    // TEMP, should move all the time stamp code to Global Resource

    //SYL220210 the very first backup should have timestamp of 1, so we increment the timestamp first
    numBackups++;

    //AG121113: measure time
    CPTimer timer;


    GlobalResource::getInstance()->setTimeStamp(numBackups);

    timer.start(); //AG121113: measure time
    lowerBoundSet->backup(cn);
    lastSolverStats.lowBoundBackupTime += timer.elapsed(); //AG121113: measure time

    timer.restart(); //AG121113: measure time
    upperBoundSet->backup(cn);
    lastSolverStats.upBoundBackupTime += timer.elapsed(); //AG121113: measure time

    //numBackups++;
    GlobalResource::getInstance()->setTimeStamp(numBackups);

    //AG121113: increase counts of backups
    lastSolverStats.lowBoundBackupCountSinceLast++;
    lastSolverStats.upBoundBackupCountSinceLast++;

    return beliefNState;
}

cacherow_stval SARSOP::backupLBonly(cacherow_stval beliefNState) {

    unsigned int stateidx = beliefNState.sval;
    int row = beliefNState.row;

    //cout << "in SARSOP::backup(), beliefNState.sval : " << beliefNState.sval << " beliefNState.row : " << beliefNState.row << endl;

    //do belief propogation if the belief is a fringe node in tree
    BeliefTreeNode* cn = beliefCacheSet[stateidx]->getRow(row)->REACHABLE;

    //AG121113: measure time
    CPTimer timer;


    //should we use BeliefTreeNode or should we use row index?
    // TEMP, should move all the time stamp code to Global Resource
    GlobalResource::getInstance()->setTimeStamp(numBackups);

    timer.start(); //AG121113: measure time
    lowerBoundSet->backup(cn);
    lastSolverStats.lowBoundBackupTime += timer.elapsed(); //AG121113: measure time

    //bounds->backupUpperBoundBVpair->backup(*cn);
    numBackups++;
    GlobalResource::getInstance()->setTimeStamp(numBackups);
    return beliefNState;
}

void SARSOP::writePolicy(string fileName, string problemName) {
    writeToFile(fileName, problemName);
}

void SARSOP::writeToFile(const std::string& outFileName, string problemName) {
    lowerBoundSet->writeToFile(outFileName, problemName);

}

void SARSOP::printHeader() {
    cout << endl;
    printDivider();
    cout << " Time   |#Trial |#Backup |LBound    |UBound    |Precision  |#Alphas |#Beliefs  " << endl;
    printDivider();
}

void SARSOP::printDivider() {
    cout << "-------------------------------------------------------------------------------" << endl;
}



void SARSOP::onGetNode(PointBasedAlgorithm *solver, BeliefTreeNode* node, SharedPointer<BeliefWithState>& belief) {
    SARSOP *sarsopSolver = (SARSOP *)solver;
    int stateidx = belief->sval;
    int row = node->cacheIndex.row;
    int timeStamp = sarsopSolver->numBackups;

    // SARSOP Bin Manager related
    sarsopSolver->binManagerSet->binManagerSet[stateidx]->binManagerDataTable.set(row).binned = false;

    // TODO: fix this bug, UB_ACTION is set by backup, but if a node is allocated and sampled before backup, UB_ACTION is not defined
    sarsopSolver->upperBoundSet->set[stateidx]->dataTable->set(row).UB_ACTION = 0;


    list<SharedPointer<AlphaPlane> >* alphas = new list<SharedPointer<AlphaPlane> >();
    sarsopSolver->lowerBoundSet->set[stateidx]->dataTable->set(row).ALPHA_PLANES= alphas;

    // TODO:: fix it
    SharedPointer<AlphaPlane>alpha = sarsopSolver->lowerBoundSet->getValueAlpha(belief);
    //REAL_VALUE lbVal = sarsopSolver->lowerBoundSet->getValue(belief);
    REAL_VALUE lbVal = inner_prod(*alpha->alpha, *belief->bvec);
    //REAL_VALUE lbVal = bounds->getLowerBoundValue(b_s, &alpha);



    SARSOPAlphaPlaneTuple *dataAttachedToAlpha = (SARSOPAlphaPlaneTuple *)(alpha->solverData);
    //TODO: dataAttachedToAlpha->maxMeta



    REAL_VALUE ubVal =sarsopSolver->upperBoundSet->getValue(belief);
    // TODO:: node->lastUBVal = ubVal;

    solver->beliefCacheSet[stateidx]->getRow( row)->REACHABLE = node;
    solver->beliefCacheSet[stateidx]->getRow( row)->UB = ubVal;
    solver->beliefCacheSet[stateidx]->getRow( row)->LB = lbVal;

    sarsopSolver->lowerBoundSet->set[stateidx]->dataTable->set(row).ALPHA_TIME_STAMP = timeStamp;


    if(timeStamp!=-1) {
        //assert(solver->beliefCacheSet[stateidx]->getRow( row)->isFringe );
        DEBUG_TRACE("getNode timeStamp!=-1");
        if(!hasMaxMetaAt(alpha, node->cacheIndex.row)) {	// assume that the alpha is from the correct boundsSet[]
            DEBUG_TRACE("!hasMaxMetaAt");
            AlphaPlaneMaxMeta* newMax = new AlphaPlaneMaxMeta();
            newMax->cacheIndex = node->cacheIndex.row;
            newMax->lastLB = lbVal;
            newMax->timestamp = GlobalResource::getInstance()->getTimeStamp();
            dataAttachedToAlpha->maxMeta.push_back(newMax);
        }
    }


}


bool SARSOP::hasMaxMetaAt(SharedPointer<AlphaPlane>alpha, int index) {
   SARSOPAlphaPlaneTuple *attachedData = (SARSOPAlphaPlaneTuple *)alpha->solverData;
   FOREACH(AlphaPlaneMaxMeta* , entry, attachedData->maxMeta) {
       if((*entry)->cacheIndex == index) {
           return true;
       }
   }
   return false;
}
