#pragma once

#include "uhplot/config.hpp"
#include <QColor>

namespace uhplot {

struct UHPLOT_PUBLIC_API ColorPalette
{
    static QColor getColor(int index);
};

} // namespace uh
