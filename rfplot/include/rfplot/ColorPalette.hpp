#pragma once

#include "rfplot/config.hpp"
#include <QColor>

namespace rfplot {

struct RFPLOT_PUBLIC_API ColorPalette
{
    static QColor getColor(int index);
};

} // namespace uh
