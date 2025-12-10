#include "MObs.h"
using namespace std;

MObs::MObs(vector<long> obs): obs(obs)
{
    hashCode = 0;
    // computeHash();
}

MObs::MObs(MObs const& other): obs(other.obs)
{
    hashCode = other.hashCode;
    // computeHash();
}

MObs::MObs()
{
    hashCode = 0;
}

bool MObs::operator==(const MObs& obs) const
{
    if (obs.hashCode != this->hashCode) return false;

    for (long i = 0; i < (long)this->obs.size(); i++)
        if (this->obs[i] != obs.obs[i]) return false;

    return true;
}

bool MObs::operator<(const MObs& obs) const
{
    return (this->compare(obs) == -1);
}

int MObs::compare(const MObs& obs) const
{
    for (long i = 0; i < (long)this->obs.size(); i++) {
        if (this->obs[i] > obs.obs[i]) return 1;
        if (this->obs[i] < obs.obs[i]) return -1;
    }
    return 0;
}

void MObs::computeHash()
{
    hashCode = 0;
    for (int i=0; i < (long)obs.size(); i++)
        hashCode = (hashCode*largePrime + obs[i]) % modulo;
}
