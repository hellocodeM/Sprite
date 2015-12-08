#ifndef ILALLOCATOR_HPP
#define ILALLOCATOR_HPP

/**
 * implicit linked list allocator
 * allocate memory in form of blocks, which is aligned to 2.
 * the first four byte of each block, tells whether this block is allocated, and its size.
 * since the block is aligned to 2, the lowest bit is useless and could be used as 'allocated' bit.
 */
class ImplicitListAllocator {
public:
    ImplicitListAllocator(uint32_t start, uint32_t end) {
        mstart_ = reinterpret_cast<void*>(start);
        mend_ = reinterpret_cast<void*>(end);
        record(mstart_, end - start - 4, kFree);
    }

    /**
     * set the heap start and end.
     * Since in kernel, global variable will not be initialized, 
     * we need to set the boundry of heap manually.
     */
    void set_bound(uint32_t start, uint32_t end) {
        mstart_ = reinterpret_cast<void*>(start);
        mend_ = reinterpret_cast<void*>(end);
        record(mstart_, end - start - 4, kFree);
    }

    /**
     * Allocate memory
     */
    void* alloc(uint32_t size) {
        // aligned 2
        if (size & 0b1) size += 1;

        void* cur = mstart_;
        for (; cur != mend_; cur = next_block(cur)) {
            uint32_t block_size = size_of_block(cur);
            if (block_size >= size && !is_allocated(cur)) {
                void* remain = slice_block(cur, size);
                uint32_t remain_size = block_size - 4 - size;  // 4 bytes is for header
                if (remain_size > 0) {
                    record(cur, size, kAllocated);
                    record(remain, remain_size, kFree);
                } else {
                    record(cur, block_size, kAllocated);
                }
                return reinterpret_cast<uint8_t*>(cur) + 4;
            }
        }
        return nullptr;
    }

    /**
     * Free memory
     */
    void free(void* pos) {
        uint32_t* block = reinterpret_cast<uint32_t*>(pos);
        block--;
        if (!is_allocated(next_block(block))) {
            uint32_t new_size = size_of_block(block) + 4 + size_of_block(next_block(block));
            record(block, new_size, kFree);
        } else {
            record(block, size_of_block(block), kFree);
        }
    }

private:
    void* slice_block(void* block, uint32_t size) const {
        void* remain = reinterpret_cast<uint8_t*>(block) + size + 4;
        return remain;
    }

    void record(void* block, uint32_t size, uint8_t allocated) {
        *reinterpret_cast<uint32_t*>(block) = size | (allocated & 0b1);
    }

    uint32_t size_of_block(void* block) const {
        return *reinterpret_cast<uint32_t*>(block) & 0xFFFFFFFE;
    }

    bool is_allocated(void* block) const { return *reinterpret_cast<uint32_t*>(block) & 0b1; }

    void* next_block(void* block) const {
        uint32_t p_value = reinterpret_cast<uint32_t>(block);
        p_value += size_of_block(block) + 4;
        return reinterpret_cast<void*>(p_value);
    }

    /* data members */
    enum AllocState { kFree = 0, kAllocated = 1 };
    void *mstart_, *mend_;
};

#endif
