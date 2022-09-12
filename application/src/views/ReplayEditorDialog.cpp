#include "application/ui_ReplayEditorDialog.h"
#include "application/views/ReplayEditorDialog.hpp"
#include "application/models/ReplayManager.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/SetFormat.hpp"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTimeEdit>
#include <QSpinBox>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayEditorDialog::ReplayEditorDialog(
        ReplayManager* replayManager,
        rfcommon::Session* session,
        const QString& currentFileName,
        QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::ReplayEditorDialog)
    , replayManager_(replayManager)
    , session_(session)
    , currentFileName_(currentFileName)
{
    ui_->setupUi(this);

    // Window icon
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

#define X(name, shortstr, longstr) ui_->setFormat->addItem(longstr);
    SET_FORMAT_LIST
#undef X

    rfcommon::MappingInfo* map = session->tryGetMappingInfo();
    rfcommon::MetaData* mdata = session->tryGetMetaData();

    ui_->stageID->setText(QString::number(mdata->stageID().value()));
    ui_->stageName->setText(map->stage.toName(mdata->stageID()));
    ui_->timeStarted->setDateTime(QDateTime::fromMSecsSinceEpoch(mdata->timeStarted().millisSinceEpoch()));
    ui_->timeEnded->setDateTime(QDateTime::fromMSecsSinceEpoch(mdata->timeEnded().millisSinceEpoch()));

    switch (mdata->type())
    {
        case rfcommon::MetaData::GAME: {
            auto game = mdata->asGame();

            ui_->tournamentName->setText(game->tournamentName().cStr());
            ui_->eventName->setText(game->eventName().cStr());

            ui_->setNumber->setValue(game->setNumber().value());
            ui_->setFormat->setCurrentIndex(game->setFormat().index());
            ui_->customSetFormat->setVisible(game->setFormat().type() == rfcommon::SetFormat::OTHER);
            if (game->setFormat().type() == rfcommon::SetFormat::OTHER)
                ui_->customSetFormat->setText(game->setFormat().longDescription());
            ui_->gameNumber->setValue(game->gameNumber().value());
            ui_->round->setText(game->roundName().cStr());

            const auto& comms = game->commentators();
            if (comms.count() > 0)
                ui_->commentator1->setText(comms[0].cStr());
            if (comms.count() > 1)
                ui_->commentator2->setText(comms[1].cStr());
        } break;

        case rfcommon::MetaData::TRAINING: {
            ui_->groupBox_tournament->setVisible(false);
            ui_->groupBox_event->setVisible(false);
            ui_->groupBox_commentators->setVisible(false);
            ui_->gameNumber->setVisible(false);
            ui_->setNumber->setVisible(false);
            ui_->setFormat->setVisible(false);
            ui_->customSetFormat->setVisible(false);
        } break;
    }

    QGridLayout* playerInfoLayout = new QGridLayout;
    ui_->groupBox_playerInfo->setLayout(playerInfoLayout);
    for (int fighterIdx = 0; fighterIdx != mdata->fighterCount(); ++fighterIdx)
    {
        QLineEdit* sponsor = new QLineEdit;
        sponsor->setText(mdata->sponsor(fighterIdx).cStr());
        sponsor->setFixedWidth(100);
        sponsor->setPlaceholderText("Sponsor");

        QLineEdit* name = new QLineEdit;
        name->setText(mdata->name(fighterIdx).cStr());

        QLabel* label = new QLabel();
        label->setText("Player " + QString::number(fighterIdx + 1) + ":");

        playerInfoLayout->addWidget(label, fighterIdx, 0);
        playerInfoLayout->addWidget(sponsor, fighterIdx, 1);
        playerInfoLayout->addWidget(name, fighterIdx, 2);

        connect(sponsor, &QLineEdit::textChanged, [this, fighterIdx](const QString& text) {
            onPlayerSponsorChanged(fighterIdx, text);
        });
        connect(name, &QLineEdit::textChanged, [this, fighterIdx](const QString& text) {
            onPlayerNameChanged(fighterIdx, text);
        });
    }

    connect(ui_->cancel, &QPushButton::released, this, &ReplayEditorDialog::close);
    connect(ui_->save, &QPushButton::released, this, &ReplayEditorDialog::onSaveClicked);

    connect(ui_->tournamentName, &QLineEdit::textChanged, this, &ReplayEditorDialog::onTournamentNameChanged);
    connect(ui_->eventName, &QLineEdit::textChanged, this, &ReplayEditorDialog::onEventNameChanged);

    connect(ui_->timeStarted, &QDateTimeEdit::dateTimeChanged, this, &ReplayEditorDialog::onTimeStartedChanged);
    connect(ui_->setNumber, qOverload<int>(&QSpinBox::valueChanged), this, &ReplayEditorDialog::onSetNumberChanged);
    connect(ui_->setFormat, qOverload<int>(&QComboBox::currentIndexChanged), this, &ReplayEditorDialog::onSetFormatChanged);
    connect(ui_->customSetFormat, &QLineEdit::textChanged, this, &ReplayEditorDialog::customFormatTextChanged);
    connect(ui_->gameNumber, qOverload<int>(&QSpinBox::valueChanged), this, &ReplayEditorDialog::onGameNumberChanged);
    connect(ui_->round, &QLineEdit::textChanged, this, &ReplayEditorDialog::onRoundChanged);

    connect(ui_->commentator1, &QLineEdit::textChanged, this, &ReplayEditorDialog::onCommentatorChanged);
    connect(ui_->commentator2, &QLineEdit::textChanged, this, &ReplayEditorDialog::onCommentatorChanged);
}

