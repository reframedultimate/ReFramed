#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QVBoxLayout;

namespace rfapp {

class MetaDataEditWidget_Tournament : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Tournament(MetaDataEditModel* model, QWidget* parent=nullptr);
    ~MetaDataEditWidget_Tournament();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onAddTOReleased();
    void onAddSponsorReleased();

private:
    void onAdoptMetaData(rfcommon::MetaData* mdata) override;
    void onOverwriteMetaData(rfcommon::MetaData* mdata) override;
    void onMetaDataCleared(rfcommon::MetaData* mdata) override;
    void onBracketTypeChangedUI(rfcommon::BracketType bracketType) override;

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
