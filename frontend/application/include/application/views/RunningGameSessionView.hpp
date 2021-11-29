#pragma once

#include <QWidget>
#include "application/listeners/RunningGameSessionManagerListener.hpp"

class QGroupBox;
class QLabel;

namespace uh {
    class RunningGameSession;
}

namespace Ui {
    class RunningGameSessionView;
}

namespace uhapp {

class RunningGameSessionManager;
class SessionView;

class RunningGameSessionView : public QWidget
                             , public RunningGameSessionManagerListener
{
    Q_OBJECT
public:
    RunningGameSessionView(RunningGameSessionManager* manager, QWidget* parent=nullptr);
    ~RunningGameSessionView();

    void showDamagePlot();

private slots:
    void onComboBoxFormatIndexChanged(int index);
    void onLineEditFormatChanged(const QString& formatDesc);
    void onSpinBoxGameNumberChanged(int value);
    void onLineEditP1TextChanged(const QString& name);
    void onLineEditP2TextChanged(const QString& name);

private:
    void onRunningGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onRunningGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) override;
    void onRunningGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onRunningGameSessionManagerDisconnectedFromServer() override;

    void onRunningGameSessionManagerMatchStarted(uh::RunningGameSession* recording) override;
    void onRunningGameSessionManagerMatchEnded(uh::RunningGameSession* recording) override;

    void onRunningGameSessionManagerPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionManagerSetNumberChanged(uh::SetNumber number) override;
    void onRunningGameSessionManagerGameNumberChanged(uh::GameNumber number) override;
    void onRunningGameSessionManagerFormatChanged(const uh::SetFormat& format) override;
    void onRunningGameSessionManagerNewPlayerState(int player, const uh::PlayerState& state) override;
    void onRunningGameSessionManagerWinnerChanged(int winner) override;

private:
    Ui::RunningGameSessionView* ui_;
    RunningGameSessionManager* runningGameSessionManager_;
    SessionView* sessionView_;
    std::vector<QGroupBox*> names_;
    std::vector<QLabel*> fighterName_;
    std::vector<QLabel*> fighterStatus_;
    std::vector<QLabel*> fighterDamage_;
    std::vector<QLabel*> fighterStocks_;
};

}
