#ifndef __OBS_H
#define __OBS_H

#include <vector>

class MObs
{
  public:
    explicit MObs(std::vector<long> obs);
    MObs(MObs const& other);
    MObs();
    virtual ~MObs() {}

    bool operator==(const MObs& obs) const;
    bool operator<(const MObs& obs) const;

    int compare(const MObs& obs) const;

    void computeHash();

    std::vector<long> obs;
    unsigned long hashCode;

    // Should we just use public, like hashCode is public
  private:
    // static member of type integral (int, char, byte) can be
    // initialized in the declaration
    const static long largePrime = 105907;
    const static unsigned long modulo = 179426549;
};

#endif
