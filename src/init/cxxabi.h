#ifndef CXXABI_H
#define CXXABI_H

extern "C" {
struct atexit_func_entry_t {
    void (*destructor_func)(void*);
    void *obj_ptr;
    void *dso_handle;
};

int __cxa_atexit(void (*f)(void*), void *obj, void* dso);
void __cxa_finalize(void *f);
}
#endif
