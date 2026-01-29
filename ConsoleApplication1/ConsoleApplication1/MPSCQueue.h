#pragma once

#include <atomic>
#include <memory>
#include <utility>

// Simple MPSC queue: multiple producers, single consumer.
template<typename T>
class MPSCQueue {
    struct Node {
        std::atomic<Node*> next;
        std::unique_ptr<T> value;

        Node() : next(nullptr), value(nullptr) {}

        template<typename... Args>
        explicit Node(Args&&... args)
            : next(nullptr), value(std::make_unique<T>(std::forward<Args>(args)...)) {}
    };

public:
    MPSCQueue() {
        Node* dummy = new Node();
        head_ = dummy;
        tail_.store(dummy, std::memory_order_relaxed);
    }

    ~MPSCQueue() {
        Node* node = head_;
        while (node) {
            Node* next = node->next.load(std::memory_order_relaxed);
            delete node;
            node = next;
        }
    }

    // push by copy
    void push(const T& v) {
        Node* node = new Node(v);
        node->next.store(nullptr, std::memory_order_relaxed);
        Node* prev = tail_.exchange(node, std::memory_order_acq_rel);
        prev->next.store(node, std::memory_order_release);
    }

    // push by move
    void push(T&& v) {
        Node* node = new Node(std::move(v));
        node->next.store(nullptr, std::memory_order_relaxed);
        Node* prev = tail_.exchange(node, std::memory_order_acq_rel);
        prev->next.store(node, std::memory_order_release);
    }

    // construct in place
    template<typename... Args>
    void emplace(Args&&... args) {
        Node* node = new Node(std::forward<Args>(args)...);
        node->next.store(nullptr, std::memory_order_relaxed);
        Node* prev = tail_.exchange(node, std::memory_order_acq_rel);
        prev->next.store(node, std::memory_order_release);
    }

    // try to pop into out; returns false if queue empty
    bool pop(T& out) {
        Node* next = head_->next.load(std::memory_order_acquire);
        if (!next) return false;
        out = std::move(*next->value);
        Node* old = head_;
        head_ = next;
        delete old;
        return true;
    }

    bool empty() const {
        Node* next = head_->next.load(std::memory_order_acquire);
        return next == nullptr;
    }

private:
    Node* head_; // consumer-owned dummy node
    std::atomic<Node*> tail_; // producers update
};
