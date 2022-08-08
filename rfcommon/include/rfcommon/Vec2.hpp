#pragma once

#include "rfcommon/config.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API Vec2
{
    Vec2(float x, float y);
public:
    typedef float ComponentType;

    static Vec2 fromValues(ComponentType x, ComponentType y);
    static Vec2 makeZero();

    ~Vec2();

    ComponentType x() const { return x_; }
    ComponentType y() const { return y_; }

    bool operator==(const Vec2& other) const { return x_ == other.x_ && y_ == other.y_; }
    bool operator!=(const Vec2& other) const { return x_ != other.x_ && y_ != other.y_; }

private:
    ComponentType x_, y_;
};

}
