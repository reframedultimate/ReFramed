#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

namespace rfapp {

class MetaDataEditWidget_Event : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Event(QWidget* parent=nullptr);
    ~MetaDataEditWidget_Event();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

    void adoptMetaData() override;
    void overwriteMetaData() override;

private:
    void onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetaDataTournamentDetailsChanged() override;
    void onMetaDataEventDetailsChanged() override;
    void onMetaDataCommentatorsChanged() override;
    void onMetaDataGameDetailsChanged() override;
    void onMetaDataPlayerDetailsChanged() override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;
    void onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
};

}
