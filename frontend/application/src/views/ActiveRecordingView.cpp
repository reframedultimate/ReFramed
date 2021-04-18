#include "application/Util.hpp"
#include "application/ui_ActiveRecordingView.h"
#include "application/views/ActiveRecordingView.hpp"
#include "application/views/RecordingView.hpp"
#include "application/models/ActiveRecordingManager.hpp"
#include "application/models/ActiveRecording.hpp"

#include <QVBoxLayout>

namespace uh {

// ----------------------------------------------------------------------------
ActiveRecordingView::ActiveRecordingView(ActiveRecordingManager* manager, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::ActiveRecordingView)
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

    connect(activeRecordingManager_, SIGNAL(connectedToServer()), this, SLOT(onActiveRecordingManagerConnectedToServer()));
    connect(activeRecordingManager_, SIGNAL(disconnectedFromServer()), this, SLOT(onActiveRecordingManagerDisconnectedFromServer()));

    activeRecordingManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveRecordingView::~ActiveRecordingView()
{
    activeRecordingManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onComboBoxFormatIndexChanged(int index)
{
    if (static_cast<SetFormat::Type>(index) == SetFormat::OTHER)
        ui_->lineEdit_formatOther->setVisible(true);
    else
        ui_->lineEdit_formatOther->setVisible(false);
    activeRecordingManager_->setFormat(SetFormat(static_cast<SetFormat::Type>(index), ui_->lineEdit_formatOther->text()));
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onLineEditFormatChanged(const QString& formatDesc)
{
    activeRecordingManager_->setFormat(SetFormat(SetFormat::OTHER, formatDesc));
    ui_->comboBox_format->setCurrentIndex(static_cast<int>(SetFormat::OTHER));
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onSpinBoxGameNumberChanged(int value)
{
    activeRecordingManager_->setGameNumber(value);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onLineEditP1TextChanged(const QString& name)
{
    activeRecordingManager_->setP1Name(name);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onLineEditP2TextChanged(const QString& name)
{
    activeRecordingManager_->setP2Name(name);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerConnectedToServer()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_waiting);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerDisconnectedFromServer()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_disconnected);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerRecordingStarted(ActiveRecording* recording)
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
        const QString* fighterNameStr = recording->mappingInfo().fighterID.map(recording->playerFighterID(i));
        fighterName_[i]->setText(fighterNameStr ? *fighterNameStr : "(Unknown Fighter)");

        fighterStatus_[i] = new QLabel();
        fighterDamage_[i] = new QLabel();
        fighterStocks_[i] = new QLabel();

        names_[i] = new QGroupBox;
        names_[i]->setTitle(recording->playerName(i));

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
    const QString* stageStr = recording->mappingInfo().stageID.map(recording->stageID());
    ui_->label_stage->setText(stageStr ? *stageStr : "(Unknown Stage)");
    ui_->label_date->setText(recording->timeStarted().toString());
    ui_->label_timeRemaining->setText("");

    // Prepare recording view for new game
    recordingView_->setRecording(recording);

    // Show active page, if not already
    ui_->stackedWidget->setCurrentWidget(ui_->page_active);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerRecordingEnded(ActiveRecording* recording)
{
    (void)recording;
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerP1NameChanged(const QString& name)
{
    if (0 >= names_.size())
        return;

    const QSignalBlocker blocker(ui_->lineEdit_player1);

    names_[0]->setTitle(name);
    ui_->lineEdit_player1->setText(name);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerP2NameChanged(const QString& name)
{
    if (1 >= names_.size())
        return;

    const QSignalBlocker blocker(ui_->lineEdit_player2);

    names_[1]->setTitle(name);
    ui_->lineEdit_player2->setText(name);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerFormatChanged(const SetFormat& format)
{
    const QSignalBlocker blocker1(ui_->comboBox_format);
    const QSignalBlocker blocker2(ui_->lineEdit_formatOther);

    ui_->comboBox_format->setCurrentIndex(static_cast<int>(format.type()));

    if (format.type() == SetFormat::OTHER)
        ui_->lineEdit_formatOther->setText(format.description());
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerSetNumberChanged(int number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerGameNumberChanged(int number)
{
    const QSignalBlocker blocker2(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(number);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerPlayerStateAdded(int player, const PlayerState& state)
{
    if (player >= fighterStatus_.size())
        return;

    fighterStatus_[player]->setText(QString::number(state.status()));
    fighterDamage_[player]->setText(QString::number(state.damage()));
    fighterStocks_[player]->setText(QString::number(state.stocks()));

    ui_->label_timeRemaining->setText(QTime(0, 0).addSecs(state.frame() / 60.0).toString());
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::onActiveRecordingManagerWinnerChanged(int winner)
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
