#include "application/models/MetaDataEditModel.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/widgets/MetaDataEditWidget_Commentators.hpp"
#include "application/widgets/MetaDataEditWidget_Event.hpp"
#include "application/widgets/MetaDataEditWidget_Game.hpp"
#include "application/widgets/MetaDataEditWidget_Tournament.hpp"
#include "application/views/ReplayEditorDialog.hpp"

#include "rfcommon/Session.hpp"
#include "rfcommon/Profiler.hpp"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTimeEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QScrollArea>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayEditorDialog::ReplayEditorDialog(
        ReplayManager* replayManager,
        rfcommon::Session* session,
        const QString& currentFileNameParts,
        QWidget* parent)
    : QDialog(parent)
    , metaDataEditModel_(new MetaDataEditModel)
    , replayManager_(replayManager)
    , session_(session)
    , currentFileNameParts_(currentFileNameParts)
{
    setWindowTitle("Edit Replay Meta Data");

    QVBoxLayout* metaDataEditLayout = new QVBoxLayout;
    metaDataEditLayout->addWidget(new MetaDataEditWidget_Tournament(metaDataEditModel_.get()));
    metaDataEditLayout->addWidget(new MetaDataEditWidget_Commentators(metaDataEditModel_.get()));
    metaDataEditLayout->addWidget(new MetaDataEditWidget_Event(metaDataEditModel_.get()));
    metaDataEditLayout->addWidget(new MetaDataEditWidget_Game(metaDataEditModel_.get()));
    metaDataEditLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QWidget* metaDataEditContents = new QWidget;
    metaDataEditContents->setLayout(metaDataEditLayout);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(metaDataEditContents);

    QPushButton* saveButton = new QPushButton("Save");
    QPushButton* cancelButton = new QPushButton("Cancel");

    QHBoxLayout* saveCancelLayout = new QHBoxLayout;
    saveCancelLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    saveCancelLayout->addWidget(saveButton);
    saveCancelLayout->addWidget(cancelButton);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(scrollArea);
    l->addLayout(saveCancelLayout);
    setLayout(l);

    if (auto map = session_->tryGetMappingInfo())
        if (auto mdata = session_->tryGetMetaData())
            metaDataEditModel_->setAndAdopt(map, mdata);

    connect(cancelButton, &QPushButton::released, this, &ReplayEditorDialog::close);
}

// ----------------------------------------------------------------------------
ReplayEditorDialog::~ReplayEditorDialog()
{
    // We have to make sure that the UI elements registered to the metaDataEditModel_
    // get deleted before the model itself is deleted
    delete layout()->itemAt(0)->widget();  // this is the scroll area widget
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSaveClicked()
{
    PROFILE(ReplayEditorDialog, onSaveClicked);

    if (replayManager_->saveReplayOver(session_, currentFileNameParts_))
        close();
    else
        QMessageBox::critical(this, "Error", "Failed to save file");
}

}
