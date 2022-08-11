#include "application/ui_ActiveSessionView.h"
#include "application/models/ActiveSessionManager.hpp"
#include "application/views/ActiveSessionView.hpp"
#include "application/views/ReplayViewer.hpp"
#include "application/Util.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Session.hpp"

#include <QVBoxLayout>
#include <QDateTime>

namespace rfapp {

// ----------------------------------------------------------------------------
ActiveSessionView::ActiveSessionView(
        ActiveSessionManager* activeSessionManager,
        PluginManager* pluginManager,
        QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::ActiveSessionView)
    , activeSessionManager_(activeSessionManager)
{
    ui_->setupUi(this);
    ui_->lineEdit_formatOther->setVisible(false);
    ui_->layout_sessionViewer->addWidget(new ReplayViewer(activeSessionManager_->protocol(), pluginManager));

#define X(type, shortstr, longstr) ui_->comboBox_format->addItem(longstr);
    SET_FORMAT_LIST
#undef X

    connect(ui_->comboBox_format, qOverload<int>(&QComboBox::currentIndexChanged), this, &ActiveSessionView::onComboBoxFormatIndexChanged);
    connect(ui_->lineEdit_formatOther, &QLineEdit::textChanged, this, &ActiveSessionView::onLineEditFormatChanged);
    connect(ui_->spinBox_gameNumber, qOverload<int>(&QSpinBox::valueChanged), this, &ActiveSessionView::onSpinBoxGameNumberChanged);
    connect(ui_->lineEdit_player1, &QLineEdit::textChanged, this, &ActiveSessionView::onLineEditP1TextChanged);
    connect(ui_->lineEdit_player2, &QLineEdit::textChanged, this, &ActiveSessionView::onLineEditP2TextChanged);

    activeSessionManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveSessionView::~ActiveSessionView()
{
    activeSessionManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onComboBoxFormatIndexChanged(int index)
{
    if (static_cast<rfcommon::SetFormat::Type>(index) == rfcommon::SetFormat::OTHER)
        ui_->lineEdit_formatOther->setVisible(true);
    else
        ui_->lineEdit_formatOther->setVisible(false);
    activeSessionManager_->setSetFormat(rfcommon::SetFormat(
        static_cast<rfcommon::SetFormat::Type>(index),
        ui_->lineEdit_formatOther->text().toStdString().c_str()
    ));
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onLineEditFormatChanged(const QString& formatDesc)
{
    activeSessionManager_->setSetFormat(rfcommon::SetFormat(rfcommon::SetFormat::OTHER, formatDesc.toStdString().c_str()));
    ui_->comboBox_format->setCurrentIndex(static_cast<int>(rfcommon::SetFormat::OTHER));
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onSpinBoxGameNumberChanged(int value)
{
    activeSessionManager_->setGameNumber(rfcommon::GameNumber::fromValue(value));
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onLineEditP1TextChanged(const QString& name)
{
    activeSessionManager_->setP1Name(name);
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onLineEditP2TextChanged(const QString& name)
{
    activeSessionManager_->setP2Name(name);
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerConnected(const char* ip, uint16_t port)
{
    ui_->label_connection->setText("<html><head/><body><p><span style=\"font-weight:600; color:#00A000;\">Connected to " + QString(ip) + "</span></p></body></html>");
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerDisconnected()
{
    ui_->label_connection->setText("<html><head/><body><p><span style=\"font-weight:600; color:#ff0000;\">Disconnected</span></p></body></html>");
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameStarted(rfcommon::Session* game)
{
    clearLayout(ui_->layout_playerInfo);

    rfcommon::MappingInfo* map = game->tryGetMappingInfo();
    rfcommon::MetaData* mdata = game->tryGetMetaData();

    if (map == nullptr || mdata == nullptr)
        return;

    // Create individual player UIs
    int count = mdata->fighterCount();
    names_.resize(count);
    fighterName_.resize(count);
    fighterDamage_.resize(count);
    fighterStocks_.resize(count);
    for (int i = 0; i != count; ++i)
    {
        fighterName_[i] = new QLabel(map->fighter.toName(mdata->fighterID(i)));

        fighterDamage_[i] = new QLabel();
        fighterStocks_[i] = new QLabel();

        names_[i] = new QGroupBox;
        names_[i]->setTitle(mdata->name(i).cStr());

        QFormLayout* layout = new QFormLayout;
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">Fighter:</span></p></body></html>"),
            fighterName_[i]);
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
    const uint64_t stamp = mdata->timeStarted().millisSinceEpoch();
    ui_->label_stage->setText(map->stage.toName(mdata->stageID()));
    ui_->label_started->setText(QDateTime::fromMSecsSinceEpoch(stamp).toString());
    ui_->label_remaining->setText("--");
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameEnded(rfcommon::Session* game)
{
    ui_->label_stage->setText("--");
    ui_->label_started->setText("--");
    ui_->label_remaining->setText("--");
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingStarted(rfcommon::Session* training)
{
    clearLayout(ui_->layout_playerInfo);
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingEnded(rfcommon::Session* training)
{
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTimeRemainingChanged(double seconds)
{
    ui_->label_remaining->setText(QTime(0, 0).addSecs(static_cast<int>(seconds)).toString());
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerFighterStateChanged(int fighterIdx, float damage, int stocks)
{
    fighterDamage_[fighterIdx]->setText(QString::number(damage, 'f', 1) + "%");
    fighterStocks_[fighterIdx]->setText(QString::number(stocks));
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTimeStartedChanged(rfcommon::TimeStamp timeStarted)
{
    ui_->label_started->setText(QDateTime::fromMSecsSinceEpoch(timeStarted.millisSinceEpoch()).toString());
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTimeEndedChanged(rfcommon::TimeStamp timeEnded)
{
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerPlayerNameChanged(int playerIdx, const rfcommon::String& name)
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
void ActiveSessionView::onActiveSessionManagerSetNumberChanged(rfcommon::SetNumber number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameNumberChanged(rfcommon::GameNumber number)
{
    const QSignalBlocker blocker(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(number.value());
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerSetFormatChanged(const rfcommon::SetFormat& format)
{
    const QSignalBlocker blocker1(ui_->comboBox_format);
    const QSignalBlocker blocker2(ui_->lineEdit_formatOther);

    ui_->comboBox_format->setCurrentIndex(format.index());

    if (format.type() == rfcommon::SetFormat::OTHER)
        ui_->lineEdit_formatOther->setText(format.longDescription());
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerWinnerChanged(int winner)
{
    for (int i = 0; i != names_.size(); ++i)
    {
        if (i == winner)
            names_[i]->setStyleSheet("background-color: rgb(211, 249, 216)");
        else
            names_[i]->setStyleSheet("background-color: rgb(249, 214, 211)");
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingSessionNumberChanged(rfcommon::GameNumber number)
{
}

}
