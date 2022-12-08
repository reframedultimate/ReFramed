#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QVBoxLayout;
class QLineEdit;

namespace rfapp {

class ActiveSessionManager;

class MetaDataEditWidget_AutoAssociateVideo : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_AutoAssociateVideo(MetaDataEditModel* model, ActiveSessionManager* activeSessionManager, QWidget* parent=nullptr);
    ~MetaDataEditWidget_AutoAssociateVideo();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onCheckBoxEnableToggled(bool enable);
    void onToolButtonChooseDirectoryReleased();

private:
    void onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata) override;
    void onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata) override;
    void onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata) override;
    void onNextGameStarted() override;
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
    ActiveSessionManager* activeSessionManager_;
    QLabel* label_chooseDir;
    QToolButton* toolButton_chooseDir;
    QLineEdit* lineEdit_dir;

    bool ignoreSelf_ = false;
};

}
