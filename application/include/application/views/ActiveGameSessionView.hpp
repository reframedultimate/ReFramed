#pragma once

#include <QWidget>
#include "application/listeners/ActiveGameSessionManagerListener.hpp"

class QGroupBox;
class QLabel;

namespace rfcommon {
    class Session;
}

namespace Ui {
    class ActiveGameSessionView;
}

namespace rfapp {

class PluginManager;
class ActiveGameSessionManager;
class SessionView;

class ActiveGameSessionView : public QWidget
                            , public ActiveGameSessionManagerListener
{
    Q_OBJECT
public:
    ActiveGameSessionView(
            ActiveGameSessionManager* activeGameSessionManager,
            PluginManager* pluginManager,
            QWidget* parent=nullptr);
    ~ActiveGameSessionView();

    void showDamagePlot();

private slots:
    void onComboBoxFormatIndexChanged(int index);
    void onLineEditFormatChanged(const QString& formatDesc);
    void onSpinBoxGameNumberChanged(int value);
    void onLineEditP1TextChanged(const QString& name);
    void onLineEditP2TextChanged(const QString& name);

private:
    void onActiveGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onActiveGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) override;
    void onActiveGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onActiveGameSessionManagerDisconnectedFromServer() override;

    void onActiveGameSessionManagerMatchStarted(rfcommon::Session* recording) override;
    void onActiveGameSessionManagerMatchEnded(rfcommon::Session* recording) override;

    void onActiveGameSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onActiveGameSessionManagerSetNumberChanged(rfcommon::SetNumber number) override;
    void onActiveGameSessionManagerGameNumberChanged(rfcommon::GameNumber number) override;
    void onActiveGameSessionManagerFormatChanged(const rfcommon::SetFormat& format) override;
    void onActiveGameSessionManagerNewFrame(int frameIdx, const rfcommon::Frame& frame) override;
    void onActiveGameSessionManagerWinnerChanged(int winner) override;

private:
    Ui::ActiveGameSessionView* ui_;
    ActiveGameSessionManager* runningGameSessionManager_;
    SessionView* sessionView_;
    std::vector<QGroupBox*> names_;
    std::vector<QLabel*> fighterName_;
    std::vector<QLabel*> fighterStatus_;
    std::vector<QLabel*> fighterDamage_;
    std::vector<QLabel*> fighterStocks_;
};

}
