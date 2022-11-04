#pragma once

#include <QPixmap>

namespace rfapp {

class FighterIcon
{
    FighterIcon() {}
public:
    static QPixmap fromFighterName(const char* name, int skin);
};

}
