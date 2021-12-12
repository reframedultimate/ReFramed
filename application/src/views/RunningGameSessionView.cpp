#include "application/Util.hpp"
#include "application/ui_RunningGameSessionView.h"
#include "application/views/RunningGameSessionView.hpp"
#include "application/views/SessionView.hpp"
#include "application/models/RunningGameSessionManager.hpp"
#include "rfcommon/RunningGameSession.hpp"

#include <QVBoxLayout>
#include <QDateTime>

namespace rfapp {

// ----------------------------------------------------------------------------
RunningGameSessionView::RunningGameSessionView(
        RunningGameSessionManager* runningGameSessionManager,
        PluginManager* pluginManager,
        QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::RunningGameSessionView)
    , runningGameSessionManager_(runningGameSessionManager)
    , sessionView_(new SessionView(pluginManager))
{
    ui_->setupUi(this);
    ui_->layout_recordingView->addWidget(sessionView_);
    ui_->lineEdit_formatOther->setVisible(false);

    // Initial page should show "disconnected"
    ui_->stackedWidget->setCurrentWidget(ui_->page_disconnected);

    connect(ui_->comboBox_format, qOverload<int>(&QComboBox::currentIndexChanged), this, &RunningGameSessionView::onComboBoxFormatIndexChanged);
    connect(ui_->lineEdit_formatOther, &QLineEdit::textChanged, this, &RunningGameSessionView::onLineEditFormatChanged);
    connect(ui_->spinBox_gameNumber, qOverload<int>(&QSpinBox::valueChanged), this, &RunningGameSessionView::onSpinBoxGameNumberChanged);
    connect(ui_->lineEdit_player1, &QLineEdit::textChanged, this, &RunningGameSessionView::onLineEditP1TextChanged);
    connect(ui_->lineEdit_player2, &QLineEdit::textChanged, this, &RunningGameSessionView::onLineEditP2TextChanged);

    runningGameSessionManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
RunningGameSessionView::~RunningGameSessionView()
{
    runningGameSessionManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::showDamagePlot()
{
    sessionView_->showDamagePlot();
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onComboBoxFormatIndexChanged(int index)
{
    if (static_cast<rfcommon::SetFormat::Type>(index) == rfcommon::SetFormat::OTHER)
        ui_->lineEdit_formatOther->setVisible(true);
    else
        ui_->lineEdit_formatOther->setVisible(false);
    runningGameSessionManager_->setFormat(rfcommon::SetFormat(
        static_cast<rfcommon::SetFormat::Type>(index),
        ui_->lineEdit_formatOther->text().toStdString().c_str()
    ));
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onLineEditFormatChanged(const QString& formatDesc)
{
    runningGameSessionManager_->setFormat(rfcommon::SetFormat(rfcommon::SetFormat::OTHER, formatDesc.toStdString().c_str()));
    ui_->comboBox_format->setCurrentIndex(static_cast<int>(rfcommon::SetFormat::OTHER));
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onSpinBoxGameNumberChanged(int value)
{
    runningGameSessionManager_->setGameNumber(value);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onLineEditP1TextChanged(const QString& name)
{
    runningGameSessionManager_->setP1Name(name);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onLineEditP2TextChanged(const QString& name)
{
    runningGameSessionManager_->setP2Name(name);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port)
{
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port)
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_waiting);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerDisconnectedFromServer()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_disconnected);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerMatchStarted(rfcommon::RunningGameSession* recording)
{
    clearLayout(ui_->layout_playerInfo);

    // Create individual player UIs
    int count = recording->playerCount();
    names_.resize(count);
    fighterName_.resize(count);
    fighterStatus_.resize(count);
    fighterDamage_.resize(count);
    fighterStocks_.resize(count);
    for (int i = 0; i != count; ++i)
    {
        fighterName_[i] = new QLabel();
        const rfcommon::String* fighterNameStr = recording->mappingInfo().fighterID.map(recording->playerFighterID(i));
        fighterName_[i]->setText(fighterNameStr ? fighterNameStr->cStr() : "(Unknown Fighter)");

        fighterStatus_[i] = new QLabel();
        fighterDamage_[i] = new QLabel();
        fighterStocks_[i] = new QLabel();

        names_[i] = new QGroupBox;
        names_[i]->setTitle(recording->playerName(i).cStr());

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
        names_[i]->setLayout(layout);
        ui_->layout_playerInfo->addWidget(names_[i]);
    }

    // Set game info
    const rfcommon::String* stageStr = recording->mappingInfo().stageID.map(recording->stageID());
    ui_->label_stage->setText(stageStr ? stageStr->cStr() : "(Unknown Stage)");
    ui_->label_date->setText(QDateTime::fromMSecsSinceEpoch(recording->timeStampStartedMs()).toString());
    ui_->label_timeRemaining->setText("");

    // Prepare recording view for new game
    sessionView_->setSession(recording);

    // Show active page, if not already
    ui_->stackedWidget->setCurrentWidget(ui_->page_active);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerMatchEnded(rfcommon::RunningGameSession* recording)
{
    (void)recording;
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerPlayerNameChanged(int playerIdx, const rfcommon::SmallString<15>& name)
{
    if (names_.size() <= playerIdx)
        return;

    names_[playerIdx]->setTitle(name.cStr());

    if (playerIdx == 0)
    {
        const QSignalBlocker blocker(ui_->lineEdit_player1);
        ui_->lineEdit_player1->setText(name.cStr());
    }
    else if (playerIdx == 1)
    {
        const QSignalBlocker blocker(ui_->lineEdit_player2);
        ui_->lineEdit_player2->setText(name.cStr());
    }
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerFormatChanged(const rfcommon::SetFormat& format)
{
    const QSignalBlocker blocker1(ui_->comboBox_format);
    const QSignalBlocker blocker2(ui_->lineEdit_formatOther);

    ui_->comboBox_format->setCurrentIndex(static_cast<int>(format.type()));

    if (format.type() == rfcommon::SetFormat::OTHER)
        ui_->lineEdit_formatOther->setText(format.description().cStr());
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerSetNumberChanged(rfcommon::SetNumber number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerGameNumberChanged(rfcommon::GameNumber number)
{
    const QSignalBlocker blocker(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(number);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerNewPlayerState(int player, const rfcommon::PlayerState& state)
{
    if (player >= fighterStatus_.size())
        return;

    fighterStatus_[player]->setText(QString::number(state.status()));
    fighterDamage_[player]->setText(QString::number(state.damage()));
    fighterStocks_[player]->setText(QString::number(state.stocks()));

    ui_->label_timeRemaining->setText(QTime(0, 0).addSecs(state.frame() / 60.0).toString());
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerWinnerChanged(int winner)
{
    for (int i = 0; i != names_.size(); ++i)
    {
        if (i == winner)
            names_[i]->setStyleSheet("background-color: rgb(211, 249, 216)");
        else
            names_[i]->setStyleSheet("background-color: rgb(249, 214, 211)");
    }
}

}
