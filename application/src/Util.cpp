#include "rfcommon/Profiler.hpp"
#include "application/Util.hpp"

#include <QLayout>
#include <QLayoutItem>
#include <QWidget>
#include <QStackedWidget>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
#include <QScreen>

#include <cstring>

// ----------------------------------------------------------------------------
qhash_result_t qHash(const QDir& c, qhash_result_t seed) noexcept
{
    return qHash(c.canonicalPath().toUtf8(), seed);
}
qhash_result_t qHash(const QFileInfo& c, qhash_result_t seed) noexcept
{
    return qHash(c.absoluteFilePath().toUtf8(), seed);
}

namespace rfapp {

// ----------------------------------------------------------------------------
void clearLayout(QLayout* layout)
{
    PROFILE(UtilGlobal, clearLayout);

    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->layout() != nullptr)
            item->layout()->deleteLater();
        if (item->widget() != nullptr)
            item->widget()->deleteLater();
    }
}

// ----------------------------------------------------------------------------
void clearStackedWidget(QStackedWidget* sw)
{
    PROFILE(UtilGlobal, clearStackedWidget);

    while (sw->count())
    {
        QWidget* widget = sw->widget(0);
        sw->removeWidget(widget);
        widget->deleteLater();
    }
}

// ----------------------------------------------------------------------------
QRect calculatePopupGeometryKeepSize(const QWidget* main, const QWidget* popup, QRect popupRect)
{
    PROFILE(UtilGlobal, calculatePopupGeometryKeepSize);

    QRect mainRect = main->geometry();

    return QRect(
        mainRect.left() + mainRect.width() / 2 - popupRect.width() / 2,
        mainRect.top() + mainRect.height() / 2 - popupRect.height() / 2,
        popupRect.width(),
        popupRect.height()
    );
}

// ----------------------------------------------------------------------------
QRect calculatePopupGeometryActiveScreen()
{
    PROFILE(UtilGlobal, calculatePopupGeometryActiveScreen);

    QScreen* screen = QApplication::screenAt(QCursor::pos());
    if (screen == nullptr)
        screen = QApplication::primaryScreen();

    QRect screenRect = screen->geometry();
    int width = screenRect.width() * 3 / 4;
    int height = screenRect.height() * 3 / 4;
    int x = (screenRect.width() - width) / 2 + screenRect.x();
    int y = (screenRect.height() - height) / 2 + screenRect.y();

    return QRect(x, y, width, height);
}

}
