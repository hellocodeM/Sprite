#ifndef LIST_H
#define LIST_H

#include "new.hpp"
#include "types.h"

/**
 * Linked list.
 */
template <class T>
class List {
    struct ListNode {
        T data;
        ListNode* next;
        ListNode* prev;
    };

    struct iterator {
        ListNode* node;

        iterator(ListNode* n) : node(n) {}

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

        T& operator*() { return node->data; }

        T* operator->() { return &node->data; }

        bool operator==(iterator rhs) const { return node == rhs.node; }

        bool operator!=(iterator rhs) const { return !(*this == rhs); }
    };
public:
    using iterator = struct iterator;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using size_t = uint32_t;

    /* constructor, destructor, operator = */
    List(): size_(0), head_(0) {}

    ~List() {
        //todo
    }

    List(const List& other) {

    }

    List(List&& other) {

    }

    List& operator= (const List& rhs) {

    }

    List& operator= (List&& rhs) {

    }

    void assign(size_t count, const T& value) {

    }

    template <class InputIterator>
    void assign(InputIterator first, InputIterator last) {

    }

    /* element access */
    reference front() {

    }

    reference back() {

    }

    /* iterators */
    iterator begin() {

    }

    iterator rbegin() {

    }

    iterator end() {

    }

    iterator rend() {

    }

    /* capacity */
    bool empty() const {

    }

    size_t size() const {

    }

    /* modifier */
    void clear() {

    }

    iterator insert(iterator pos, const reference value) {

    }

    iterator insert(iterator pos, rvalue_ref value) {

    }

    template <class InputIterator>
    void insert(iterator pos, InputIterator first, InputIterator last) {

    }

    iterator erase(iterator pos) {

    }

    iterator erase(iterator first, iterator last) {

    }

    void push_back(const reference value) {

    }

    void pop_back() {

    }

    void push_front(const reference value) {

    }

    void pop_front() {

    }

    void swap(List& other) {

    }

    /* operations */

};

#endif
