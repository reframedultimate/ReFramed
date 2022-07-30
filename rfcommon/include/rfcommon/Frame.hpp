#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterState.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API Frame : public SmallVector<FighterState, 4>
{};

}

