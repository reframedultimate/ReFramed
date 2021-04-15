#pragma once

#include <QWidget>
#include "uh/listeners/ActiveRecordingManagerListener.hpp"

class QGroupBox;
class QLabel;

namespace Ui {
    class ActiveRecordingView;
}

namespace uh {

class ActiveRecordingManager;
class RecordingView;
class PlayerState;

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
    void onActiveRecordingManagerRecordingStarted(ActiveRecording* recording) override;
    void onActiveRecordingManagerRecordingEnded(ActiveRecording* recording) override;
    void onActiveRecordingManagerRecordingSaved(const QFileInfo& absPathToFile) override;

    void onActiveRecordingManagerP1NameChanged(const QString& name) override;
    void onActiveRecordingManagerP2NameChanged(const QString& name) override;
    void onActiveRecordingManagerSetNumberChanged(int number) override;
    void onActiveRecordingManagerGameNumberChanged(int number) override;
    void onActiveRecordingManagerFormatChanged(const SetFormat& format) override;
    void onActiveRecordingManagerPlayerStateAdded(int player, const PlayerState& state) override;
    void onActiveRecordingManagerWinnerChanged(int winner) override;

private:
    Ui::ActiveRecordingView* ui_;
    ActiveRecordingManager* activeRecordingManager_;
    RecordingView* recordingView_;
    QVector<QGroupBox*> names_;
    QVector<QLabel*> fighterName_;
    QVector<QLabel*> fighterStatus_;
    QVector<QLabel*> fighterDamage_;
    QVector<QLabel*> fighterStocks_;
};

}
