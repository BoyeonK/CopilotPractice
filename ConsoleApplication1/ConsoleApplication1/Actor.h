#pragma once

#include "MPSCQueue.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

// Actor base: single consumer processing messages of type T posted by multiple producers.
template<typename T>
class Actor {
public:
    Actor() : running_(false) {}
    virtual ~Actor() { stop(); }

    // start the consumer thread
    void start() {
        bool expected = false;
        if (!running_.exchange(true)) {
            worker_ = std::thread([this]{ run(); });
        }
    }

    // stop the consumer thread and wait for it to finish processing queued items
    void stop() {
        if (!running_.exchange(false)) return;
        cv_.notify_one();
        if (worker_.joinable()) worker_.join();
    }

    // post by copy
    void post(const T& msg) {
        queue_.push(msg);
        cv_.notify_one();
    }

    // post by move
    void post(T&& msg) {
        queue_.emplace(std::move(msg));
        cv_.notify_one();
    }

protected:
    // override to handle a message (by value)
    virtual void onMessage(T msg) = 0;

private:
    void run() {
        while (running_.load() || !queue_.empty()) {
            T item;
            {
                std::unique_lock<std::mutex> lk(mutex_);
                cv_.wait(lk, [this]{ return !queue_.empty() || !running_.load(); });
            }
            while (queue_.pop(item)) {
                onMessage(std::move(item));
            }
        }
    }

    MPSCQueue<T> queue_;
    std::atomic<bool> running_;
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
