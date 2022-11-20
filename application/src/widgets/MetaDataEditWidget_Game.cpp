#include "application/ui_MetaDataEditWidget_Game.h"
#include "application/models/MetaDataEditModel.hpp"
#include "application/widgets/MetaDataEditWidget_Game.hpp"

#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Round.hpp"
#include "rfcommon/SetFormat.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::MetaDataEditWidget_Game(MetaDataEditModel* model, QWidget* parent)
    : MetaDataEditWidget(model, parent)
    , ui_(new Ui::MetaDataEditWidget_Game)
{
    ui_->setupUi(contentWidget());

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

    setTitle("Game");
    setExpanded(true);

    connect(ui_->dateTimeEdit_started, &QDateTimeEdit::dateTimeChanged, this, &MetaDataEditWidget_Game::onDateTimeStartedChanged);
    connect(ui_->pushButton_resetTimeStarted, &QPushButton::released, this, &MetaDataEditWidget_Game::onPushButtonResetTimeStartedReleased);
    connect(ui_->comboBox_roundType, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetaDataEditWidget_Game::onComboBoxRoundTypeChanged);
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
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::~MetaDataEditWidget_Game()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onDateTimeStartedChanged(const QDateTime& dateTime)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonResetTimeStartedReleased()
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onComboBoxRoundTypeChanged(int index)
{
    enableGrandFinalOptions(index == rfcommon::Round::GRAND_FINALS);
    enableRoundCounter(
                index == rfcommon::Round::WINNERS_ROUND ||
                index == rfcommon::Round::LOSERS_ROUND ||
                index == rfcommon::Round::POOLS);

    updateSize();
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onComboBoxSetFormatChanged(int index)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onSpinBoxGameNumberChanged(int value)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onCheckBoxLeftLoserSideChanged(bool enable)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onCheckBoxRightLoserSideChanged(bool enable)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftNameChanged(const QString& text)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightNameChanged(const QString& text)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftSponsorChanged(const QString& text)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightSponsorChanged(const QString& text)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftSocialChanged(const QString& text)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightSocialChanged(const QString& text)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditLeftPronounsChanged(const QString& text)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onLineEditRightPronounsChanged(const QString& text)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonIncLeftScoreReleased()
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonDecLeftScoreReleased()
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonIncRightScoreReleased()
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onPushButtonDecRightScoreReleased()
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::enableRoundCounter(bool enable)
{
    ui_->spinBox_roundNumber->setVisible(enable);
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::enableFreePlayOption(bool enable)
{
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
void MetaDataEditWidget_Game::onAdoptMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    switch (mdata->type())
    {
        case rfcommon::MetaData::GAME: {
            rfcommon::GameMetaData* g = mdata->asGame();
            ui_->dateTimeEdit_started->setDateTime(QDateTime::fromMSecsSinceEpoch(g->timeStarted().millisSinceEpoch()));
            ui_->dateTimeEdit_ended->setDateTime(QDateTime::fromMSecsSinceEpoch(g->timeEnded().millisSinceEpoch()));
            ui_->lineEdit_stageID->setText(QString::number(g->stageID().value()));
            ui_->lineEdit_stageName->setText(QString::fromUtf8(map->stage.toName(g->stageID())));
            ui_->comboBox_roundType->setCurrentIndex(g->round().index());
            ui_->spinBox_roundNumber->setValue(g->round().number().value());
            ui_->comboBox_setFormat->setCurrentIndex(g->setFormat().index());

            if (g->fighterCount() == 2)  // 1v1
            {
                ui_->lineEdit_leftName->setText(QString::fromUtf8(g->playerName(0).cStr()));
                ui_->lineEdit_leftSponsor->setText(QString::fromUtf8(g->playerSponsor(0).cStr()));
                ui_->lineEdit_leftSocial->setText(QString::fromUtf8(g->playerSocial(0).cStr()));
                ui_->lineEdit_leftPronouns->setText(QString::fromUtf8(g->playerPronouns(0).cStr()));

                ui_->lineEdit_rightName->setText(QString::fromUtf8(g->playerName(1).cStr()));
                ui_->lineEdit_rightSponsor->setText(QString::fromUtf8(g->playerSponsor(1).cStr()));
                ui_->lineEdit_rightSocial->setText(QString::fromUtf8(g->playerSocial(1).cStr()));
                ui_->lineEdit_rightPronouns->setText(QString::fromUtf8(g->playerPronouns(1).cStr()));

                if (g->round().type() == rfcommon::Round::GRAND_FINALS)
                {
                    ui_->checkBox_leftL->setChecked(g->playerIsLoserSide(0));
                    ui_->checkBox_rightL->setChecked(g->playerIsLoserSide(1));
                }
            }
        } break;

        case rfcommon::MetaData::TRAINING:
            break;
    }
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onOverwriteMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    switch (mdata->type())
    {
        case rfcommon::MetaData::GAME: {
            rfcommon::GameMetaData* g = mdata->asGame();
            if (g->fighterCount() == 2)  // 1v1
            {
                g->setPlayerName(0, ui_->lineEdit_leftName->text().toUtf8().constData());
                g->setPlayerSponsor(0, ui_->lineEdit_leftSponsor->text().toUtf8().constData());
                g->setPlayerSocial(0, ui_->lineEdit_leftSocial->text().toUtf8().constData());
                g->setPlayerPronouns(0, ui_->lineEdit_leftPronouns->text().toUtf8().constData());

                g->setPlayerName(1, ui_->lineEdit_rightName->text().toUtf8().constData());
                g->setPlayerSponsor(1, ui_->lineEdit_rightSponsor->text().toUtf8().constData());
                g->setPlayerSocial(1, ui_->lineEdit_rightSocial->text().toUtf8().constData());
                g->setPlayerPronouns(1, ui_->lineEdit_rightPronouns->text().toUtf8().constData());

                if (ui_->comboBox_roundType->currentIndex() == rfcommon::Round::GRAND_FINALS)
                {
                    g->setPlayerIsLoserSide(0, ui_->checkBox_leftL->isChecked());
                    g->setPlayerIsLoserSide(1, ui_->checkBox_rightL->isChecked());
                }
            }
        } break;

        case rfcommon::MetaData::TRAINING: {
        } break;
    }

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

    if (prevMetaData_.notNull() &&
            mdata->type() == prevMetaData_->type() &&
            allTagsMatch(mdata, prevMetaData_) &&
            allFightersMatch(mdata, prevMetaData_))
    {

    }
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onMetaDataCleared(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{

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
    rfcommon::MetaData* m = model_->metaData();

    switch (m->type())
    {
        case rfcommon::MetaData::GAME: {
            rfcommon::GameMetaData* game = m->asGame();
            onBracketTypeChangedUI(game->bracketType());
        } break;

        case rfcommon::MetaData::TRAINING: {
        } break;
    }
}
void MetaDataEditWidget_Game::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_Game::onMetaDataGameDetailsChanged()
{

}
void MetaDataEditWidget_Game::onMetaDataPlayerDetailsChanged()
{

}
void MetaDataEditWidget_Game::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Game::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
