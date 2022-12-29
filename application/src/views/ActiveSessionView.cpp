#include "application/models/ActiveSessionManager.hpp"
#include "application/models/MetadataEditModel.hpp"
#include "application/views/ActiveSessionView.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/widgets/CollapsibleSplitter.hpp"
#include "application/widgets/MetadataEditWidget_AutoAssociateVideo.hpp"
#include "application/widgets/MetadataEditWidget_Commentators.hpp"
#include "application/widgets/MetadataEditWidget_Event.hpp"
#include "application/widgets/MetadataEditWidget_Game.hpp"
#include "application/widgets/MetadataEditWidget_Tournament.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"

#include <QVBoxLayout>
#include <QDateTime>
#include <QSplitter>
#include <QScrollArea>
#include <QSpacerItem>

namespace rfapp {

// ----------------------------------------------------------------------------
namespace {

class ScrollAreaWithSizeHint : public QScrollArea
{
public:
    QSize sizeHint() const override { return QSize(900, 100); }
};

}

// ----------------------------------------------------------------------------
ActiveSessionView::ActiveSessionView(
        ActiveSessionManager* activeSessionManager,
        PluginManager* pluginManager,
        PlayerDetails* playerDetails,
        QWidget* parent)
    : QWidget(parent)
    , activeSessionManager_(activeSessionManager)
    , metadataEditModel_(new MetadataEditModel)
{
    MetadataEditWidget_AutoAssociateVideo* assocVideo = new MetadataEditWidget_AutoAssociateVideo(metadataEditModel_.get(), activeSessionManager_);
    MetadataEditWidget_Tournament* tournament = new MetadataEditWidget_Tournament(metadataEditModel_.get());
    MetadataEditWidget_Commentators* commentators = new MetadataEditWidget_Commentators(metadataEditModel_.get());
    MetadataEditWidget_Event* event = new MetadataEditWidget_Event(metadataEditModel_.get());
    MetadataEditWidget_Game* game = new MetadataEditWidget_Game(metadataEditModel_.get(), playerDetails);

    event->setExpanded(true);
    game->setExpanded(true);

    QVBoxLayout* metadataEditLayout = new QVBoxLayout;
    metadataEditLayout->addWidget(assocVideo);
    metadataEditLayout->addWidget(tournament);
    metadataEditLayout->addWidget(commentators);
    metadataEditLayout->addWidget(event);
    metadataEditLayout->addWidget(game);
    metadataEditLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QWidget* metadataEditContents = new QWidget;
    metadataEditContents->setLayout(metadataEditLayout);

    scrollArea_ = new ScrollAreaWithSizeHint;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setWidget(metadataEditContents);

    splitter_ = new CollapsibleSplitter(Qt::Horizontal);
    splitter_->addWidget(scrollArea_);
    splitter_->addWidget(new PluginDockView(activeSessionManager_->protocol(), pluginManager));
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(splitter_);
    setLayout(l);

    activeSessionManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveSessionView::~ActiveSessionView()
{
    // Scroll area contains widgets that are registered as listeners to
    // metadataEditModel_. Have to delete them explicitly, otherwise the model
    // is deleted before the widgets are deleted.
    delete scrollArea_;

    activeSessionManager_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ActiveSessionView::toggleSideBar()
{
    PROFILE(ActiveSessionView, toggleSideBar);

    splitter_->toggleCollapse();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameStarted(rfcommon::Session* game)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerGameStarted);

    auto* map = game->tryGetMappingInfo();
    auto* mdata = game->tryGetMetadata();
    assert(map);
    assert(mdata);

    metadataEditModel_->setAndOverwrite({map}, {mdata});
    metadataEditModel_->startNextGame();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameEnded(rfcommon::Session* game)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerGameEnded);

    metadataEditModel_->clear();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingStarted(rfcommon::Session* training)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerTrainingStarted);

}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingEnded(rfcommon::Session* training)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerTrainingEnded);

}

}
