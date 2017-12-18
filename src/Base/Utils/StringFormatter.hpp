//
// Created by Filippo Vicentini on 18/12/2017.
//

#ifndef SIMULATOR_STRINGFORMATTER_HPP
#define SIMULATOR_STRINGFORMATTER_HPP

#include <iostream>
#include <memory>
using namespace std;

template<typename ... Args>
string string_format(const string& format, Args ... args){
    size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
    unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return string(buf.get(), buf.get() + size);
}

#endif //SIMULATOR_STRINGFORMATTER_HPP
