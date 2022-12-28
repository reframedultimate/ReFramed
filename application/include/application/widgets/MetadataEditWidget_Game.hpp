#pragma once

#include "application/widgets/MetadataEditWidget.hpp"

class QVBoxLayout;

namespace Ui {
    class MetadataEditWidget_Game;
}

namespace rfcommon {
    class Metadata;
}

namespace rfapp {

class PlayerDetails;

class MetadataEditWidget_Game : public MetadataEditWidget
{
    Q_OBJECT

public:
    explicit MetadataEditWidget_Game(MetadataEditModel* model, PlayerDetails* playerDetails, QWidget* parent=nullptr);
    ~MetadataEditWidget_Game();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onDateTimeStartedChanged(const QDateTime& dateTime);
    void onPushButtonResetTimeStartedReleased();
    void onComboBoxRoundTypeChanged(int index);
    void onSpinBoxRoundNumberChanged(int value);
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
    void updateRoundTypeUI(int roundTypeIndex);
    void enableRoundCounter(bool enable);
    void enableFreePlayOption(bool enable);
    void enableRoundTypeSelection(bool enable);
    void enableGrandFinalOptions(bool enable);

private:
    void onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata) override;
    void onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata) override;
    void onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata) override;
    void onNextGameStarted() override;
    void onBracketTypeChangedUI(rfcommon::BracketType bracketType) override;

    void onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetadataTournamentDetailsChanged() override;
    void onMetadataEventDetailsChanged() override;
    void onMetadataCommentatorsChanged() override;
    void onMetadataGameDetailsChanged() override;
    void onMetadataPlayerDetailsChanged() override;
    void onMetadataWinnerChanged(int winnerPlayerIdx) override;
    void onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
    PlayerDetails* playerDetails_;
    Ui::MetadataEditWidget_Game* ui_;
    bool ignoreSelf_ = false;
};

}