// ----------------------------------------------------------------------------
ReplayEditorDialog::~ReplayEditorDialog()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSaveClicked()
{
    PROFILE(ReplayEditorDialog, onSaveClicked);

    if (replayManager_->saveReplayOver(session_, currentFileName_))
        close();
    else
        QMessageBox::critical(this, "Error", "Failed to save file");
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onTournamentNameChanged(const QString& name)
{
    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setTournamentName(name.toUtf8().constData());
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onEventNameChanged(const QString& name)
{
    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setEventName(name.toUtf8().constData());
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onTimeStartedChanged(const QDateTime& started)
{
    PROFILE(ReplayEditorDialog, onTimeStartedChanged);

    if (auto mdata = session_->tryGetMetaData())
    {
        const QDateTime ended = started.addMSecs(mdata->length().millis());
        ui_->timeEnded->setDateTime(ended);
        mdata->setTimeStarted(rfcommon::TimeStamp::fromMillisSinceEpoch(started.toMSecsSinceEpoch()));
        mdata->setTimeEnded(rfcommon::TimeStamp::fromMillisSinceEpoch(ended.toMSecsSinceEpoch()));
    }
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSetNumberChanged(int value)
{
    PROFILE(ReplayEditorDialog, onSetNumberChanged);

    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setSetNumber(rfcommon::SetNumber::fromValue(value));
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSetFormatChanged(int index)
{
    PROFILE(ReplayEditorDialog, onSetFormatChanged);

    ui_->customSetFormat->setVisible(index == rfcommon::SetFormat::OTHER);

    if (auto mdata = session_->tryGetMetaData())
    {
        if (index < rfcommon::SetFormat::OTHER)
            mdata->asGame()->setSetFormat(rfcommon::SetFormat::fromIndex(index));
        else
        {
            QByteArray ba = ui_->customSetFormat->text().toUtf8();
            mdata->asGame()->setSetFormat(rfcommon::SetFormat::makeOther(ba.constData()));
        }
    }
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::customFormatTextChanged(const QString& text)
{
    PROFILE(ReplayEditorDialog, customFormatTextChanged);

    if (auto mdata = session_->tryGetMetaData())
    {
        QByteArray ba = text.toUtf8();
        mdata->asGame()->setSetFormat(rfcommon::SetFormat::makeOther(ba.constData()));
    }
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onGameNumberChanged(int value)
{
    PROFILE(ReplayEditorDialog, onGameNumberChanged);

    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setGameNumber(rfcommon::GameNumber::fromValue(value));
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onRoundChanged(const QString& name)
{
    PROFILE(ReplayEditorDialog, onRoundChanged);

    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->setRoundName(name.toUtf8().constData());
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onPlayerSponsorChanged(int fighterIdx, const QString& sponsor)
{
    PROFILE(ReplayEditorDialog, onPlayerSponsorChanged);

    if (auto mdata = session_->tryGetMetaData())
    {
        QByteArray ba = sponsor.toUtf8();
        mdata->asGame()->setSponsor(fighterIdx, ba.constData());
    }
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onPlayerNameChanged(int fighterIdx, const QString& name)
{
    PROFILE(ReplayEditorDialog, onPlayerNameChanged);

    if (auto mdata = session_->tryGetMetaData())
    {
        QByteArray ba = name.toUtf8();
        mdata->asGame()->setName(fighterIdx, ba.constData());
    }
}

// ----------------------------------------------------------------------------
static void updateCommentators(rfcommon::GameMetaData* mdata, QLineEdit* c1, QLineEdit* c2)
{
    rfcommon::SmallVector<rfcommon::String, 2> names;
    if (c1->text().length() > 0)
        names.push(c1->text().toUtf8().constData());
    if (c2->text().length() > 0)
        names.push(c2->text().toUtf8().constData());

    mdata->setCommentators(std::move(names));
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onCommentatorChanged(const QString& name)
{
    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            updateCommentators(mdata->asGame(), ui_->commentator1, ui_->commentator2);
}

}
