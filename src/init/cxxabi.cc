#include "cxxabi.h"

extern "C" {
atexit_func_entry_t __atexit_funcs[128];
int __atexit_func_count = 0;
void* __dso_handle = 0;

int __cxa_atexit(void (*f)(void*), void* obj, void* dso) {
    __atexit_funcs[__atexit_func_count].destructor_func = f;
    __atexit_funcs[__atexit_func_count].obj_ptr = obj;
    __atexit_funcs[__atexit_func_count].dso_handle = dso;
    ++__atexit_func_count;
    return 0;
}

void __cxa_finalize(void* f) {
    int i = __atexit_func_count;
    if (!f) {
        while (i--) {
            if (__atexit_funcs[i].destructor_func) {
                (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            }
        }
        return;
    }
    for (; i >= 0; i--) {
        if (__atexit_funcs[i].destructor_func == f) {
            (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            __atexit_funcs[i].destructor_func = 0;
        }
    }
}
}
