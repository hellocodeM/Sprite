#ifndef NEW_HPP
#define NEW_HPP

#include "types.h"

void* operator new(uint32_t count); 

void* operator new[](uint32_t count);

void* operator new(uint32_t count, void* ptr); 

void* operator new[](uint32_t count, void* ptr); 

void operator delete(void* ptr) noexcept ; 

void operator delete[](void* ptr) noexcept; 

#endif
