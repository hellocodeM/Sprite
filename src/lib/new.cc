#include "new.h"
#include "kmalloc.h"
#include "types.h"

void* operator new(uint32_t count) { return kmalloc(count); }

void* operator new[](uint32_t count) { return kmalloc(count); }

void* operator new(uint32_t count, void* ptr) { return ptr; }

void* operator new[](uint32_t count, void* ptr) { return ptr; }

void operator delete(void* ptr) noexcept { kfree(ptr); }

void operator delete[](void* ptr) noexcept { kfree(ptr); }



