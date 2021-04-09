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
class DamagePlot;
class PlayerState;

class ActiveRecordingView : public QWidget
                          , public ActiveRecordingManagerListener
{
    Q_OBJECT
public:
    ActiveRecordingView(ActiveRecordingManager* model, QWidget* parent=nullptr);
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
    void onActiveRecordingManagerRecordingSaved(const QString& fileName) override;
    void onActiveRecordingManagerP1NameChanged(const QString& name) override;
    void onActiveRecordingManagerP2NameChanged(const QString& name) override;
    void onActiveRecordingManagerFormatChanged(SetFormat format, const QString& otherFormatDesc) override;
    void onActiveRecordingManagerSetNumberChanged(int number) override;
    void onActiveRecordingManagerGameNumberChanged(int number) override;
    void onActiveRecordingManagerPlayerStateAdded(int playerID, const PlayerState& state) override;

private:
    Ui::ActiveRecordingView* ui_;
    DamagePlot* plot_;
    ActiveRecordingManager* model_;
    QVector<QGroupBox*> names_;
    QVector<QLabel*> fighterName_;
    QVector<QLabel*> fighterStatus_;
    QVector<QLabel*> fighterDamage_;
    QVector<QLabel*> fighterStocks_;
};

}
