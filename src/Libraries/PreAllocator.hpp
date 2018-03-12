//
// Created by Filippo Vicentini on 08/03/2018.
//

#ifndef SIMULATOR_PREALLOCATOR_HPP
#define SIMULATOR_PREALLOCATOR_HPP

#include <cstdint>
#include <iterator>
#include <vector>
#include <iostream>

template <typename T>
class PreAllocator
{
private:
    T* memory_ptr;
    std::size_t memory_size;

public:
    typedef std::size_t     size_type;
    typedef T*              pointer;
    typedef T               value_type;

    PreAllocator(T* memory_ptr, std::size_t memory_size) : memory_ptr(memory_ptr), memory_size(memory_size) {}

    PreAllocator(const PreAllocator& other) throw() : memory_ptr(other.memory_ptr), memory_size(other.memory_size) {};

    template<typename U>
    PreAllocator(const PreAllocator<U>& other) throw() : memory_ptr(other.memory_ptr), memory_size(other.memory_size) {};

    template<typename U>
    PreAllocator& operator = (const PreAllocator<U>& other) { return *this; }
    PreAllocator<T>& operator = (const PreAllocator& other) { return *this; }
    ~PreAllocator() {}


    pointer allocate(size_type n, const void* hint = 0) {return memory_ptr;}
    void deallocate(T* ptr, size_type n) {}

    size_type max_size() const {return memory_size;}
};

#endif //SIMULATOR_PREALLOCATOR_HPP
