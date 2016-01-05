#ifndef SPRITE_BITMAP_HPP
#define SPRITE_BITMAP_HPP

#include "types.h"

/**
 * A bitmap, whose size must be multiple of 32
 */
template <size_t N>
class bitmap {
public:
    using bit_store = uint8_t;

    class bit_ref {
        bit_ref(bitmap<N>* bm, size_t pos) : bits_(bm), pos_(pos) {}
        friend bitmap;

    public:
        bit_ref& operator=(bool x) {
            bits_->set(pos_, x);
            return *this;
        }

        operator bool() const { return bits_->test(pos_); }

    private:
        bitmap<N>* bits_;
        size_t pos_;
    };

    bitmap() {
        for (auto& x : data_) x = 0;
    }

    bitmap(const bitmap& rhs) = default;

    bitmap(void* bits, size_t len) { assign(bits, len); }

    ~bitmap() = default;

    void assign(void* bits, size_t len) { memcpy(data_, bits, len); }

    bit_ref operator[](size_t idx) { return bit_ref(this, idx); }

    bool all() const {
        for (auto x : data_)
            if (x != 0xFF) return false;
        return true;
    }

    bool any() const {
        for (auto x : data_)
            if (x) return true;
        return false;
    }

    bool none() const { return !all(); }

    bool count() const {
        int mask = 0x1;
        int cnt = 0;

        for (auto x : data_) {
            while (x) {
                if (x & mask) ++cnt;
                x >>= 1;
            }
        }
        return cnt;
    }

    size_t size() const { return N; }

    bitmap<N>& set(size_t pos, bool x = true) {
        int i = pos / sizeof(bit_store);
        auto& target = data_[i];
        int j = pos % sizeof(bit_store);

        bit_store mask = (1 << (sizeof(bit_store) * 8 - 1 - j));
        if (x) {
            target |= mask;
        } else {
            mask = ~mask;
            target &= mask;
        }
        return *this;
    }

    bitmap<N>& clear() {
        for (auto& x : data_) x = 0;
        return *this;
    }

    bool test(size_t pos) const {
        int i = pos / sizeof(bit_store);
        auto& target = data_[i];
        int j = pos % sizeof(bit_store);

        bit_store mask = (1 << (sizeof(bit_store) * 8 - 1 - j));
        return mask & target;
    }

    bitmap<N>& flip() {
        for (auto& x : data_) x = ~x;
        return *this;
    }

    bitmap<N>& flip(size_t pos) {
        if (test(pos))
            set(pos, false);
        else
            set(pos, true);
        return *this;
    }

    size_t find_first(bool value) const {
        for (int j = 0; j < length_; j++) {
            auto x = data_[j];
            bit_store mask = 0x80;
            int i = 0;

            while (mask) {
                if (!!(x & mask) == value) return j * sizeof(bit_store) + i;
                mask >>= 1;
            }
        }
        return size();
    }

    bitmap<N>& operator&=(const bitmap<N>& other) {
        for (int i = 0; i < length_; i++) {
            data_[i] &= other.data_[i];
        }
        return *this;
    }

    friend void init_fs();
    friend void sync_imap();
    friend void sync_zmap();
private:
    const static int length_ = (N - 1) / 8 / sizeof(bit_store) + 1;
    bit_store data_[length_];
};

#endif
