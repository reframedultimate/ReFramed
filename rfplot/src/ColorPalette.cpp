#include "rfcommon/Profiler.hpp"
#include "rfplot/ColorPalette.hpp"

#include <QVector>

namespace rfplot {

static QVector<QColor> generateColorPalette()
{
    PROFILE(ColorPaletteGlobal, generateColorPalette);

    QVector<QColor> colors;

    colors.push_back(QColor(0, 0, 255));
    colors.push_back(QColor(255, 0, 0));
    colors.push_back(QColor(40, 128, 0));
    colors.push_back(QColor(255, 128, 0));
    colors.push_back(QColor(0, 255, 255));
    colors.push_back(QColor(0, 128, 255));
    colors.push_back(QColor(255, 0, 255));

    return colors;
}

// ----------------------------------------------------------------------------
QColor ColorPalette::getColor(int index)
{
    PROFILE(ColorPalette, getColor);

    static QVector<QColor> colors = generateColorPalette();

    while(index >= colors.size())
        index -= colors.size();
    return colors.at(index);
}

}
