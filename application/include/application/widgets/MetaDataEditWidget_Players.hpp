#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QVBoxLayout;

namespace rfapp {

class MetaDataEditWidget_Players : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Players(QWidget* parent=nullptr);
    ~MetaDataEditWidget_Players();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

    void adoptMetaData() override;
    void overwriteMetaData() override;

private slots:
    void onAddTOReleased();
    void onAddSponsorReleased();

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
    QVBoxLayout* TOLayout_;
    QVBoxLayout* sponsorsLayout_;
};

}
