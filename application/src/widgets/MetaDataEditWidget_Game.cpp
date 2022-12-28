#include "application/ui_MetaDataEditWidget_Game.h"
#include "application/models/MetaDataEditModel.hpp"
#include "application/models/PlayerDetails.hpp"
#include "application/widgets/MetaDataEditWidget_Game.hpp"

#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/TrainingMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/SessionNumber.hpp"
#include "rfcommon/Round.hpp"
#include "rfcommon/SetFormat.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::MetaDataEditWidget_Game(MetaDataEditModel* model, PlayerDetails* playerDetails, QWidget* parent)
    : MetaDataEditWidget(model, parent)
    , playerDetails_(playerDetails)
    , ui_(new Ui::MetaDataEditWidget_Game)
{
    ui_->setupUi(contentWidget());
    setTitle("Game");

#define X(type, shortstr, longstr) if (strlen(shortstr) > 0) ui_->comboBox_roundType->addItem(longstr);
    ROUND_TYPES_LIST
#undef X
#define X(type, shortstr, longstr) ui_->comboBox_setFormat->addItem(longstr);
    SET_FORMAT_LIST
#undef X
    ui_->comboBox_setFormat->setCurrentIndex(rfcommon::SetFormat::FREE);

    ui_->pushButton_resetTimeStarted->setVisible(false);
    ui_->comboBox_roundType->setVisible(false);
    ui_->checkBox_leftL->setVisible(false);
    ui_->checkBox_rightL->setVisible(false);

    updateSize();

    connect(ui_->dateTimeEdit_started, &QDateTimeEdit::dateTimeChanged, this, &MetaDataEditWidget_Game::onDateTimeStartedChanged);
    connect(ui_->pushButton_resetTimeStarted, &QPushButton::released, this, &MetaDataEditWidget_Game::onPushButtonResetTimeStartedReleased);
    connect(ui_->comboBox_roundType, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetaDataEditWidget_Game::onComboBoxRoundTypeChanged);
    connect(ui_->spinBox_roundNumber, qOverload<int>(&QSpinBox::valueChanged), this, &MetaDataEditWidget_Game::onSpinBoxRoundNumberChanged);
    connect(ui_->comboBox_setFormat, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetaDataEditWidget_Game::onComboBoxSetFormatChanged);
    connect(ui_->spinBox_gameNumber, qOverload<int>(&QSpinBox::valueChanged), this, &MetaDataEditWidget_Game::onSpinBoxGameNumberChanged);
    connect(ui_->checkBox_leftL, &QCheckBox::stateChanged, this, &MetaDataEditWidget_Game::onCheckBoxLeftLoserSideChanged);
    connect(ui_->checkBox_rightL, &QCheckBox::stateChanged, this, &MetaDataEditWidget_Game::onCheckBoxRightLoserSideChanged);
    connect(ui_->lineEdit_leftName, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditLeftNameChanged);
    connect(ui_->lineEdit_rightName, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditRightNameChanged);
    connect(ui_->lineEdit_leftSponsor, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditLeftSponsorChanged);
    connect(ui_->lineEdit_rightSponsor, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditRightSponsorChanged);
    connect(ui_->lineEdit_leftSocial, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditLeftSocialChanged);
    connect(ui_->lineEdit_rightSocial, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditRightSocialChanged);
    connect(ui_->lineEdit_leftPronouns, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditLeftPronounsChanged);
    connect(ui_->lineEdit_rightPronouns, &QLineEdit::textChanged, this, &MetaDataEditWidget_Game::onLineEditRightPronounsChanged);
    connect(ui_->toolButton_incLeftScore, &QToolButton::released, this, &MetaDataEditWidget_Game::onPushButtonIncLeftScoreReleased);
    connect(ui_->toolButton_decLeftScore, &QToolButton::released, this, &MetaDataEditWidget_Game::onPushButtonDecLeftScoreReleased);
    connect(ui_->toolButton_incRightScore, &QToolButton::released, this, &MetaDataEditWidget_Game::onPushButtonIncRightScoreReleased);
    connect(ui_->toolButton_decRightScore, &QToolButton::released, this, &MetaDataEditWidget_Game::onPushButtonDecRightScoreReleased);
    connect(ui_->pushButton_resetScore, &QPushButton::released, [this] { ui_->spinBox_gameNumber->setValue(1); });
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::~MetaDataEditWidget_Game()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onDateTimeStartedChanged(const QDateTime& dateTime)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        mdata->setTimeStarted(rfcommon::TimeStamp::fromMillisSinceEpoch(dateTime.toMSecsSinceEpoch()));
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonResetTimeStartedReleased()
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onComboBoxRoundTypeChanged(int index)
{
    updateRoundTypeUI(index);
    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setRound(rfcommon::Round::fromIndex(index, g->round().number()));
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onSpinBoxRoundNumberChanged(int value)
{
    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setRound(rfcommon::Round::fromType(
                    g->round().type(),
                    rfcommon::SessionNumber::fromValue(value)));
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onComboBoxSetFormatChanged(int index)
{
    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setSetFormat(rfcommon::SetFormat::fromIndex(index));
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onSpinBoxGameNumberChanged(int value)
{
    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt();
    int rightScore = ui_->label_rightScore->text().toInt();
    int expectedGameNumber = 1 + leftScore + rightScore;

    while (expectedGameNumber > value)
    {
        if (leftScore > rightScore)
            leftScore--;
        else
            rightScore--;
        expectedGameNumber = 1 + leftScore + rightScore;
    }

    ui_->label_leftScore->setText(QString::number(leftScore));
    ui_->label_rightScore->setText(QString::number(rightScore));

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(value));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onCheckBoxLeftLoserSideChanged(bool enable)
{
    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setPlayerIsLoserSide(0, enable);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onCheckBoxRightLoserSideChanged(bool enable)
{
    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setPlayerIsLoserSide(1, enable);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftNameChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerName(0, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(0).cStr(),
                    g->playerName(0).cStr(),
                    g->playerSponsor(0).cStr(),
                    g->playerSocial(0).cStr(),
                    g->playerPronouns(0).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightNameChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerName(1, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(1).cStr(),
                    g->playerName(1).cStr(),
                    g->playerSponsor(1).cStr(),
                    g->playerSocial(1).cStr(),
                    g->playerPronouns(1).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftSponsorChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerSponsor(0, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(0).cStr(),
                    g->playerName(0).cStr(),
                    g->playerSponsor(0).cStr(),
                    g->playerSocial(0).cStr(),
                    g->playerPronouns(0).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightSponsorChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerSponsor(1, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(1).cStr(),
                    g->playerName(1).cStr(),
                    g->playerSponsor(1).cStr(),
                    g->playerSocial(1).cStr(),
                    g->playerPronouns(1).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftSocialChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerSocial(0, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(0).cStr(),
                    g->playerName(0).cStr(),
                    g->playerSponsor(0).cStr(),
                    g->playerSocial(0).cStr(),
                    g->playerPronouns(0).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightSocialChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerSocial(1, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(1).cStr(),
                    g->playerName(1).cStr(),
                    g->playerSponsor(1).cStr(),
                    g->playerSocial(1).cStr(),
                    g->playerPronouns(1).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftPronounsChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerPronouns(0, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(0).cStr(),
                    g->playerName(0).cStr(),
                    g->playerSponsor(0).cStr(),
                    g->playerSocial(0).cStr(),
                    g->playerPronouns(0).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightPronounsChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
        {
            rfcommon::GameMetaData* g = mdata->asGame();
            g->setPlayerPronouns(1, text.toUtf8().constData());
            playerDetails_->addOrModifyPlayer(g->playerTag(1).cStr(),
                    g->playerName(1).cStr(),
                    g->playerSponsor(1).cStr(),
                    g->playerSocial(1).cStr(),
                    g->playerPronouns(1).cStr());
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonIncLeftScoreReleased()
{
    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt() + 1;
    int rightScore = ui_->label_rightScore->text().toInt();
    int gameNumber = 1 + leftScore + rightScore;

    bool store = ui_->spinBox_gameNumber->blockSignals(true);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_leftScore->setText(QString::number(leftScore));
    ui_->spinBox_gameNumber->blockSignals(store);

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonDecLeftScoreReleased()
{
    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt() - 1;
    int rightScore = ui_->label_rightScore->text().toInt();
    if (leftScore < 0)
        leftScore = 0;
    int gameNumber = 1 + leftScore + rightScore;

    bool store = ui_->spinBox_gameNumber->blockSignals(true);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_leftScore->setText(QString::number(leftScore));
    ui_->spinBox_gameNumber->blockSignals(store);

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonIncRightScoreReleased()
{
    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt();
    int rightScore = ui_->label_rightScore->text().toInt() + 1;
    int gameNumber = 1 + leftScore + rightScore;

    bool store = ui_->spinBox_gameNumber->blockSignals(true);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_rightScore->setText(QString::number(rightScore));
    ui_->spinBox_gameNumber->blockSignals(store);

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonDecRightScoreReleased()
{
    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt();
    int rightScore = ui_->label_rightScore->text().toInt() - 1;
    if (rightScore < 0)
        rightScore = 0;
    int gameNumber = 1 + leftScore + rightScore;

    bool store = ui_->spinBox_gameNumber->blockSignals(true);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_rightScore->setText(QString::number(rightScore));
    ui_->spinBox_gameNumber->blockSignals(store);

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::updateRoundTypeUI(int roundTypeIndex)
{
    enableGrandFinalOptions(
            roundTypeIndex == rfcommon::Round::GRAND_FINALS);
    enableRoundCounter(
            roundTypeIndex == rfcommon::Round::WINNERS_ROUND ||
            roundTypeIndex == rfcommon::Round::LOSERS_ROUND ||
            roundTypeIndex == rfcommon::Round::POOLS);

    updateSize();
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::enableRoundCounter(bool enable)
{
    ui_->spinBox_roundNumber->setVisible(enable);
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::enableFreePlayOption(bool enable)
{
    QSignalBlocker blockSetFormat(ui_->comboBox_setFormat);

    if (enable)
    {
        // Fill in all options, including free play
        QComboBox* cb = ui_->comboBox_setFormat;
        int saveIdx = cb->currentIndex();
        cb->clear();
#define X(name, shortstr, longstr) cb->addItem(longstr);
        SET_FORMAT_LIST
#undef X
        cb->setCurrentIndex(saveIdx);
    }
    else
    {
        // Remove free play option from set format dropdown
        QComboBox* cb = ui_->comboBox_setFormat;
        int saveIdx = cb->currentIndex();
        cb->clear();
#define X(name, shortstr, longstr) if (rfcommon::SetFormat::name != rfcommon::SetFormat::FREE) cb->addItem(longstr);
        SET_FORMAT_LIST
#undef X
        cb->setCurrentIndex(saveIdx);
    }
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::enableRoundTypeSelection(bool enable)
{
    ui_->comboBox_roundType->setVisible(enable);
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::enableGrandFinalOptions(bool enable)
{
    ui_->checkBox_leftL->setVisible(enable);
    ui_->checkBox_rightL->setVisible(enable);
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{
    rfcommon::TimeStamp::Type timeStartedEpoch = 0, timeEndedEpoch = 0;
    QString stageName, leftName, rightName;
    int stageID = -1, roundTypeIdx = -1, roundNumber = -1, setFormatIdx = -1, gameNumber = -1, lScore = -1, rScore = -1;
    QStringList names, sponsors, socials, pronouns;

    bool first = true;
    for (int i = 0; i != mdata.count(); ++i)
    {
        switch (mdata[i]->type())
        {
            case rfcommon::MetaData::GAME: {
                rfcommon::GameMetaData* g = mdata[i]->asGame();

                if (first)
                {
                    timeStartedEpoch = g->timeStarted().millisSinceEpoch();
                    timeEndedEpoch = g->timeEnded().millisSinceEpoch();
                    stageID = g->stageID().value();
                    stageName = QString::fromUtf8(map[i]->stage.toName(g->stageID()));
                    roundTypeIdx = g->round().index();
                    roundNumber = g->round().number().value();
                    setFormatIdx = g->setFormat().index();
                    gameNumber = g->score().gameNumber().value();
                    lScore = g->score().left();
                    rScore = g->score().right();

                    if (g->fighterCount() == 2)  // 1v1
                    {
                        leftName = QString::fromUtf8(g->playerTag(0).cStr());
                        rightName = QString::fromUtf8(g->playerTag(1).cStr());
                        for (int p = 0; p != 2; ++p)
                        {
                            names += QString::fromUtf8(g->playerName(p).cStr());
                            sponsors += QString::fromUtf8(g->playerSponsor(p).cStr());
                            socials += QString::fromUtf8(g->playerSocial(p).cStr());
                            pronouns += QString::fromUtf8(g->playerPronouns(p).cStr());
                        }
                    }
                }
                else
                {
                    if (timeStartedEpoch != g->timeStarted().millisSinceEpoch())
                        timeStartedEpoch = 0;
                    if (timeEndedEpoch != g->timeEnded().millisSinceEpoch())
                        timeEndedEpoch = 0;
                    if (stageID != g->stageID().value())
                    {
                        stageID = -1;
                        stageName = "*";
                    }
                    if (roundTypeIdx != g->round().index())
                        roundTypeIdx = -1;
                    if (roundNumber != g->round().number().value())
                        roundNumber = -1;
                    if (setFormatIdx != g->setFormat().index())
                        setFormatIdx = -1;
                    if (gameNumber != g->score().gameNumber().value())
                        gameNumber = -1;
                    if (lScore != g->score().left())
                        lScore = -1;
                    if (rScore != g->score().right())
                        rScore = -1;

                    if (g->fighterCount() == 2 && names.size() == 2)  // 1v1
                    {
                        if (leftName != QString::fromUtf8(g->playerTag(0).cStr()))
                            leftName = "Player 1";
                        if (rightName != QString::fromUtf8(g->playerTag(1).cStr()))
                            rightName = "Player 2";
                        for (int p = 0; p != 2; ++p)
                        {
                            if (names[p] != QString::fromUtf8(g->playerName(p).cStr()))
                                names[p] = "*";
                            if (sponsors[p] != QString::fromUtf8(g->playerSponsor(p).cStr()))
                                sponsors[p] = "*";
                            if (socials[p] != QString::fromUtf8(g->playerSocial(p).cStr()))
                                socials[p] = "*";
                            if (pronouns[p] != QString::fromUtf8(g->playerPronouns(p).cStr()))
                                pronouns[p] = "*";
                        }
                    }
                }

            } break;

            case rfcommon::MetaData::TRAINING:
                break;
        }
        first = false;
    }

    // We do not want to trigger any signals when modifying the UI, because
    // most of these try to update the model
    QSignalBlocker blockTimeStarted(ui_->dateTimeEdit_started);
    QSignalBlocker blockTimeEnded(ui_->dateTimeEdit_ended);
    QSignalBlocker blockStageID(ui_->lineEdit_stageID);
    QSignalBlocker blockStageName(ui_->lineEdit_stageName);
    QSignalBlocker blockRoundType(ui_->comboBox_roundType);
    QSignalBlocker blockRoundNumber(ui_->spinBox_roundNumber);
    QSignalBlocker blockSetFormat(ui_->comboBox_setFormat);
    QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);

    QSignalBlocker blockLName(ui_->lineEdit_leftName);
    QSignalBlocker blockLSponsor(ui_->lineEdit_leftSponsor);
    QSignalBlocker blockLSocial(ui_->lineEdit_leftSocial);
    QSignalBlocker blockLPronouns(ui_->lineEdit_leftPronouns);
    QSignalBlocker blockRName(ui_->lineEdit_rightName);
    QSignalBlocker blockRSponsor(ui_->lineEdit_rightSponsor);
    QSignalBlocker blockRSocial(ui_->lineEdit_rightSocial);
    QSignalBlocker blockRPronouns(ui_->lineEdit_rightPronouns);

    ui_->dateTimeEdit_started->setMinimumDateTime(QDateTime::fromMSecsSinceEpoch(0));
    ui_->dateTimeEdit_started->setDateTime(QDateTime::fromMSecsSinceEpoch(0));
    if (timeStartedEpoch == 0)
        ui_->dateTimeEdit_started->setSpecialValueText("*");
    else
        ui_->dateTimeEdit_started->setDateTime(QDateTime::fromMSecsSinceEpoch(timeStartedEpoch));

    ui_->dateTimeEdit_ended->setMinimumDateTime(QDateTime::fromMSecsSinceEpoch(0));
    ui_->dateTimeEdit_ended->setDateTime(QDateTime::fromMSecsSinceEpoch(0));
    if (timeEndedEpoch == 0)
        ui_->dateTimeEdit_ended->setSpecialValueText("*");
    else
        ui_->dateTimeEdit_ended->setDateTime(QDateTime::fromMSecsSinceEpoch(timeEndedEpoch));

    if (stageID == -1)
    {
        ui_->lineEdit_stageID->setText("*");
        ui_->lineEdit_stageName->setText("*");
    }
    else
    {
        ui_->lineEdit_stageID->setText(QString::number(stageID));
        ui_->lineEdit_stageName->setText(stageName);
    }

    if (roundTypeIdx == -1)
    {
        ui_->comboBox_roundType->setCurrentIndex(0);
        ui_->comboBox_roundType->setCurrentText("*");
    }
    else
    {
        ui_->comboBox_roundType->setCurrentIndex(roundTypeIdx);
        updateRoundTypeUI(roundTypeIdx);
    }

    if (roundNumber == -1)
    {
        ui_->spinBox_roundNumber->setMinimum(0);
        ui_->spinBox_roundNumber->setSpecialValueText("*");
    }
    else
        ui_->spinBox_roundNumber->setValue(roundNumber);

    if (setFormatIdx == -1)
    {
        ui_->comboBox_setFormat->setPlaceholderText("*");
        ui_->comboBox_setFormat->setCurrentIndex(-1);
    }
    else
        ui_->comboBox_setFormat->setCurrentIndex(setFormatIdx);

    if (gameNumber == -1)
    {
        ui_->spinBox_gameNumber->setMinimum(0);
        ui_->spinBox_gameNumber->setSpecialValueText("*");
    }
    else
        ui_->spinBox_gameNumber->setValue(gameNumber);

    if (lScore == -1)
        ui_->label_leftScore->setText("*");
    else
        ui_->label_leftScore->setText(QString::number(lScore));

    if (rScore == -1)
        ui_->label_rightScore->setText("*");
    else
        ui_->label_rightScore->setText(QString::number(rScore));

    if (names.size() == 2)  // 1v1
    {
        ui_->label_leftName->setText(leftName);
        ui_->label_rightName->setText(rightName);

        ui_->lineEdit_leftName->setText(names[0]);
        ui_->lineEdit_leftSponsor->setText(sponsors[0]);
        ui_->lineEdit_leftSocial->setText(socials[0]);
        ui_->lineEdit_leftPronouns->setText(pronouns[0]);

        ui_->lineEdit_rightName->setText(names[1]);
        ui_->lineEdit_rightSponsor->setText(sponsors[1]);
        ui_->lineEdit_rightSocial->setText(socials[1]);
        ui_->lineEdit_rightPronouns->setText(pronouns[1]);
    }
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{
    assert(map.count() == mdata.count());

    ignoreSelf_ = true;

    for (int i = 0; i != map.count(); ++i)
    {
        switch (mdata[i]->type())
        {
            case rfcommon::MetaData::GAME: {
                rfcommon::GameMetaData* g = mdata[i]->asGame();

                // We make an exception with time started/ended and with stage
                // ID, because these are values that can't be edited by the user
                // and are set by the game. Adopt these instead.
                const QSignalBlocker blockDateStarted(ui_->dateTimeEdit_started);
                const QSignalBlocker blockDateEnded(ui_->dateTimeEdit_ended);
                const QSignalBlocker blockStageID(ui_->lineEdit_stageID);
                const QSignalBlocker blockStageName(ui_->lineEdit_stageName);
                ui_->dateTimeEdit_started->setDateTime(QDateTime::fromMSecsSinceEpoch(
                        g->timeStarted().millisSinceEpoch()));
                ui_->dateTimeEdit_ended->setDateTime(QDateTime::fromMSecsSinceEpoch(
                        g->timeEnded().millisSinceEpoch()));
                ui_->lineEdit_stageID->setText(QString::number(g->stageID().value()));
                ui_->lineEdit_stageName->setText(QString::fromUtf8(map[i]->stage.toName(g->stageID())));

                g->setRound(rfcommon::Round::fromIndex(
                        ui_->comboBox_roundType->currentIndex(),
                        rfcommon::SessionNumber::fromValue(ui_->spinBox_roundNumber->value())));
                g->setSetFormat(rfcommon::SetFormat::fromIndex(
                        ui_->comboBox_setFormat->currentIndex()));

                if (mdata[i]->fighterCount() == 2)  // 1v1
                {
                    g->setScore(rfcommon::ScoreCount::fromScoreAndGameNumber(
                            ui_->label_leftScore->text().toInt(),
                            ui_->label_rightScore->text().toInt(),
                            rfcommon::GameNumber::fromValue(ui_->spinBox_gameNumber->value())));

                    const QSignalBlocker blockLName(ui_->lineEdit_leftName);
                    const QSignalBlocker blockLSponsor(ui_->lineEdit_leftSponsor);
                    const QSignalBlocker blockLSocial(ui_->lineEdit_leftSocial);
                    const QSignalBlocker blockLPronouns(ui_->lineEdit_leftPronouns);
                    const QSignalBlocker blockRName(ui_->lineEdit_rightName);
                    const QSignalBlocker blockRSponsor(ui_->lineEdit_rightSponsor);
                    const QSignalBlocker blockRSocial(ui_->lineEdit_rightSocial);
                    const QSignalBlocker blockRPronouns(ui_->lineEdit_rightPronouns);

                    // The player's tag is set by the game and cannot be changed
                    // by the user. If it doesn't contain a valid tag -- either
                    // because the player hasn't chosen one, or if the game is
                    // being replayed instead of being played live -- we simply
                    // copy the data in the UI over into the metadata object.
                    //
                    // If there is a valid tag, we can try to fetch the player
                    // details if this player has played before.
                    const PlayerDetails::Player defaultPlayer{"", "", "", "he/him"};
                    const PlayerDetails::Player* p;
                    p = playerDetails_->findTag(g->playerTag(0));
                    if (p == nullptr)
                        if (playerDetails_->findName(ui_->lineEdit_leftName->text().toUtf8().constData()) != nullptr)
                            p = &defaultPlayer;
                    if (p)
                    {
                        g->setPlayerName(0, p->name.cStr());
                        g->setPlayerSponsor(0, p->sponsor.cStr());
                        g->setPlayerSocial(0, p->social.cStr());
                        g->setPlayerPronouns(0, p->pronouns.cStr());

                        ui_->lineEdit_leftName->setText(QString::fromUtf8(p->name.cStr()));
                        ui_->lineEdit_leftSponsor->setText(QString::fromUtf8(p->sponsor.cStr()));
                        ui_->lineEdit_leftSocial->setText(QString::fromUtf8(p->social.cStr()));
                        ui_->lineEdit_leftPronouns->setText(QString::fromUtf8(p->pronouns.cStr()));
                    }
                    else
                    {
                        g->setPlayerName(0, ui_->lineEdit_leftName->text().toUtf8().constData());
                        g->setPlayerSponsor(0, ui_->lineEdit_leftSponsor->text().toUtf8().constData());
                        g->setPlayerSocial(0, ui_->lineEdit_leftSocial->text().toUtf8().constData());
                        g->setPlayerPronouns(0, ui_->lineEdit_leftPronouns->text().toUtf8().constData());

                        playerDetails_->addOrModifyPlayer(g->playerTag(0).cStr(),
                                g->playerName(0).cStr(),
                                g->playerSponsor(0).cStr(),
                                g->playerSocial(0).cStr(),
                                g->playerPronouns(0).cStr());
                    }

                    p = playerDetails_->findTag(g->playerTag(1));
                    if (p == nullptr)
                        if (playerDetails_->findName(ui_->lineEdit_rightName->text().toUtf8().constData()) != nullptr)
                            p = &defaultPlayer;
                    if (p)
                    {
                        g->setPlayerName(1, p->name.cStr());
                        g->setPlayerSponsor(1, p->sponsor.cStr());
                        g->setPlayerSocial(1, p->social.cStr());
                        g->setPlayerPronouns(1, p->pronouns.cStr());

                        ui_->lineEdit_rightName->setText(QString::fromUtf8(p->name.cStr()));
                        ui_->lineEdit_rightSponsor->setText(QString::fromUtf8(p->sponsor.cStr()));
                        ui_->lineEdit_rightSocial->setText(QString::fromUtf8(p->social.cStr()));
                        ui_->lineEdit_rightPronouns->setText(QString::fromUtf8(p->pronouns.cStr()));
                    }
                    else
                    {
                        g->setPlayerName(1, ui_->lineEdit_rightName->text().toUtf8().constData());
                        g->setPlayerSponsor(1, ui_->lineEdit_rightSponsor->text().toUtf8().constData());
                        g->setPlayerSocial(1, ui_->lineEdit_rightSocial->text().toUtf8().constData());
                        g->setPlayerPronouns(1, ui_->lineEdit_rightPronouns->text().toUtf8().constData());

                        playerDetails_->addOrModifyPlayer(g->playerTag(0).cStr(),
                                g->playerName(0).cStr(),
                                g->playerSponsor(0).cStr(),
                                g->playerSocial(0).cStr(),
                                g->playerPronouns(0).cStr());
                    }

                    // If the game counter or score isn't in range of the set format,
                    // try to correct
                    int maxGames = std::numeric_limits<int>::max();
                    int maxScore = std::numeric_limits<int>::max();
                    switch (g->setFormat().type())
                    {
                        case rfcommon::SetFormat::BO3  : maxGames = 3;  maxScore = 2;  break;
                        case rfcommon::SetFormat::BO5  : maxGames = 5;  maxScore = 3;  break;
                        case rfcommon::SetFormat::BO7  : maxGames = 7;  maxScore = 4;  break;
                        case rfcommon::SetFormat::FT5  : maxGames = 10; maxScore = 5;  break;
                        case rfcommon::SetFormat::FT10 : maxGames = 20; maxScore = 10; break;
                        case rfcommon::SetFormat::FREE : break;
                    }
                    if (g->score().gameNumber().value() > maxGames || g->score().left() > maxScore || g->score().right() > maxScore)
                    {
                        g->setScore(rfcommon::ScoreCount::fromScore(0, 0));

                        const QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);
                        ui_->label_leftScore->setText("0");
                        ui_->label_rightScore->setText("0");
                        ui_->spinBox_gameNumber->setValue(1);
                    }
                }
            } break;

            case rfcommon::MetaData::TRAINING: {
            } break;
        }
    }

    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata)
{
    playerDetails_->save();
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onNextGameStarted()
{
    auto allTagsMatch = [](rfcommon::MetaData* a, rfcommon::MetaData* b) -> bool {
        assert(a->fighterCount() == b->fighterCount());
        for (int i = 0; i != a->fighterCount(); ++i)
            if (a->playerTag(i) != b->playerTag(i))
                return false;
        return true;
    };

    auto allFightersMatch = [](rfcommon::MetaData* a, rfcommon::MetaData* b) -> bool {
        assert(a->fighterCount() == b->fighterCount());
        for (int i = 0; i != a->fighterCount(); ++i)
            if (a->playerFighterID(i) != b->playerFighterID(i))
                return false;
        return true;
    };

    auto advanceRound = [](rfcommon::GameMetaData* mdata, rfcommon::GameMetaData* prev) {
        switch (prev->round().type())
        {
            using namespace rfcommon;

            case Round::WINNERS_ROUND:
            case Round::LOSERS_ROUND:
            case Round::POOLS:
            case Round::FREE:
                mdata->setRound(Round::fromType(
                        prev->round().type(),
                        prev->round().number() + 1));
                break;
            case Round::WINNERS_QUARTER:
                mdata->setRound(Round::fromType(Round::WINNERS_SEMI));
                break;
            case Round::WINNERS_SEMI:
                mdata->setRound(Round::fromType(Round::WINNERS_FINALS));
                break;
            case Round::WINNERS_FINALS:
                mdata->setRound(Round::fromType(Round::GRAND_FINALS));
                mdata->setPlayerIsLoserSide(prev->winner(), false);
                mdata->setPlayerIsLoserSide(1 - prev->winner(), true);
                break;
            case Round::LOSERS_QUARTER:
                mdata->setRound(Round::fromType(Round::LOSERS_SEMI));
                break;
            case Round::LOSERS_SEMI:
                mdata->setRound(Round::fromType(Round::LOSERS_FINALS));
                break;
            case Round::LOSERS_FINALS:
                mdata->setRound(Round::fromType(Round::GRAND_FINALS));
                mdata->setPlayerIsLoserSide(prev->winner(), true);
                mdata->setPlayerIsLoserSide(1 - prev->winner(), false);
                break;
            case Round::GRAND_FINALS:
                mdata->setPlayerIsLoserSide(0, true);
                mdata->setPlayerIsLoserSide(1, true);
                break;
        }

        mdata->setScore(rfcommon::ScoreCount::fromScore(0, 0));
    };

    auto advanceScores = [&advanceRound](rfcommon::GameMetaData* mdata, rfcommon::GameMetaData* prev) {
        // Update score based on who won the previous game
        int p1 = prev->score().left();
        int p2 = prev->score().right();
        if (prev->winner() == 0)
            p1++;
        else
            p2++;

        // Check if this game starts a new round
        int maxScore = -1;
        switch (prev->setFormat().type())
        {
            case rfcommon::SetFormat::BO3  : maxScore = 2;  break;
            case rfcommon::SetFormat::BO5  : maxScore = 3;  break;
            case rfcommon::SetFormat::BO7  : maxScore = 4;  break;
            case rfcommon::SetFormat::FT5  : maxScore = 5;  break;
            case rfcommon::SetFormat::FT10 : maxScore = 10; break;
            case rfcommon::SetFormat::FREE : break;
        }
        if (maxScore > -1 && (p1 >= maxScore || p2 >= maxScore))
            advanceRound(mdata, prev);
        else
            mdata->setScore(rfcommon::ScoreCount::fromScore(p1, p2));
    };

    rfcommon::MetaData* mdata = model_->metaData()[0];
    rfcommon::MetaData* prev = model_->prevMetaData();

    ignoreSelf_ = true;
    switch (model_->prevMetaData()->type())
    {
        case rfcommon::MetaData::GAME: {
            if (mdata->type() == prev->type() &&
                allTagsMatch(mdata, prev) &&
                allFightersMatch(mdata, prev) &&
                mdata->asGame()->setFormat() == prev->asGame()->setFormat() &&
                mdata->asGame()->bracketType() == prev->asGame()->bracketType() &&
                mdata->asGame()->round() == prev->asGame()->round())
            {
                advanceScores(mdata->asGame(), prev->asGame());
            }
            else
            {
                advanceRound(mdata->asGame(), prev->asGame());
            }

            // Update UI with possibly new values
            const QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);
            ui_->spinBox_gameNumber->setValue(mdata->asGame()->score().gameNumber().value());
            ui_->label_leftScore->setText(QString::number(mdata->asGame()->score().left()));
            ui_->label_rightScore->setText(QString::number(mdata->asGame()->score().right()));

            const QSignalBlocker blockRoundType(ui_->comboBox_roundType);
            const QSignalBlocker blockRoundNumber(ui_->spinBox_roundNumber);
            updateRoundTypeUI(mdata->asGame()->round().index());
            ui_->comboBox_roundType->setCurrentIndex(mdata->asGame()->round().index());
            ui_->spinBox_roundNumber->setValue(mdata->asGame()->round().number().value());
        } break;

        case rfcommon::MetaData::TRAINING: {
            if (mdata->type() == prev->type())
                mdata->asTraining()->setSessionNumber(prev->asTraining()->sessionNumber() + 1);
        } break;
    }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onBracketTypeChangedUI(rfcommon::BracketType bracketType)
{
    switch (bracketType.type())
    {
        case rfcommon::BracketType::SINGLES:
        case rfcommon::BracketType::DOUBLES:
        case rfcommon::BracketType::SIDE:
        case rfcommon::BracketType::AMATEURS:
            enableFreePlayOption(false);
            enableRoundTypeSelection(true);
            enableGrandFinalOptions(ui_->comboBox_roundType->currentIndex() == rfcommon::Round::GRAND_FINALS);
            enableRoundCounter(
                        ui_->comboBox_roundType->currentIndex() == rfcommon::Round::WINNERS_ROUND ||
                        ui_->comboBox_roundType->currentIndex() == rfcommon::Round::LOSERS_ROUND ||
                        ui_->comboBox_roundType->currentIndex() == rfcommon::Round::POOLS);
            break;

        case rfcommon::BracketType::MONEYMATCH:
        case rfcommon::BracketType::PRACTICE:
        case rfcommon::BracketType::FRIENDLIES:
        case rfcommon::BracketType::OTHER:
            enableFreePlayOption(true);
            enableRoundTypeSelection(false);
            enableGrandFinalOptions(false);
            enableRoundCounter(true);
            break;
    }

    switch (bracketType.type())
    {
        case rfcommon::BracketType::SINGLES:
        case rfcommon::BracketType::DOUBLES:
        case rfcommon::BracketType::SIDE:
        case rfcommon::BracketType::AMATEURS:
            ui_->comboBox_setFormat->setCurrentIndex(rfcommon::SetFormat::BO3);
            break;

        case rfcommon::BracketType::MONEYMATCH:
            ui_->comboBox_setFormat->setCurrentIndex(rfcommon::SetFormat::BO5);
            break;

        case rfcommon::BracketType::PRACTICE:
        case rfcommon::BracketType::FRIENDLIES:
        case rfcommon::BracketType::OTHER:
            ui_->comboBox_setFormat->setCurrentIndex(rfcommon::SetFormat::FREE);
            break;
    }

    updateSize();
}

void MetaDataEditWidget_Game::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Game::onMetaDataTournamentDetailsChanged() {}
void MetaDataEditWidget_Game::onMetaDataEventDetailsChanged()
{
    if (ignoreSelf_)
        return;
    onAdoptMetaData(model_->mappingInfo(), model_->metaData());
}
void MetaDataEditWidget_Game::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_Game::onMetaDataGameDetailsChanged()
{
    if (ignoreSelf_)
        return;
    onAdoptMetaData(model_->mappingInfo(), model_->metaData());
}
void MetaDataEditWidget_Game::onMetaDataPlayerDetailsChanged()
{
    if (ignoreSelf_)
        return;
    onAdoptMetaData(model_->mappingInfo(), model_->metaData());
}
void MetaDataEditWidget_Game::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Game::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
