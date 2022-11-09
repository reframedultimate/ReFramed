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

private slots:
    void onAddCommentatorReleased();

private:
    void onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    // Game related events
    void onMetaDataPlayerNameChanged(int fighterIdx, const char* name) override;
    void onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) override;
    void onMetaDataTournamentNameChanged(const char* name) override;
    void onMetaDataEventNameChanged(const char* name) override;
    void onMetaDataRoundNameChanged(const char* name) override;
    void onMetaDataCommentatorsChanged(const rfcommon::SmallVector<rfcommon::String, 2>& names) override;
    void onMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;

    // In training mode this increments every time a new training room is loaded
    void onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) override;

private:
    QVBoxLayout* commentatorsLayout_;
};

}
