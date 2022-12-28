#include "application/models/MetaDataEditModel.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/widgets/MetaDataEditWidget_Commentators.hpp"
#include "application/widgets/MetaDataEditWidget_Event.hpp"
#include "application/widgets/MetaDataEditWidget_Game.hpp"
#include "application/widgets/MetaDataEditWidget_Tournament.hpp"
#include "application/views/ReplayEditorDialog.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
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

// ----------------------------------------------------------------------------
ReplayEditorDialog::ReplayEditorDialog(
        ReplayManager* replayManager,
        PlayerDetails* playerDetails,
        const QStringList& replayFileNames,
        QWidget* parent)
    : QDialog(parent)
    , metaDataEditModel_(new MetaDataEditModel)
    , replayManager_(replayManager)
{
    setWindowTitle("Edit Replay Meta Data");

    MetaDataEditWidget_Tournament* tournament = new MetaDataEditWidget_Tournament(metaDataEditModel_.get());
    MetaDataEditWidget_Commentators* commentators = new MetaDataEditWidget_Commentators(metaDataEditModel_.get());
    MetaDataEditWidget_Event* event = new MetaDataEditWidget_Event(metaDataEditModel_.get());
    MetaDataEditWidget_Game* game = new MetaDataEditWidget_Game(metaDataEditModel_.get(), playerDetails);

    tournament->setExpanded(true);
    commentators->setExpanded(true);
    event->setExpanded(true);
    game->setExpanded(true);

    QVBoxLayout* metaDataEditLayout = new QVBoxLayout;
    metaDataEditLayout->addWidget(tournament);
    metaDataEditLayout->addWidget(commentators);
    metaDataEditLayout->addWidget(event);
    metaDataEditLayout->addWidget(game);
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

    MetaDataEditModel::MappingInfoList mappingInfo;
    MetaDataEditModel::MetaDataList metaData;
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
        auto mdata = session->tryGetMetaData();
        if (map == nullptr || mdata == nullptr)
        {
            if (QMessageBox::question(this,
                "Missing meta data",
                "Replay has missing meta data \"" + fileName + "\"\n"
                "Absolute path was: \"" + filePathUtf8.cStr() + "\"\n\n"
                "Would you like to continue without including this file?") == QMessageBox::Yes)
            {
                continue;
            }
            QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
            return;
        }

        mappingInfo.push(map);
        metaData.push(mdata);
        loadedSessions_.push(session);
        loadedSessionFileNames_.push_back(fileName);
    }

    if (mappingInfo.count() == 0)
    {
        QMessageBox::critical(this,
                "No meta data loaded",
                "The selected replays contain no meta data. Can't do anything");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }

    metaDataEditModel_->setAndAdopt(std::move(mappingInfo), std::move(metaData));

    connect(saveButton, &QPushButton::released, this, &ReplayEditorDialog::onSaveClicked);
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
