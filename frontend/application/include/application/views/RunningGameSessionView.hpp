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
class RecordingView;

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

private slots:
    void onRunningGameSessionManagerConnectedToServer();
    void onRunningGameSessionManagerDisconnectedFromServer();

private:
    void onRunningGameSessionManagerRecordingStarted(uh::RunningGameSession* recording) override;
    void onRunningGameSessionManagerRecordingEnded(uh::RunningGameSession* recording) override;

    void onRunningGameSessionManagerP1NameChanged(const QString& name) override;
    void onRunningGameSessionManagerP2NameChanged(const QString& name) override;
    void onRunningGameSessionManagerSetNumberChanged(uh::SetNumber number) override;
    void onRunningGameSessionManagerGameNumberChanged(uh::GameNumber number) override;
    void onRunningGameSessionManagerFormatChanged(const uh::SetFormat& format) override;
    void onRunningGameSessionManagerPlayerStateAdded(int player, const uh::PlayerState& state) override;
    void onRunningGameSessionManagerWinnerChanged(int winner) override;

private:
    Ui::RunningGameSessionView* ui_;
    RunningGameSessionManager* activeRecordingManager_;
    RecordingView* recordingView_;
    std::vector<QGroupBox*> names_;
    std::vector<QLabel*> fighterName_;
    std::vector<QLabel*> fighterStatus_;
    std::vector<QLabel*> fighterDamage_;
    std::vector<QLabel*> fighterStocks_;
};

}
