#ifndef ATOMIC_H
#define ATOMIC_H


class atomic_t {
public:
    inline void add(int i) {
        asm volatile (
                "lock;"
                "add %1, %0;"
                : "+m"(counter)
                : "ir"(i));
    }

    inline void reduce(int i) {
        asm volatile (
                "lock;"
                "dec %1, %0;"
                : "+m"(counter)
                : "ir"(i));
    }

    inline void set(int i) {
        asm volatile (
                "lock;"
                "mov %1, %0;"
                : "+m"(counter)
                : "ir"(i));
    }

private:
    int counter = 0;
};

#endif
