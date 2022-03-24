#pragma once

#include "rfcommon/String.hpp"
#include "rfcommon/Types.hpp"

namespace rfcommon {
    class Session;
}

class DamageTimePlotListener
{
public:
    virtual void onDamageTimePlotStartNew() = 0;
    virtual void onDamageTimePlotDataChanged() = 0;
    virtual void onDamageTimePlotNameChanged(int fighterIdx) = 0;
};
