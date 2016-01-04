#ifndef SPRITE_LRUCACHE_HPP
#define SPRITE_LRUCACHE_HPP

#include "types.h"
#include "new.h"

/**
 * LRU cache. key is size_t, value is T.
 */
template <class T, size_t N>
class LRUCache {
private:
    struct ListNode {
        size_t key;
        ListNode* prev = nullptr;
        ListNode* next = nullptr;
        T value;

        ListNode() {}
    };

public:
    LRUCache() = default;

    ~LRUCache() { clear(); }

    void clear() {
        for (auto& x : table_) x = nullptr;
        if (queue_) {
            auto iter = queue_;
            do {
                auto tmp = iter->next;
                delete iter;
                iter = tmp;
            } while (iter != queue_);
            queue_ = nullptr;
        }
        while (free_) {
            auto tmp = free_->next;
            delete free_;
            free_ = tmp;
        }
    }

    size_t size() const { return N; }

    /**
     * Number of cache.
     */
    size_t count() const { return count_; }

    /* modifier */

    /**
     * cache a value.
     */
    void Set(size_t k, const T& v) { Set(k, &v); }

    T* Alloc(size_t k) { return Set(k, nullptr); }
    /**
     * Get the cache.
     * If miss, return nullptr
     */
    T* Get(size_t k) {
        size_t hash_value = hash(k);
        int cnt = 0;
        while (++cnt <= size() && table_[hash_value] && table_[hash_value]->key != k) {
            hash_value = rehash(hash_value);
        }
        if (!table_[hash_value] || cnt > size()) return nullptr;
        Promote(hash_value);
        return &table_[hash_value]->value;
    }

private:
    T* Set(size_t k, T* v) {
        if (count() >= size()) {
            auto tmp = front();
            pop_front();
            erase(tmp->key);
        }

        size_t hash_value = hash(k);

        while (table_[hash_value]) {
            hash_value = rehash(hash_value);
        }

        auto& entry = table_[hash_value];
        if (!entry) {
            entry = alloc();
            push_back(entry);
        }
        entry->key = k;
        if (v) {
            memcpy(&entry->value, v, sizeof(T));
        }
        Promote(hash_value);
        return &entry->value;
    }
    /**
     * move a list-node to the head of queue
     */
    void Promote(size_t k) {
        auto node = table_[k];
        detach(node);
        if (node == queue_) queue_ = queue_->next;
        push_back(node);
    }

    /**
     * hash table operations
     */
    size_t hash(size_t value) const { return value % size(); }

    size_t rehash(size_t value) const { return hash(++value); }

    void erase(size_t k) {
        size_t hash_value = hash(k);
        while (table_[hash_value]->key != k) {
            hash_value = rehash(hash_value);
        }
        free(table_[hash_value]);
        table_[hash_value] = nullptr;
    }

    /**
     * Allocator for the ListNode.
     * Use a list as a object pool, to avoid frequently allocation.
     */
    ListNode* alloc() {
        if (free_) {
            auto tmp = free_;
            free_ = free_->next;
            return tmp;
        } else {
            return new ListNode();
        }
    }

    void free(ListNode* node) {
        node->next = free_;
        free_ = node;
    }

    /**
     * queue operations
     */
    void detach(ListNode* x) {
        --count_;
        x->prev->next = x->next;
        x->next->prev = x->prev;
    }

    void push_back(ListNode* x) {
        assert(count_ < size(), "full list");
        ++count_;
        if (queue_) {
            auto last = queue_->prev;
            last->next = x;
            x->prev = last;
            x->next = queue_;
            queue_->prev = x;
        } else {
            queue_ = x;
            queue_->prev = queue_->next = x;
        }
    }

    void pop_front() {
        --count_;
        assert(queue_, "empty list");
        auto last = queue_->prev;
        auto successor = queue_->next;
        successor->prev = last;
        last->next = successor;
        queue_ = successor;
    }

    ListNode* front() {
        assert(queue_, "empty list");
        return queue_;
    }

    /* data members */
    ListNode* queue_ = nullptr;       // LRU queue
    ListNode* free_ = nullptr;        // ListNode pool
    ListNode* table_[N] = {nullptr};  // hash table
    size_t count_ = 0;
};

#endif
