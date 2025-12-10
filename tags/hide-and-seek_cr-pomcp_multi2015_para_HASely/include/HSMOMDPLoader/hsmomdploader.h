#ifndef HSMOMDPLOADER_H
#define HSMOMDPLOADER_H


#include "hsconfig.h"

#include "MOMDP.h"
#include "solverUtils.h"


/*!
  */
class HSMOMDPLoader
{
public:
    HSMOMDPLoader();

    SharedPointer<MOMDP> loadMOMDPFromFile(char* file);


private:
    SharedPointer<MOMDP> _momdp;
};

#endif // HSMOMDPLOADER_H
