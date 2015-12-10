#ifndef NEW_HPP
#define NEW_HPP

#include "kmalloc.h"

inline void* operator new(uint32_t count) { return kmalloc(count); }

inline void* operator new[](uint32_t count) { return kmalloc(count); }

inline void* operator new(uint32_t count, void* ptr) { return ptr; }

inline void* operator new[](uint32_t count, void* ptr) { return ptr; }

inline void operator delete(void* ptr) { kfree(ptr); }

inline void operator delete[](void* ptr) { kfree(ptr); }

#endif
