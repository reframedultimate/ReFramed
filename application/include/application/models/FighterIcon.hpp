#pragma once

#include <QPixmap>

#include "rfcommon/FighterID.hpp"
#include "rfcommon/Costume.hpp"

namespace rfapp {

class FighterIcon
{
    FighterIcon() {}
public:
    static QPixmap fromFighterName(const char* name, rfcommon::Costume costume);
    static QPixmap fromFighterID(rfcommon::FighterID fighterID, rfcommon::Costume costume);
};

}
