#include "uh/Util.hpp"
#include <QLayout>
#include <QLayoutItem>
#include <QWidget>

namespace uh {

// ----------------------------------------------------------------------------
void clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->layout() != nullptr)
            item->layout()->deleteLater();
        if (item->widget() != nullptr)
            item->widget()->deleteLater();
    }
}

}

