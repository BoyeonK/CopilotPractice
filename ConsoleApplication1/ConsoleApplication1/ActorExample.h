#pragma once

#include "Actor.h"
#include "CustomClass.h"
#include "ObjectPool.h"
#include <iostream>

// Example actor that handles CustomClass messages.
class CustomActor : public Actor<CustomClass> {
public:
    CustomActor(ObjectPool<CustomClass>& pool) : pool_(pool) {}

protected:
    void onMessage(CustomClass msg) override {
        // process message (print)
        std::cout << "Actor received: " << msg.text << " -> " << msg.value << std::endl;
        // Example: when processed, return a new object to pool (simulate reuse)
        CustomClass* p = pool_.acquire(msg.text + "-ret", msg.value);
        pool_.release(p);
    }

private:
    ObjectPool<CustomClass>& pool_;
};
