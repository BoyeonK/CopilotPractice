#pragma once

#include <vector>
#include <stack>
#include <memory>

template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initial = 0) { reserve(initial); }
    ~ObjectPool() = default;

    void reserve(size_t n) {
        for (size_t i = 0; i < n; ++i) {
            pool_.push_back(std::make_unique<T>());
            free_.push(pool_.back().get());
        }
    }

    template<typename... Args>
    T* acquire(Args&&... args) {
        if (free_.empty()) {
            pool_.push_back(std::make_unique<T>());
            T* ptr = pool_.back().get();
            *ptr = T(std::forward<Args>(args)...);
            return ptr;
        } else {
            T* ptr = free_.top();
            free_.pop();
            *ptr = T(std::forward<Args>(args)...);
            return ptr;
        }
    }

    void release(T* obj) {
        free_.push(obj);
    }

    size_t capacity() const { return pool_.size(); }
    size_t available() const { return free_.size(); }

private:
    std::vector<std::unique_ptr<T>> pool_;
    std::stack<T*> free_;
};
