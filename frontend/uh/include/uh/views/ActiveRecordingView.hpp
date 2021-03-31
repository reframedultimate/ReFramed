#pragma once

#include <QWidget>
#include "uh/listeners/RecordingListener.hpp"
#include "uh/models/GameInfo.hpp"
#include "uh/models/PlayerInfo.hpp"
#include "uh/models/PlayerState.hpp"
#include "uh/models/MappingInfo.hpp"

class QGroupBox;
class QLabel;

namespace Ui {
    class ActiveRecordingView;
}

namespace uh {

class GameInfo;
class MappingInfo;
class PlayerInfo;
class PlayerState;
class DamagePlot;
class Recording;

class ActiveRecordingView : public QWidget
                          , public RecordingListener
{
    Q_OBJECT
public:
    ActiveRecordingView(QWidget* parent=nullptr);
    ~ActiveRecordingView();

signals:
    void gameFormatChanged(const QString& format);
    void gameIndexChanged(int index);
    void player1NameChanged(const QString& name);
    void player2NameChanged(const QString& name);

public slots:
    void setWaitingForGame();
    void setActive();
    void setDisconnected();

    void setTimeStarted(const QDateTime& date);
    void setStageName(const QString& stage);
    void setPlayerCount(int count);
    void setPlayerTag(int index, const QString& tag);
    void setPlayerFighterName(int index, const QString& fighterName);

private slots:
    void onRecordingStarted(Recording* recording);
    void onRecordingEnded(Recording* recording);

    void onComboBoxFormatIndexChanged(int index);
    void onLineEditFormatChanged(const QString& formatDesc);
    void onSpinBoxGameNumberChanged(int value);
    void onLineEditP1TextChanged(const QString& name);
    void onLineEditP2TextChanged(const QString& name);

private:
    void onRecordingPlayerStateAdded(int playerID, const PlayerState& state);

private:
    Ui::ActiveRecordingView* ui_;
    DamagePlot* plot_;
    Recording* activeRecording_ = nullptr;
    QVector<QGroupBox*> tags_;
    QVector<QLabel*> fighterName_;
    QVector<QLabel*> fighterStatus_;
    QVector<QLabel*> fighterDamage_;
    QVector<QLabel*> fighterStocks_;

    QString lastP1Tag_;
    QString lastP2Tag_;
};

}
