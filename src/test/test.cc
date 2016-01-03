#include <cassert>

#include "bitmap.hpp"

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
}
