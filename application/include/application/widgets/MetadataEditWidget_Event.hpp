#pragma once

#include "application/widgets/MetadataEditWidget.hpp"

class QComboBox;
class QLineEdit;
class QLabel;

namespace rfapp {

class MetadataEditWidget_Event : public MetadataEditWidget
{
    Q_OBJECT

public:
    explicit MetadataEditWidget_Event(MetadataEditModel* model, QWidget* parent=nullptr);
    ~MetadataEditWidget_Event();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:
    void updateUIVisibility(rfcommon::BracketType bracketType);

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

private slots:
    void onComboBoxBracketTypeChanged(int index);
    void onLineEditBracketURLChanged(const QString& text);
    void onLineEditOtherChanged(const QString& text);

private:
    QComboBox* comboBox_bracketType_;
    QLabel* label_bracketURL_;
    QLineEdit* lineEdit_bracketURL_;
    QLineEdit* lineEdit_otherBracketType_;
    bool ignoreSelf_ = false;
};

}
