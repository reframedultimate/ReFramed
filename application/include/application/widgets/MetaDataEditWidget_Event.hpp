#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QLineEdit;
class QLabel;

namespace rfapp {

class MetaDataEditWidget_Event : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Event(MetaDataEditModel* model, QWidget* parent=nullptr);
    ~MetaDataEditWidget_Event();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:
    void onAdoptMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) override;
    void onOverwriteMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) override;
    void onMetaDataCleared(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) override;
    void onBracketTypeChangedUI(rfcommon::BracketType bracketType) override;

    void onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetaDataTournamentDetailsChanged() override;
    void onMetaDataEventDetailsChanged() override;
    void onMetaDataCommentatorsChanged() override;
    void onMetaDataGameDetailsChanged() override;
    void onMetaDataPlayerDetailsChanged() override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;
    void onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private slots:
    void onComboBoxBracketTypeChanged(int index);

private:
    QLabel* label_bracketURL_;
    QLineEdit* lineEdit_bracketURL_;
    QLineEdit* lineEdit_otherBracketType_;
};

}
