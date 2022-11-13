#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QVBoxLayout;

namespace Ui {
    class MetaDataEditWidget_Game;
}

namespace rfapp {

class MetaDataEditWidget_Game : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Game(QWidget* parent=nullptr);
    ~MetaDataEditWidget_Game();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

    void adoptMetaData() override;
    void overwriteMetaData() override;

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
    Ui::MetaDataEditWidget_Game* ui_;
};

}
