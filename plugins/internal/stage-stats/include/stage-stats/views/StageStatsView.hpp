#pragma once

#include "stage-stats/listeners/StageStatsListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class QGridLayout;
class StageStatsModel;

class StageStatsView
        : public QWidget
        , public StageStatsListener
{
public:
    explicit StageStatsView(StageStatsModel* model, QWidget* parent=nullptr);
    ~StageStatsView();

private:
    void onDataUpdated() override;

private:
    StageStatsModel* model_;
    QGridLayout* layout_;
};
