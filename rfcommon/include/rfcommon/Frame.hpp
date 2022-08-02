#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterState.hpp"

namespace rfcommon {

template <int N>
class Frame : public SmallVector<FighterState, N>
{};

}
