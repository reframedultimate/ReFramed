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

class PluginManager;
class ActiveSessionManager;
class SessionView;

class ActiveSessionView : public QWidget
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
    // TODO
    void onGameStarted(rfcommon::Session* game);
    void onTrainingStarted(rfcommon::Session* training);

    void onActiveSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onActiveSessionManagerSetNumberChanged(rfcommon::SetNumber number) override;
    void onActiveSessionManagerGameNumberChanged(rfcommon::GameNumber number) override;
    void onActiveSessionManagerFormatChanged(const rfcommon::SetFormat& format) override;
    void onActiveSessionManagerWinnerChanged(int winner) override;
    void onActiveSessionManagerTrainingSessionNumberChanged(rfcommon::GameNumber number) override;

private:
    Ui::ActiveSessionView* ui_;
    ActiveSessionManager* activeSessionManager_;
    SessionView* sessionView_;
    std::vector<QGroupBox*> names_;
    std::vector<QLabel*> fighterName_;
    std::vector<QLabel*> fighterStatus_;
    std::vector<QLabel*> fighterDamage_;
    std::vector<QLabel*> fighterStocks_;
};

}
