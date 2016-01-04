#include <cassert>

#include "bitmap.hpp"
#include "lrucache.hpp"

void test_cache() {
    LRUCache<int, 3> cache;
    cache.Set(0, 0);
    cache.Set(1, 1);
    assert(*cache.Get(0) == 0);
    cache.Set(2, 2);
    assert(*cache.Get(1) == 1);
    assert(*cache.Get(0) == 0);
    assert(*cache.Get(2) == 2);
    
    cache.Set(4, 4);
    assert(cache.Get(1) == nullptr);

    for (int i = 0; i < 1024; i++)
        cache.Set(i, i);
}

void test_bitmap() {
    bitmap<32> bm;
    assert(bm.none());

    // operator[]
    bm[0] = true;
    assert(bm.test(0));
    
    assert(bm.find_first(true) == 0);
    bm.clear();
    bm.flip();
    bm.set(1, false);
    assert(bm.find_first(false) == 1);

}

int main() {
    test_bitmap();
    test_cache();
}
