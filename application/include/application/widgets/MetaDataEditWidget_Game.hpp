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
    void onComboBoxRoundTypeChanged(int index);

private:
    void enableFreePlayOption(bool enable);
    void enableRoundTypeSelection(bool enable);
    void enableGrandFinalOptions(bool enable);

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
    Ui::MetaDataEditWidget_Game* ui_;
    rfcommon::Reference<rfcommon::MetaData> prevMetaData_;
};

}
