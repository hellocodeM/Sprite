#ifndef LIST_H
#define LIST_H

/**
 * Linked list.
 */
template <class T, class Allocator>
class List {
    struct ListNode {
        T data;
        ListNode* next;
        ListNode* prev;
    };

public:
    struct iterator {
        ListNode* node;

        iterator(ListNode* n):node(n) {
        }

        iterator operator++() {
            node = node->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(node);
            node = node->next;
            return tmp;
        }

        iterator operator--() {
            node = node->prev;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp(node);
            node = node->prev;
            return tmp;
        }

        T& operator*() {
            return node->data;
        }

        T* operator->() {
            return &node->data;
        }

        bool operator==(iterator rhs) const {
            return node == rhs.node;
        }

        bool operator!=(iterator rhs) const {
            return !(*this == rhs);
        }
    };
    /* property */
    uint32_t size() const {
        return size_;
    }

    /* modify */
    void push_front(const T& d) {
        ListNode* node = new ListNode(d);
        node->next = head_;
        head_ = node;
    }

    void pop_front() {
        assert(head_);
        ListNode* tmp = head_;
        head_ = head_->next;
        delete tmp;
    }

    T& front() {
        return head_->data;
    }

    iterator insert_after(iterator iter, const T& data) {
        ListNode* node = new ListNode(data);
        ListNode* next = iter.node.next;
        iter.node.next = node;
        node->next = next;
        return iterator(node);
    }

    void inter_before(iterator iter, const T& data) {
        ListNode* node = new ListNode(data);
        ListNode* prev = iter.node.prev;
        prev->next = node;
        node = &iter.node;
    }

    void erase(iterator iter) {
        ListNode* next = iter.node;
        assert(next);
        
    }

    /* visit */
    iterator begin() {
        return iterator(head_);
    }

    iterator end() {
        return iterator{};
    }
private:
    uint32_t size_;
    ListNode* head_;
};

#endif
