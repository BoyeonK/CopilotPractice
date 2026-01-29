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
        // optionally return to pool by constructing a new object and releasing
        // Here we don't have the original pointer, so skipping pool.release.
    }

private:
    ObjectPool<CustomClass>& pool_;
};
