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
        T* value;
        ListNode* prev;
        ListNode* next;
    };

public:
    LRUCache() {}

    ~LRUCache() {
        if (queue_) {
            auto iter = queue_;
            do {
                delete iter->value;
                iter = iter->next;
            } while (iter != queue_);
        }
    }

    void clear() {
        for (auto& x : table_) x = nullptr;
        if (queue_) {
            auto iter = queue_;
            do {
                delete iter->value;
                iter = iter->next;
            } while (iter != queue_);
        }
    }

    size_t size() const { return N; }

    size_t count() const { return count_; }

    /* modifier */
    void Set(size_t k, T* v) {
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
            entry = new ListNode();
            push_back(entry);
        }
        entry->key = k;
        entry->value = v;
        Promote(hash_value);
    }

    T* Get(size_t k) {
        size_t hash_value = hash(k);
        int cnt = 0;
        while (++cnt <= size() && table_[hash_value] && table_[hash_value]->key != k) {
            hash_value = rehash(hash_value);
        }
        if (!table_[hash_value] || cnt > size()) return nullptr;
        Promote(hash_value);
        return table_[hash_value]->value;
    }

private:
    size_t hash(size_t value) const { return value % size(); }

    size_t rehash(size_t value) const { return hash(++value); }

    void erase(size_t k) {
        size_t hash_value = hash(k);
        while (table_[hash_value]->key != hash_value) {
            hash_value = rehash(hash_value);
        }
        table_[hash_value] = nullptr;
    }

    /* queue operations */
    void detach(ListNode* x) {
        --count_;
        x->prev->next = x->next;
        x->next->prev = x->prev;
    }

    void push_back(ListNode* x) {
        assert(count_ < size());
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
        assert(queue_ && "empty list");
        auto last = queue_->prev;
        auto successor = queue_->next;
        successor->prev = last;
        last->next = successor;
        queue_ = successor;
    }

    ListNode* front() {
        assert(queue_ && "empty list");
        return queue_;
    }

    void Promote(size_t k) {
        auto node = table_[k];
        detach(node);
        if (node == queue_)
            queue_ = queue_->next;
        push_back(node);
    }

    /* data members */
    ListNode* queue_ = nullptr;
    ListNode* table_[N] = {nullptr};
    size_t count_ = 0;
};

#endif
