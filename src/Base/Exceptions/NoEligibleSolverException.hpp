//
// Created by Filippo Vicentini on 22/06/2018.
//

#ifndef SIMULATOR_NOELIGIBLESOLVER_HPP
#define SIMULATOR_NOELIGIBLESOLVER_HPP

#include <stdexcept>
#include <string>

class NoEligibleSolver : public std::runtime_error {
public:
    NoEligibleSolver()
            : std::runtime_error("The settings provided don't match any eligible solver."){};
};
#endif //SIMULATOR_NOELIGIBLESOLVER_HPP
