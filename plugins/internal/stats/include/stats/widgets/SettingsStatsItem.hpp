#pragma once

#include "stats/StatType.hpp"
#include <QLabel>

class SettingsStatsItem : public QLabel
{
    Q_OBJECT

public:
    explicit SettingsStatsItem(StatType type, QWidget* parent=nullptr);

    StatType type() const
        { return type_; }

    const char* typeAsString() const
        { return statTypeToString(type_); }

    void setDragInProgress();
    void setDragCancelled();

private:
    const StatType type_;
};
