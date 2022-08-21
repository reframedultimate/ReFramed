#include "application/ui_ReplayEditorDialog.h"
#include "application/views/ReplayEditorDialog.hpp"
#include "application/models/ReplayManager.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/SetFormat.hpp"

#include <QFormLayout>
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
            auto game = static_cast<rfcommon::GameMetaData*>(mdata);
            ui_->gameNumber->setValue(game->gameNumber().value());
            ui_->setNumber->setValue(game->setNumber().value());
            ui_->setFormat->setCurrentIndex(game->setFormat().index());
            ui_->customSetFormat->setVisible(game->setFormat().type() == rfcommon::SetFormat::OTHER);
            if (game->setFormat().type() == rfcommon::SetFormat::OTHER)
                ui_->customSetFormat->setText(game->setFormat().longDescription());
        } break;

        case rfcommon::MetaData::TRAINING: {
            ui_->gameNumber->setVisible(false);
            ui_->setNumber->setVisible(false);
            ui_->setFormat->setVisible(false);
            ui_->customSetFormat->setVisible(false);
        } break;
    }

    QFormLayout* playerInfoLayout = new QFormLayout;
    ui_->groupBox_playerInfo->setLayout(playerInfoLayout);
    for (int fighterIdx = 0; fighterIdx != mdata->fighterCount(); ++fighterIdx)
    {
        QLineEdit* edit = new QLineEdit;
        edit->setText(mdata->name(fighterIdx).cStr());
        playerInfoLayout->addRow("Player " + QString::number(fighterIdx + 1) + " name:", edit);

        connect(edit, &QLineEdit::textChanged, [this, fighterIdx](const QString& text){
            onPlayerNameChanged(fighterIdx, text);
        });
    }

    connect(ui_->cancel, &QPushButton::released, this, &ReplayEditorDialog::close);
    connect(ui_->save, &QPushButton::released, this, &ReplayEditorDialog::onSaveClicked);
    connect(ui_->timeStarted, &QDateTimeEdit::dateTimeChanged, this, &ReplayEditorDialog::onTimeStartedChanged);
    connect(ui_->gameNumber, qOverload<int>(&QSpinBox::valueChanged), this, &ReplayEditorDialog::onGameNumberChanged);
    connect(ui_->setNumber, qOverload<int>(&QSpinBox::valueChanged), this, &ReplayEditorDialog::onSetNumberChanged);
    connect(ui_->setFormat, qOverload<int>(&QComboBox::currentIndexChanged), this, &ReplayEditorDialog::onSetFormatChanged);
    connect(ui_->customSetFormat, &QLineEdit::textChanged, this, &ReplayEditorDialog::customFormatTextChanged);
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
void ReplayEditorDialog::onGameNumberChanged(int value)
{
    PROFILE(ReplayEditorDialog, onGameNumberChanged);

    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            static_cast<rfcommon::GameMetaData*>(mdata)->setGameNumber(
                    rfcommon::GameNumber::fromValue(value));
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSetNumberChanged(int value)
{
    PROFILE(ReplayEditorDialog, onSetNumberChanged);

    if (auto mdata = session_->tryGetMetaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            static_cast<rfcommon::GameMetaData*>(mdata)->setSetNumber(
                    rfcommon::SetNumber::fromValue(value));
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSetFormatChanged(int index)
{
    PROFILE(ReplayEditorDialog, onSetFormatChanged);

    ui_->customSetFormat->setVisible(index == rfcommon::SetFormat::OTHER);

    if (auto mdata = session_->tryGetMetaData())
    {
        if (index < rfcommon::SetFormat::OTHER)
        {
            static_cast<rfcommon::GameMetaData*>(mdata)->setSetFormat(
                    rfcommon::SetFormat::fromIndex(index));
        }
        else
        {
            QByteArray ba = ui_->customSetFormat->text().toUtf8();
            static_cast<rfcommon::GameMetaData*>(mdata)->setSetFormat(
                    rfcommon::SetFormat::makeOther(ba.constData()));
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
        static_cast<rfcommon::GameMetaData*>(mdata)->setSetFormat(
                rfcommon::SetFormat::makeOther(ba.constData()));
    }
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onPlayerNameChanged(int fighterIdx, const QString& name)
{
    PROFILE(ReplayEditorDialog, onPlayerNameChanged);

    if (auto mdata = session_->tryGetMetaData())
    {
        QByteArray ba = name.toUtf8();
        static_cast<rfcommon::GameMetaData*>(mdata)->setName(fighterIdx, ba.constData());
    }
}

}
