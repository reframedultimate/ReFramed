#include "application/models/MetadataEditModel.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/widgets/MetadataEditWidget_Commentators.hpp"
#include "application/widgets/MetadataEditWidget_Event.hpp"
#include "application/widgets/MetadataEditWidget_Game.hpp"
#include "application/widgets/MetadataEditWidget_Tournament.hpp"
#include "application/views/ReplayEditorDialog.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTimeEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QScrollArea>

namespace rfapp {

using nlohmann::json;

// ----------------------------------------------------------------------------
ReplayEditorDialog::ReplayEditorDialog(
        Config* config,
        ReplayManager* replayManager,
        PlayerDetails* playerDetails,
        const QStringList& replayFileNames,
        QWidget* parent)
    : QDialog(parent)
    , ConfigAccessor(config)
    , metadataEditModel_(new MetadataEditModel)
    , replayManager_(replayManager)
{
    setWindowTitle("Edit Replay Metadata");

    metadataEditWidgets_.push_back(new MetadataEditWidget_Tournament(metadataEditModel_.get()));
    metadataEditWidgets_.push_back(new MetadataEditWidget_Commentators(metadataEditModel_.get()));
    metadataEditWidgets_.push_back(new MetadataEditWidget_Event(metadataEditModel_.get()));
    metadataEditWidgets_.push_back(new MetadataEditWidget_Game(metadataEditModel_.get(), playerDetails));

    json& cfg = configRoot();
    json& jReplayEditorDialog = cfg["replayeditordialog"];
    json& jExpanded = jReplayEditorDialog["expanded"];
    if (jExpanded.is_array())
        for (int i = 0; i != (int)jExpanded.size() && i < metadataEditWidgets_.size(); ++i)
            if (jExpanded[i].is_boolean() && jExpanded[i].get<bool>())
                metadataEditWidgets_[i]->setExpanded(true);

    QVBoxLayout* metadataEditLayout = new QVBoxLayout;
    for (MetadataEditWidget* w : metadataEditWidgets_)
        metadataEditLayout->addWidget(w);
    metadataEditLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QWidget* metadataEditContents = new QWidget;
    metadataEditContents->setLayout(metadataEditLayout);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(metadataEditContents);

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

    MetadataEditModel::MappingInfoList mappingInfo;
    MetadataEditModel::MetadataList metadata;
    for (const auto& fileName : replayFileNames)
    {
        auto filePathUtf8 = replayManager_->resolveGameFile(fileName.toUtf8().constData());
        rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(replayManager_, filePathUtf8.cStr());
        if (session == nullptr)
        {
            if (QMessageBox::question(this,
                "Error loading file",
                "Failed to load replay file \"" + fileName + "\"\n"
                "Absolute path was: \"" + filePathUtf8.cStr() + "\"\n\n"
                "Would you like to continue without including this file?") == QMessageBox::Yes)
            {
                continue;
            }
            QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
            return;
        }

        auto map = session->tryGetMappingInfo();
        auto mdata = session->tryGetMetadata();
        if (map == nullptr || mdata == nullptr)
        {
            if (QMessageBox::question(this,
                "Missing metadata",
                "Replay has missing metadata \"" + fileName + "\"\n"
                "Absolute path was: \"" + filePathUtf8.cStr() + "\"\n\n"
                "Would you like to continue without including this file?") == QMessageBox::Yes)
            {
                continue;
            }
            QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
            return;
        }

        mappingInfo.push(map);
        metadata.push(mdata);
        loadedSessions_.push(session);
        loadedSessionFileNames_.push_back(fileName);
    }

    if (mappingInfo.count() == 0)
    {
        QMessageBox::critical(this,
                "No metadata loaded",
                "The selected replays contain no metadata. Can't do anything");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }

    metadataEditModel_->setAndAdopt(std::move(mappingInfo), std::move(metadata));

    connect(saveButton, &QPushButton::released, this, &ReplayEditorDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::released, this, &ReplayEditorDialog::close);
}

// ----------------------------------------------------------------------------
ReplayEditorDialog::~ReplayEditorDialog()
{
    json& cfg = configRoot();
    json& jReplayEditorDialog = cfg["replayeditordialog"];
    json jExpanded = json::array();
    for (MetadataEditWidget* w : metadataEditWidgets_)
        jExpanded.push_back(w->isExpanded());
    jReplayEditorDialog["expanded"] = jExpanded;

    // We have to make sure that the UI elements registered to the metadataEditModel_
    // get deleted before the model itself is deleted
    delete layout()->itemAt(0)->widget();  // this is the scroll area widget
}

// ----------------------------------------------------------------------------
void ReplayEditorDialog::onSaveClicked()
{
    PROFILE(ReplayEditorDialog, onSaveClicked);

    bool success = true;
    for (int i = 0; i != loadedSessions_.count(); ++i)
    {
        rfcommon::Session* session = loadedSessions_[i];
        auto fileName = loadedSessionFileNames_[i];
        if (replayManager_->saveReplayOver(session, fileName) == false)
        {
            success = false;
            if (QMessageBox::question(this,
                "Error saving file",
                "Failed to save replay file \"" + fileName + "\"\n"
                "Continue?") != QMessageBox::Yes)
            {
                return;
            }
        }
    }

    if (success)
        close();
}

}
