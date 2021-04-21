#pragma once

#include <QWidget>
#include "application/listeners/ActiveRecordingManagerListener.hpp"

class QGroupBox;
class QLabel;

namespace uh {
    class ActiveRecording;
}

namespace Ui {
    class ActiveRecordingView;
}

namespace uhapp {

class ActiveRecordingManager;
class RecordingView;

class ActiveRecordingView : public QWidget
                          , public ActiveRecordingManagerListener
{
    Q_OBJECT
public:
    ActiveRecordingView(ActiveRecordingManager* manager, QWidget* parent=nullptr);
    ~ActiveRecordingView();

private slots:
    void onComboBoxFormatIndexChanged(int index);
    void onLineEditFormatChanged(const QString& formatDesc);
    void onSpinBoxGameNumberChanged(int value);
    void onLineEditP1TextChanged(const QString& name);
    void onLineEditP2TextChanged(const QString& name);

private slots:
    void onActiveRecordingManagerConnectedToServer();
    void onActiveRecordingManagerDisconnectedFromServer();

private:
    void onActiveRecordingManagerRecordingStarted(uh::ActiveRecording* recording) override;
    void onActiveRecordingManagerRecordingEnded(uh::ActiveRecording* recording) override;

    void onActiveRecordingManagerP1NameChanged(const QString& name) override;
    void onActiveRecordingManagerP2NameChanged(const QString& name) override;
    void onActiveRecordingManagerSetNumberChanged(uh::SetNumber number) override;
    void onActiveRecordingManagerGameNumberChanged(uh::GameNumber number) override;
    void onActiveRecordingManagerFormatChanged(const uh::SetFormat& format) override;
    void onActiveRecordingManagerPlayerStateAdded(int player, const uh::PlayerState& state) override;
    void onActiveRecordingManagerWinnerChanged(int winner) override;

private:
    Ui::ActiveRecordingView* ui_;
    ActiveRecordingManager* activeRecordingManager_;
    RecordingView* recordingView_;
    std::vector<QGroupBox*> names_;
    std::vector<QLabel*> fighterName_;
    std::vector<QLabel*> fighterStatus_;
    std::vector<QLabel*> fighterDamage_;
    std::vector<QLabel*> fighterStocks_;
};

}
