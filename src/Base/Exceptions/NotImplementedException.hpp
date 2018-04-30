//
// Created by Filippo Vicentini on 30/04/2018.
//

#ifndef SIMULATOR_NOTIMPLEMENTEDEXCEPTION_HPP
#define SIMULATOR_NOTIMPLEMENTEDEXCEPTION_HPP

#include <stdexcept>

class NotImplemented : public std::logic_error
{
public:
    NotImplemented() : std::logic_error("Function not yet implemented.") { };
};
#endif //SIMULATOR_NOTIMPLEMENTEDEXCEPTION_HPP
