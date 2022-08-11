#include "application/ui_ReplayEditorDialog.h"
#include "application/views/ReplayEditorDialog.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/SetFormat.hpp"

#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayEditorDialog::ReplayEditorDialog(rfcommon::Session* session, QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::ReplayEditorDialog)
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
    }

    connect(ui_->cancel, &QPushButton::released, this, &ReplayEditorDialog::close);
    connect(ui_->save, &QPushButton::released, this, &ReplayEditorDialog::onSaveClicked);
}

// ----------------------------------------------------------------------------
ReplayEditorDialog::~ReplayEditorDialog()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSaveClicked()
{

}

}
