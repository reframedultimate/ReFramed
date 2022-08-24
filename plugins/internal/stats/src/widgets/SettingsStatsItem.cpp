#include "stats/widgets/SettingsStatsItem.hpp"
#include <QSizePolicy>
#include <QLabel>
#include <QStyle>

// ----------------------------------------------------------------------------
SettingsStatsItem::SettingsStatsItem(StatType type, QWidget* parent)
    : QLabel(statTypeToString(type), parent)
    , type_(type)
{
    setMinimumHeight(30);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    switch (type)
    {
#define X(type, str, colorcode) \
        case STAT_##type: \
            setStyleSheet("border: 2px solid; border-radius: 10px; background-color: " colorcode ";"); \
            break;
        STAT_TYPES_LIST
#undef X
    }
}

// ----------------------------------------------------------------------------
void SettingsStatsItem::setDragInProgress()
{
    setText("...");
}

// ----------------------------------------------------------------------------
void SettingsStatsItem::setDragCancelled()
{
    setText(typeAsString());
}
