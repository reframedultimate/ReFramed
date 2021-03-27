#pragma once

#include <QWidget>
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

class ActiveRecordingView : public QWidget
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

    void setPlayerStatus(unsigned int frame, int index, unsigned int status);
    void setPlayerDamage(unsigned int frame, int index, float damage);
    void setPlayerStockCount(unsigned int frame, int index, unsigned char stocks);

private slots:
    //void onComboBoxFormatIndexChanged(int index);

private:
    Ui::ActiveRecordingView* ui_;
    DamagePlot* plot_;
    QVector<QGroupBox*> tags_;
    QVector<QLabel*> fighterName_;
    QVector<QLabel*> fighterStatus_;
    QVector<QLabel*> fighterDamage_;
    QVector<QLabel*> fighterStocks_;
};

}
