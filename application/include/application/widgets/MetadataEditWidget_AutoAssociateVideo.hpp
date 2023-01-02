#pragma once

#include "application/widgets/MetadataEditWidget.hpp"

class QVBoxLayout;
class QLineEdit;
class QSpinBox;

namespace rfapp {

class ActiveSessionManager;

class MetadataEditWidget_AutoAssociateVideo : public MetadataEditWidget
{
    Q_OBJECT

public:
    explicit MetadataEditWidget_AutoAssociateVideo(MetadataEditModel* model, ActiveSessionManager* activeSessionManager, QWidget* parent=nullptr);
    ~MetadataEditWidget_AutoAssociateVideo();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onCheckBoxEnableToggled(bool enable);
    void onToolButtonChooseDirectoryReleased();
    void onSpinBoxFrameOffsetValueChanged(int value);

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
    ActiveSessionManager* activeSessionManager_;
    QLabel* label_chooseDir;
    QToolButton* toolButton_chooseDir;
    QLineEdit* lineEdit_dir;
    QLabel* label_frameOffset;
    QSpinBox* spinBox_frameOffset;

    bool ignoreSelf_ = false;
};

}
