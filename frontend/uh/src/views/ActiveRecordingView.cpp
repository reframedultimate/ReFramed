#include "uh/ui_ActiveRecordingView.h"
#include "uh/views/ActiveRecordingView.hpp"
#include "uh/views/DamagePlot.hpp"
#include "uh/models/Recording.hpp"

#include <QVBoxLayout>

namespace uh {

// ----------------------------------------------------------------------------
ActiveRecordingView::ActiveRecordingView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::ActiveRecordingView)
{
    ui_->setupUi(this);

    plot_ = new DamagePlot;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(plot_);
    ui_->tab_damagePlot->setLayout(layout);

    ui_->lineEdit_formatOther->setVisible(false);

    connect(ui_->comboBox_format, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxFormatIndexChanged(int)));
    connect(ui_->lineEdit_formatOther, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditFormatChanged(const QString&)));
    connect(ui_->spinBox_gameNumber, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxGameNumberChanged(int)));
    connect(ui_->lineEdit_player1, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditP1TextChanged(const QString&)));
    connect(ui_->lineEdit_player2, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditP2TextChanged(const QString&)));

    setActive();
}

// ----------------------------------------------------------------------------
ActiveRecordingView::~ActiveRecordingView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setWaitingForGame()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_waiting);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setActive()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_active);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setDisconnected()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_disconnected);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setTimeStarted(const QDateTime& date)
{
    ui_->label_date->setText(date.toString());
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setStageName(const QString& stage)
{
    ui_->label_stage->setText(stage);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerCount(int count)
{
    // Clear layout
    QLayoutItem* item;
    while ((item = ui_->layout_playerInfo->takeAt(0)) != nullptr)
    {
        if (item->layout() != nullptr)
            item->layout()->deleteLater();
        if (item->widget() != nullptr)
            item->widget()->deleteLater();
    }

    tags_.resize(count);
    fighterName_.resize(count);
    fighterStatus_.resize(count);
    fighterDamage_.resize(count);
    fighterStocks_.resize(count);

    // Create player UIs
    for (int i = 0; i != count; ++i)
    {
        tags_[i] = new QGroupBox;
        fighterName_[i] = new QLabel();
        fighterStatus_[i] = new QLabel();
        fighterDamage_[i] = new QLabel();
        fighterStocks_[i] = new QLabel();

        QFormLayout* layout = new QFormLayout;
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">Fighter:</span></p></body></html>"),
            fighterName_[i]);
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">State:</span></p></body></html>"),
            fighterStatus_[i]);
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">Damage:</span></p></body></html>"),
            fighterDamage_[i]);
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">Stocks:</span></p></body></html>"),
            fighterStocks_[i]);

        tags_[i]->setLayout(layout);
        ui_->layout_playerInfo->addWidget(tags_[i]);
    }

    plot_->resetPlot(count);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerTag(int index, const QString& tag)
{
    tags_[index]->setTitle(tag);
    plot_->setPlayerTag(index, tag);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerFighterName(int index, const QString& fighterName)
{
    fighterName_[index]->setText(fighterName);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onRecordingStarted(Recording* recording)
{
    setPlayerCount(recording->playerCount());

    uint16_t stageID = recording->gameInfo().stageID();
    const QString* stageStr = recording->mappingInfo().stageID.map(stageID);

    setTimeStarted(recording->gameInfo().timeStarted());
    setStageName(stageStr? *stageStr : "(Unknown Stage)");
    setPlayerCount(recording->playerCount());

    switch (recording->gameInfo().format())
    {
    case GameInfo::FRIENDLIES: break;
    }

    if (recording->playerCount() == 2)
    {
        PlayerInfo& info1 = recording->playerInfo(0);
        PlayerInfo& info2 = recording->playerInfo(1);

        // If the player tag changed from the last recording, then we should
        // clear any custom tag entered into the textbox so the tag that got
        // sent to us is the one that gets saved
        if (lastP1Tag_.length() && lastP1Tag_ != info1.tag())
            ui_->lineEdit_player1->setText("");
        if (lastP2Tag_.length() && lastP2Tag_ != info2.tag())
            ui_->lineEdit_player1->setText("");
        lastP1Tag_ = info1.tag();
        lastP2Tag_ = info2.tag();

        ui_->lineEdit_player1->setPlaceholderText(info1.tag());
        ui_->lineEdit_player2->setPlaceholderText(info2.tag());

        // If custom tags are set, then those have precedence
        if (ui_->lineEdit_player1->text().length() > 0)
            info1.setTag(ui_->lineEdit_player1->text());
        if (ui_->lineEdit_player2->text().length() > 0)
            info2.setTag(ui_->lineEdit_player2->text());
    }
    else
    {
        ui_->lineEdit_player1->setPlaceholderText("");
        ui_->lineEdit_player2->setPlaceholderText("");
        ui_->lineEdit_player1->setEnabled(false);
        ui_->lineEdit_player2->setEnabled(false);
    }

    for (int i = 0; i != recording->playerCount(); ++i)
    {
        const PlayerInfo& info = recording->playerInfo(i);
        const QString* fighterStr = recording->mappingInfo().fighterID.map(info.fighterID());

        setPlayerTag(i, info.tag());
        setPlayerFighterName(i, fighterStr? *fighterStr : "(Unknown Fighter)");
    }

    setActive();
    activeRecording_ = recording;
    recording->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onRecordingEnded(Recording* recording)
{
    recording->dispatcher.removeListener(this);
    activeRecording_= nullptr;

    ui_->lineEdit_player1->setEnabled(true);
    ui_->lineEdit_player2->setEnabled(true);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onRecordingPlayerStateAdded(int playerID, const PlayerState& state)
{
    fighterStatus_[playerID]->setText(QString::number(state.status()));
    fighterDamage_[playerID]->setText(QString::number(state.damage()));
    fighterStocks_[playerID]->setText(QString::number(state.stocks()));

    ui_->label_timeRemaining->setText(QTime(0, 0).addSecs(state.frame() / 60.0).toString());

    plot_->addPlayerDamageValue(playerID, state.frame(), state.damage());
    plot_->replotAndAutoScale();
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onComboBoxFormatIndexChanged(int index)
{
    if (index == GameInfo::OTHER)
        ui_->lineEdit_formatOther->setVisible(true);
    else
        ui_->lineEdit_formatOther->setVisible(false);

    if (index > GameInfo::OTHER)
        return;
    if (activeRecording_ == nullptr)
        return;

    if (index == GameInfo::OTHER)
        activeRecording_->gameInfo().setFormat(static_cast<GameInfo::Format>(index), ui_->lineEdit_formatOther->text());
    else
        activeRecording_->gameInfo().setFormat(static_cast<GameInfo::Format>(index));
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onLineEditFormatChanged(const QString& formatDesc)
{
    if (activeRecording_ == nullptr)
        return;
    activeRecording_->gameInfo().setFormat(activeRecording_->gameInfo().format(), ui_->lineEdit_formatOther->text());
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onSpinBoxGameNumberChanged(int value)
{
    if (activeRecording_ == nullptr)
        return;
    activeRecording_->gameInfo().setGameNumber(value);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onLineEditP1TextChanged(const QString& name)
{
    if (activeRecording_ == nullptr)
        return;
    if (activeRecording_->playerCount() != 2)
        return;
    activeRecording_->playerInfo(0).setTag(name);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onLineEditP2TextChanged(const QString& name)
{
    if (activeRecording_ == nullptr)
        return;
    if (activeRecording_->playerCount() != 2)
        return;
    activeRecording_->playerInfo(1).setTag(name);
}

}
