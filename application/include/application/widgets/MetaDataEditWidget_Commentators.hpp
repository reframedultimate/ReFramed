#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QVBoxLayout;

namespace rfapp {

class MetaDataEditWidget_Commentators : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Commentators(QWidget* parent=nullptr);
    ~MetaDataEditWidget_Commentators();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

    void adoptMetaData() override {}
    void overwriteMetaData() override {}

private slots:
    void onAddCommentatorReleased();

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
    QVBoxLayout* commentatorsLayout_;
};

}
