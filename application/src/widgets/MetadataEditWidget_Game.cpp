#include "application/ui_MetadataEditWidget_Game.h"
#include "application/models/MetadataEditModel.hpp"
#include "application/models/PlayerDetails.hpp"
#include "application/widgets/MetadataEditWidget_Game.hpp"

#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/TrainingMetadata.hpp"
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
MetadataEditWidget_Game::MetadataEditWidget_Game(MetadataEditModel* model, PlayerDetails* playerDetails, QWidget* parent)
    : MetadataEditWidget(model, parent)
    , playerDetails_(playerDetails)
    , ui_(new Ui::MetadataEditWidget_Game)
{
    ui_->setupUi(contentWidget());
    setTitle(QIcon::fromTheme(""), "Game");

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

    connect(ui_->dateTimeEdit_started, &QDateTimeEdit::dateTimeChanged, this, &MetadataEditWidget_Game::onDateTimeStartedChanged);
    connect(ui_->pushButton_resetTimeStarted, &QPushButton::released, this, &MetadataEditWidget_Game::onPushButtonResetTimeStartedReleased);
    connect(ui_->comboBox_roundType, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetadataEditWidget_Game::onComboBoxRoundTypeChanged);
    connect(ui_->spinBox_roundNumber, qOverload<int>(&QSpinBox::valueChanged), this, &MetadataEditWidget_Game::onSpinBoxRoundNumberChanged);
    connect(ui_->comboBox_setFormat, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetadataEditWidget_Game::onComboBoxSetFormatChanged);
    connect(ui_->spinBox_gameNumber, qOverload<int>(&QSpinBox::valueChanged), this, &MetadataEditWidget_Game::onSpinBoxGameNumberChanged);
    connect(ui_->checkBox_leftL, &QCheckBox::stateChanged, this, &MetadataEditWidget_Game::onCheckBoxLeftLoserSideChanged);
    connect(ui_->checkBox_rightL, &QCheckBox::stateChanged, this, &MetadataEditWidget_Game::onCheckBoxRightLoserSideChanged);
    connect(ui_->lineEdit_leftName, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditLeftNameChanged);
    connect(ui_->lineEdit_rightName, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditRightNameChanged);
    connect(ui_->lineEdit_leftSponsor, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditLeftSponsorChanged);
    connect(ui_->lineEdit_rightSponsor, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditRightSponsorChanged);
    connect(ui_->lineEdit_leftSocial, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditLeftSocialChanged);
    connect(ui_->lineEdit_rightSocial, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditRightSocialChanged);
    connect(ui_->lineEdit_leftPronouns, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditLeftPronounsChanged);
    connect(ui_->lineEdit_rightPronouns, &QLineEdit::textChanged, this, &MetadataEditWidget_Game::onLineEditRightPronounsChanged);
    connect(ui_->toolButton_incLeftScore, &QToolButton::released, this, &MetadataEditWidget_Game::onPushButtonIncLeftScoreReleased);
    connect(ui_->toolButton_decLeftScore, &QToolButton::released, this, &MetadataEditWidget_Game::onPushButtonDecLeftScoreReleased);
    connect(ui_->toolButton_incRightScore, &QToolButton::released, this, &MetadataEditWidget_Game::onPushButtonIncRightScoreReleased);
    connect(ui_->toolButton_decRightScore, &QToolButton::released, this, &MetadataEditWidget_Game::onPushButtonDecRightScoreReleased);
    connect(ui_->pushButton_resetScore, &QPushButton::released, [this] { ui_->spinBox_gameNumber->setValue(1); });
}

