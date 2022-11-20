#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"
#include "rfcommon/Reference.hpp"

class QVBoxLayout;

namespace Ui {
    class MetaDataEditWidget_Game;
}

namespace rfcommon {
    class MetaData;
}

namespace rfapp {

class MetaDataEditWidget_Game : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Game(MetaDataEditModel* model, QWidget* parent=nullptr);
    ~MetaDataEditWidget_Game();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onDateTimeStartedChanged(const QDateTime& dateTime);
    void onPushButtonResetTimeStartedReleased();
    void onComboBoxRoundTypeChanged(int index);
    void onComboBoxSetFormatChanged(int index);
    void onSpinBoxGameNumberChanged(int value);
    void onCheckBoxLeftLoserSideChanged(bool enable);
    void onCheckBoxRightLoserSideChanged(bool enable);
    void onLineEditLeftNameChanged(const QString& text);
    void onLineEditRightNameChanged(const QString& text);
    void onLineEditLeftSponsorChanged(const QString& text);
    void onLineEditRightSponsorChanged(const QString& text);
    void onLineEditLeftSocialChanged(const QString& text);
    void onLineEditRightSocialChanged(const QString& text);
    void onLineEditLeftPronounsChanged(const QString& text);
    void onLineEditRightPronounsChanged(const QString& text);
    void onPushButtonIncLeftScoreReleased();
    void onPushButtonDecLeftScoreReleased();
    void onPushButtonIncRightScoreReleased();
    void onPushButtonDecRightScoreReleased();

private:
    void enableRoundCounter(bool enable);
    void enableFreePlayOption(bool enable);
    void enableRoundTypeSelection(bool enable);
    void enableGrandFinalOptions(bool enable);

private:
    void onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata) override;
    void onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata) override;
    void onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata) override;
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
    Ui::MetaDataEditWidget_Game* ui_;
    rfcommon::Reference<rfcommon::MetaData> prevMetaData_;
    bool ignoreSelf_ = false;
};

}
