#pragma once

#include <QWidget>
#include "application/listeners/ActiveSessionManagerListener.hpp"

class QGroupBox;
class QLabel;

namespace rfcommon {
    class Session;
}

namespace Ui {
    class ActiveSessionView;
}

namespace rfapp {

class ActiveSessionManager;
class PluginManager;
class Protocol;

class ActiveSessionView 
    : public QWidget
    , public ActiveSessionManagerListener
{
    Q_OBJECT
public:
    ActiveSessionView(
            ActiveSessionManager* activeSessionManager,
            PluginManager* pluginManager,
            QWidget* parent=nullptr);
    ~ActiveSessionView();

private slots:
    void onComboBoxFormatIndexChanged(int index);
    void onLineEditFormatChanged(const QString& formatDesc);
    void onSpinBoxGameNumberChanged(int value);
    void onLineEditP1TextChanged(const QString& name);
    void onLineEditP2TextChanged(const QString& name);

private:
    void onActiveSessionManagerConnected(const char* ip, uint16_t port) override;
    void onActiveSessionManagerDisconnected() override;

    void onActiveSessionManagerGameStarted(rfcommon::Session* game) override;
    void onActiveSessionManagerGameEnded(rfcommon::Session* game) override;
    void onActiveSessionManagerTrainingStarted(rfcommon::Session* training) override;
    void onActiveSessionManagerTrainingEnded(rfcommon::Session* training) override;

    void onActiveSessionManagerTimeRemainingChanged(double seconds) override;
    void onActiveSessionManagerFighterStateChanged(int fighterIdx, float damage, int stocks) override;

    void onActiveSessionManagerTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onActiveSessionManagerTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    void onActiveSessionManagerPlayerNameChanged(int fighterIdx, const rfcommon::String& name) override;
    void onActiveSessionManagerSetNumberChanged(rfcommon::SetNumber number) override;
    void onActiveSessionManagerGameNumberChanged(rfcommon::GameNumber number) override;
    void onActiveSessionManagerSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onActiveSessionManagerWinnerChanged(int winnerPlayerIdx) override;

    void onActiveSessionManagerTrainingSessionNumberChanged(rfcommon::GameNumber number) override;

private:
    Ui::ActiveSessionView* ui_;
    ActiveSessionManager* activeSessionManager_;
    std::vector<QGroupBox*> names_;
    std::vector<QLabel*> fighterName_;
    std::vector<QLabel*> fighterDamage_;
    std::vector<QLabel*> fighterStocks_;
};

}
