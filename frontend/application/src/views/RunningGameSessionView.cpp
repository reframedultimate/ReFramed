#include "application/Util.hpp"
#include "application/ui_RunningGameSessionView.h"
#include "application/views/RunningGameSessionView.hpp"
#include "application/views/RecordingView.hpp"
#include "application/models/RunningGameSessionManager.hpp"
#include "uh/RunningGameSession.hpp"

#include <QVBoxLayout>
#include <QDateTime>

namespace uhapp {

// ----------------------------------------------------------------------------
RunningGameSessionView::RunningGameSessionView(RunningGameSessionManager* manager, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::RunningGameSessionView)
    , activeRecordingManager_(manager)
    , recordingView_(new RecordingView)
{
    ui_->setupUi(this);
    ui_->layout_recordingView->addWidget(recordingView_);
    ui_->lineEdit_formatOther->setVisible(false);

    // Initial page should show "disconnected"
    ui_->stackedWidget->setCurrentWidget(ui_->page_disconnected);

    connect(ui_->comboBox_format, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxFormatIndexChanged(int)));
    connect(ui_->lineEdit_formatOther, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditFormatChanged(const QString&)));
    connect(ui_->spinBox_gameNumber, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxGameNumberChanged(int)));
    connect(ui_->lineEdit_player1, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditP1TextChanged(const QString&)));
    connect(ui_->lineEdit_player2, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditP2TextChanged(const QString&)));

    connect(activeRecordingManager_, SIGNAL(connectedToServer()), this, SLOT(onRunningGameSessionManagerConnectedToServer()));
    connect(activeRecordingManager_, SIGNAL(disconnectedFromServer()), this, SLOT(onRunningGameSessionManagerDisconnectedFromServer()));

    activeRecordingManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
RunningGameSessionView::~RunningGameSessionView()
{
    activeRecordingManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::showDamagePlot()
{
    recordingView_->showDamagePlot();
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onComboBoxFormatIndexChanged(int index)
{
    if (static_cast<uh::SetFormat::Type>(index) == uh::SetFormat::OTHER)
        ui_->lineEdit_formatOther->setVisible(true);
    else
        ui_->lineEdit_formatOther->setVisible(false);
    activeRecordingManager_->setFormat(uh::SetFormat(
        static_cast<uh::SetFormat::Type>(index),
        ui_->lineEdit_formatOther->text().toStdString().c_str()
    ));
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onLineEditFormatChanged(const QString& formatDesc)
{
    activeRecordingManager_->setFormat(uh::SetFormat(uh::SetFormat::OTHER, formatDesc.toStdString().c_str()));
    ui_->comboBox_format->setCurrentIndex(static_cast<int>(uh::SetFormat::OTHER));
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onSpinBoxGameNumberChanged(int value)
{
    activeRecordingManager_->setGameNumber(value);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onLineEditP1TextChanged(const QString& name)
{
    activeRecordingManager_->setP1Name(name);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onLineEditP2TextChanged(const QString& name)
{
    activeRecordingManager_->setP2Name(name);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerConnectedToServer()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_waiting);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerDisconnectedFromServer()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_disconnected);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerRecordingStarted(uh::RunningGameSession* recording)
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
        const uh::String* fighterNameStr = recording->mappingInfo().fighterID.map(recording->playerFighterID(i));
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
    const uh::String* stageStr = recording->mappingInfo().stageID.map(recording->stageID());
    ui_->label_stage->setText(stageStr ? stageStr->cStr() : "(Unknown Stage)");
    ui_->label_date->setText(QDateTime::fromMSecsSinceEpoch(recording->timeStampStartedMs()).toString());
    ui_->label_timeRemaining->setText("");

    // Prepare recording view for new game
    recordingView_->setRecording(recording);

    // Show active page, if not already
    ui_->stackedWidget->setCurrentWidget(ui_->page_active);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerRecordingEnded(uh::RunningGameSession* recording)
{
    (void)recording;
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerP1NameChanged(const QString& name)
{
    if (0 >= names_.size())
        return;

    const QSignalBlocker blocker(ui_->lineEdit_player1);

    names_[0]->setTitle(name);
    ui_->lineEdit_player1->setText(name);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerP2NameChanged(const QString& name)
{
    if (1 >= names_.size())
        return;

    const QSignalBlocker blocker(ui_->lineEdit_player2);

    names_[1]->setTitle(name);
    ui_->lineEdit_player2->setText(name);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerFormatChanged(const uh::SetFormat& format)
{
    const QSignalBlocker blocker1(ui_->comboBox_format);
    const QSignalBlocker blocker2(ui_->lineEdit_formatOther);

    ui_->comboBox_format->setCurrentIndex(static_cast<int>(format.type()));

    if (format.type() == uh::SetFormat::OTHER)
        ui_->lineEdit_formatOther->setText(format.description().cStr());
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerSetNumberChanged(uh::SetNumber number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerGameNumberChanged(uh::GameNumber number)
{
    const QSignalBlocker blocker(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(number);
}

// ----------------------------------------------------------------------------
void RunningGameSessionView::onRunningGameSessionManagerPlayerStateAdded(int player, const uh::PlayerState& state)
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
