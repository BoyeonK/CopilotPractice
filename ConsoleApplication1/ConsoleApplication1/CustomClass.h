#pragma once

#include <string>

class CustomClass {
public:
    std::string text;
    int value;

    CustomClass() = default;
    CustomClass(const std::string& t, int v) : text(t), value(v) {}
};
