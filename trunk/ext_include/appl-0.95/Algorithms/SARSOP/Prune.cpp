#include "Prune.h"
namespace momdp
{
Prune::Prune( PointBasedAlgorithm* _solver)
{
    solver = _solver;
    problem = solver->problem;
}
}