// ----------------------------------------------------------------------------
MetadataEditWidget_Game::~MetadataEditWidget_Game()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onDateTimeStartedChanged(const QDateTime& dateTime)
{
    PROFILE(MetadataEditWidget_Game, onDateTimeStartedChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
    {
        QDateTime ended = dateTime.addMSecs(mdata->length().millis());
        mdata->setTimeStarted(rfcommon::TimeStamp::fromMillisSinceEpoch(dateTime.toMSecsSinceEpoch()));
        mdata->setTimeEnded(rfcommon::TimeStamp::fromMillisSinceEpoch(ended.toMSecsSinceEpoch()));

        ui_->dateTimeEdit_ended->setDateTime(ended);
    }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onPushButtonResetTimeStartedReleased()
{
    PROFILE(MetadataEditWidget_Game, onPushButtonResetTimeStartedReleased);

}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onComboBoxRoundTypeChanged(int index)
{
    PROFILE(MetadataEditWidget_Game, onComboBoxRoundTypeChanged);

    updateRoundTypeUI(index);
    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
            g->setRound(rfcommon::Round::fromIndex(index, g->round().number()));
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onSpinBoxRoundNumberChanged(int value)
{
    PROFILE(MetadataEditWidget_Game, onSpinBoxRoundNumberChanged);

    model_->setPendingChanges();

    // Undo changes of special text "*"
    if (ui_->spinBox_roundNumber->minimum() == 0)
    {
        ui_->spinBox_roundNumber->setMinimum(1);
        ui_->spinBox_roundNumber->setSpecialValueText("");
    }

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
            g->setRound(rfcommon::Round::fromType(
                    g->round().type(),
                    rfcommon::SessionNumber::fromValue(value)));
        }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onComboBoxSetFormatChanged(int index)
{
    PROFILE(MetadataEditWidget_Game, onComboBoxSetFormatChanged);

    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setSetFormat(rfcommon::SetFormat::fromIndex(index));
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onSpinBoxGameNumberChanged(int value)
{
    PROFILE(MetadataEditWidget_Game, onSpinBoxGameNumberChanged);

    model_->setPendingChanges();

    // Undo changes of special text "*"
    if (ui_->spinBox_gameNumber->minimum() == 0)
    {
        ui_->spinBox_gameNumber->setMinimum(1);
        ui_->spinBox_gameNumber->setSpecialValueText("");
    }

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
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onCheckBoxLeftLoserSideChanged(bool enable)
{
    PROFILE(MetadataEditWidget_Game, onCheckBoxLeftLoserSideChanged);

    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setPlayerIsLoserSide(0, enable);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onCheckBoxRightLoserSideChanged(bool enable)
{
    PROFILE(MetadataEditWidget_Game, onCheckBoxRightLoserSideChanged);

    model_->setPendingChanges();

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setPlayerIsLoserSide(1, enable);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onLineEditLeftNameChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditLeftNameChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onLineEditRightNameChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditRightNameChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onLineEditLeftSponsorChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditLeftSponsorChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onLineEditRightSponsorChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditRightSponsorChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onLineEditLeftSocialChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditLeftSocialChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onLineEditRightSocialChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditRightSocialChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onLineEditLeftPronounsChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditLeftPronounsChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onLineEditRightPronounsChanged(const QString& text)
{
    PROFILE(MetadataEditWidget_Game, onLineEditRightPronounsChanged);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
        {
            rfcommon::GameMetadata* g = mdata->asGame();
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
void MetadataEditWidget_Game::onPushButtonIncLeftScoreReleased()
{
    PROFILE(MetadataEditWidget_Game, onPushButtonIncLeftScoreReleased);

    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt() + 1;
    int rightScore = ui_->label_rightScore->text().toInt();
    int gameNumber = 1 + leftScore + rightScore;

    const QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_leftScore->setText(QString::number(leftScore));

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onPushButtonDecLeftScoreReleased()
{
    PROFILE(MetadataEditWidget_Game, onPushButtonDecLeftScoreReleased);

    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt() - 1;
    int rightScore = ui_->label_rightScore->text().toInt();
    if (leftScore < 0)
        leftScore = 0;
    int gameNumber = 1 + leftScore + rightScore;

    const QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_leftScore->setText(QString::number(leftScore));

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onPushButtonIncRightScoreReleased()
{
    PROFILE(MetadataEditWidget_Game, onPushButtonIncRightScoreReleased);

    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt();
    int rightScore = ui_->label_rightScore->text().toInt() + 1;
    int gameNumber = 1 + leftScore + rightScore;

    const QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_rightScore->setText(QString::number(rightScore));

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onPushButtonDecRightScoreReleased()
{
    PROFILE(MetadataEditWidget_Game, onPushButtonDecRightScoreReleased);

    model_->setPendingChanges();

    int leftScore = ui_->label_leftScore->text().toInt();
    int rightScore = ui_->label_rightScore->text().toInt() - 1;
    if (rightScore < 0)
        rightScore = 0;
    int gameNumber = 1 + leftScore + rightScore;

    const QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);
    ui_->spinBox_gameNumber->setValue(gameNumber);
    ui_->label_rightScore->setText(QString::number(rightScore));

    auto score = rfcommon::ScoreCount::fromScoreAndGameNumber(
        leftScore, rightScore, rfcommon::GameNumber::fromValue(gameNumber));

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setScore(score);
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::updateRoundTypeUI(int roundTypeIndex)
{
    PROFILE(MetadataEditWidget_Game, updateRoundTypeUI);

    enableGrandFinalOptions(
            roundTypeIndex == rfcommon::Round::GRAND_FINALS);
    enableRoundCounter(
            roundTypeIndex == rfcommon::Round::WINNERS_ROUND ||
            roundTypeIndex == rfcommon::Round::LOSERS_ROUND ||
            roundTypeIndex == rfcommon::Round::POOLS ||
            roundTypeIndex == rfcommon::Round::FREE);

    updateSize();
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::enableRoundCounter(bool enable)
{
    PROFILE(MetadataEditWidget_Game, enableRoundCounter);

    ui_->spinBox_roundNumber->setVisible(enable);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::enableFreePlayOption(bool enable)
{
    PROFILE(MetadataEditWidget_Game, enableFreePlayOption);

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
        cb->setCurrentIndex(saveIdx == rfcommon::SetFormat::FREE ? 0 : saveIdx);
    }
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::enableRoundTypeSelection(bool enable)
{
    PROFILE(MetadataEditWidget_Game, enableRoundTypeSelection);

    ui_->comboBox_roundType->setVisible(enable);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::enableGrandFinalOptions(bool enable)
{
    PROFILE(MetadataEditWidget_Game, enableGrandFinalOptions);

    ui_->checkBox_leftL->setVisible(enable);
    ui_->checkBox_rightL->setVisible(enable);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Game, onAdoptMetadata);

    QString stageName, leftName, rightName;
    int stageID = -1, roundTypeIdx = -1, roundNumber = -1, setFormatIdx = -1, gameNumber = -1, lScore = -1, rScore = -1;
    QStringList names, sponsors, socials, pronouns;

    bool first = true;
    for (int i = 0; i != mdata.count(); ++i)
    {
        switch (mdata[i]->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = mdata[i]->asGame();

                if (first)
                {
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

                    onBracketTypeChangedUI(g->bracketType());
                }
                else
                {
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

            case rfcommon::Metadata::TRAINING:
                break;
        }
        first = false;
    }

    // We do not want to trigger any signals when modifying the UI, because
    // most of these try to update the model
    const QSignalBlocker blockTimeStarted(ui_->dateTimeEdit_started);
    const QSignalBlocker blockTimeEnded(ui_->dateTimeEdit_ended);
    const QSignalBlocker blockStageID(ui_->lineEdit_stageID);
    const QSignalBlocker blockStageName(ui_->lineEdit_stageName);
    const QSignalBlocker blockRoundType(ui_->comboBox_roundType);
    const QSignalBlocker blockRoundNumber(ui_->spinBox_roundNumber);
    const QSignalBlocker blockSetFormat(ui_->comboBox_setFormat);
    const QSignalBlocker blockGameNumber(ui_->spinBox_gameNumber);

    const QSignalBlocker blockLName(ui_->lineEdit_leftName);
    const QSignalBlocker blockLSponsor(ui_->lineEdit_leftSponsor);
    const QSignalBlocker blockLSocial(ui_->lineEdit_leftSocial);
    const QSignalBlocker blockLPronouns(ui_->lineEdit_leftPronouns);
    const QSignalBlocker blockRName(ui_->lineEdit_rightName);
    const QSignalBlocker blockRSponsor(ui_->lineEdit_rightSponsor);
    const QSignalBlocker blockRSocial(ui_->lineEdit_rightSocial);
    const QSignalBlocker blockRPronouns(ui_->lineEdit_rightPronouns);

    // Editing the start and end dates on multiple replays makes no sense
    if (mdata.count() == 1)
    {
        ui_->dateTimeEdit_started->setDateTime(QDateTime::fromMSecsSinceEpoch(mdata[0]->timeStarted().millisSinceEpoch()));
        ui_->dateTimeEdit_ended->setDateTime(QDateTime::fromMSecsSinceEpoch(mdata[0]->timeEnded().millisSinceEpoch()));
    }
    else
    {
        ui_->dateTimeEdit_started->setMinimumDateTime(QDateTime::fromMSecsSinceEpoch(0));
        ui_->dateTimeEdit_started->setDateTime(QDateTime::fromMSecsSinceEpoch(0));
        ui_->dateTimeEdit_started->setSpecialValueText("*");

        ui_->dateTimeEdit_ended->setMinimumDateTime(QDateTime::fromMSecsSinceEpoch(0));
        ui_->dateTimeEdit_ended->setDateTime(QDateTime::fromMSecsSinceEpoch(0));
        ui_->dateTimeEdit_ended->setSpecialValueText("*");

        ui_->dateTimeEdit_started->setReadOnly(true);
    }

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
        ui_->comboBox_roundType->setCurrentIndex(-1);
        ui_->comboBox_roundType->setPlaceholderText("*");
    }
    else
    {
        ui_->comboBox_roundType->setCurrentIndex(roundTypeIdx);
        updateRoundTypeUI(roundTypeIdx);
    }

    if (roundNumber == -1)
    {
        ui_->spinBox_roundNumber->setMinimum(0);
        ui_->spinBox_roundNumber->setValue(0);
        ui_->spinBox_roundNumber->setSpecialValueText("*");
    }
    else
        ui_->spinBox_roundNumber->setValue(roundNumber);

    if (setFormatIdx == -1)
    {
        ui_->comboBox_setFormat->setCurrentIndex(-1);
        ui_->comboBox_setFormat->setPlaceholderText("*");
    }
    else
        ui_->comboBox_setFormat->setCurrentIndex(setFormatIdx);

    if (gameNumber == -1)
    {
        ui_->spinBox_gameNumber->setMinimum(0);
        ui_->spinBox_gameNumber->setValue(0);
        ui_->spinBox_gameNumber->setSpecialValueText("*");
    }
    else
        ui_->spinBox_gameNumber->setValue(gameNumber);

    if (lScore == -1)
        ui_->label_leftScore->setText("-");
    else
        ui_->label_leftScore->setText(QString::number(lScore));

    if (rScore == -1)
        ui_->label_rightScore->setText("-");
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
void MetadataEditWidget_Game::onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Game, onOverwriteMetadata);

    assert(map.count() == mdata.count());

    ignoreSelf_ = true;

    for (int i = 0; i != map.count(); ++i)
    {
        switch (mdata[i]->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = mdata[i]->asGame();

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
                        ui_->comboBox_setFormat->currentIndex() >= 0 ? ui_->comboBox_setFormat->currentIndex() : 0));

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
                    const PlayerDetails::Player defaultPlayer{"", "", "", ""};
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

            case rfcommon::Metadata::TRAINING: {
            } break;
        }
    }

    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Game, onMetadataCleared);

    playerDetails_->save();
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onNextGameStarted()
{
    PROFILE(MetadataEditWidget_Game, onNextGameStarted);

    auto allTagsMatch = [](rfcommon::Metadata* a, rfcommon::Metadata* b) -> bool {
        assert(a->fighterCount() == b->fighterCount());
        for (int i = 0; i != a->fighterCount(); ++i)
            if (a->playerTag(i) != b->playerTag(i))
                return false;
        return true;
    };

    auto allFightersMatch = [](rfcommon::Metadata* a, rfcommon::Metadata* b) -> bool {
        assert(a->fighterCount() == b->fighterCount());
        for (int i = 0; i != a->fighterCount(); ++i)
            if (a->playerFighterID(i) != b->playerFighterID(i))
                return false;
        return true;
    };

    auto advanceRound = [](rfcommon::GameMetadata* mdata, rfcommon::GameMetadata* prev) {
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

    auto advanceScores = [&advanceRound](rfcommon::GameMetadata* mdata, rfcommon::GameMetadata* prev) {
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

    rfcommon::Metadata* mdata = model_->metadata()[0];
    rfcommon::Metadata* prev = model_->prevMetadata();

    ignoreSelf_ = true;
    switch (model_->prevMetadata()->type())
    {
        case rfcommon::Metadata::GAME: {
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

        case rfcommon::Metadata::TRAINING: {
            if (mdata->type() == prev->type())
                mdata->asTraining()->setSessionNumber(prev->asTraining()->sessionNumber() + 1);
        } break;
    }
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Game::onBracketTypeChangedUI(rfcommon::BracketType bracketType)
{
    PROFILE(MetadataEditWidget_Game, onBracketTypeChangedUI);

    switch (bracketType.type())
    {
        case rfcommon::BracketType::SINGLES:
        case rfcommon::BracketType::DOUBLES:
        case rfcommon::BracketType::SIDE:
        case rfcommon::BracketType::AMATEURS:
            enableFreePlayOption(false);
            enableRoundTypeSelection(true);
            ui_->comboBox_roundType->setCurrentIndex(rfcommon::Round::WINNERS_ROUND);
            updateRoundTypeUI(rfcommon::Round::WINNERS_ROUND);
            break;

        case rfcommon::BracketType::MONEYMATCH:
            enableFreePlayOption(false);
            enableRoundTypeSelection(false);
            enableGrandFinalOptions(false);
            enableRoundCounter(true);
            break;

        case rfcommon::BracketType::PRACTICE:
        case rfcommon::BracketType::FRIENDLIES:
        case rfcommon::BracketType::OTHER:
            enableFreePlayOption(true);
            enableRoundTypeSelection(false);
            enableGrandFinalOptions(false);
            enableRoundCounter(true);
            break;
    }

    const QSignalBlocker blockSetFormat(ui_->comboBox_setFormat);
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

void MetadataEditWidget_Game::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetadataEditWidget_Game::onMetadataTournamentDetailsChanged() {}
void MetadataEditWidget_Game::onMetadataEventDetailsChanged()
{
    PROFILE(MetadataEditWidget_Game, onMetadataEventDetailsChanged);

    if (ignoreSelf_)
        return;
    onAdoptMetadata(model_->mappingInfo(), model_->metadata());
}
void MetadataEditWidget_Game::onMetadataCommentatorsChanged() {}
void MetadataEditWidget_Game::onMetadataGameDetailsChanged()
{
    PROFILE(MetadataEditWidget_Game, onMetadataGameDetailsChanged);

    if (ignoreSelf_)
        return;
    onAdoptMetadata(model_->mappingInfo(), model_->metadata());
}
void MetadataEditWidget_Game::onMetadataPlayerDetailsChanged()
{
    PROFILE(MetadataEditWidget_Game, onMetadataPlayerDetailsChanged);

    if (ignoreSelf_)
        return;
    onAdoptMetadata(model_->mappingInfo(), model_->metadata());
}
void MetadataEditWidget_Game::onMetadataWinnerChanged(int winnerPlayerIdx) {}
void MetadataEditWidget_Game::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
