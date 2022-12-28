#include "application/models/ActiveSessionManager.hpp"
#include "application/models/MetaDataEditModel.hpp"
#include "application/views/ActiveSessionView.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/widgets/CollapsibleSplitter.hpp"
#include "application/widgets/MetaDataEditWidget_AutoAssociateVideo.hpp"
#include "application/widgets/MetaDataEditWidget_Commentators.hpp"
#include "application/widgets/MetaDataEditWidget_Event.hpp"
#include "application/widgets/MetaDataEditWidget_Game.hpp"
#include "application/widgets/MetaDataEditWidget_Tournament.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
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
    , metaDataEditModel_(new MetaDataEditModel)
{
    MetaDataEditWidget_AutoAssociateVideo* assocVideo = new MetaDataEditWidget_AutoAssociateVideo(metaDataEditModel_.get(), activeSessionManager_);
    MetaDataEditWidget_Tournament* tournament = new MetaDataEditWidget_Tournament(metaDataEditModel_.get());
    MetaDataEditWidget_Commentators* commentators = new MetaDataEditWidget_Commentators(metaDataEditModel_.get());
    MetaDataEditWidget_Event* event = new MetaDataEditWidget_Event(metaDataEditModel_.get());
    MetaDataEditWidget_Game* game = new MetaDataEditWidget_Game(metaDataEditModel_.get(), playerDetails);

    event->setExpanded(true);
    game->setExpanded(true);

    QVBoxLayout* metaDataEditLayout = new QVBoxLayout;
    metaDataEditLayout->addWidget(assocVideo);
    metaDataEditLayout->addWidget(tournament);
    metaDataEditLayout->addWidget(commentators);
    metaDataEditLayout->addWidget(event);
    metaDataEditLayout->addWidget(game);
    metaDataEditLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QWidget* metaDataEditContents = new QWidget;
    metaDataEditContents->setLayout(metaDataEditLayout);

    scrollArea_ = new ScrollAreaWithSizeHint;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setWidget(metaDataEditContents);

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
    // metaDataEditModel_. Have to delete them explicitly, otherwise the model
    // is deleted before the widgets are deleted.
    delete scrollArea_;

    activeSessionManager_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ActiveSessionView::toggleSideBar()
{
    splitter_->toggleCollapse();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameStarted(rfcommon::Session* game)
{
    auto* map = game->tryGetMappingInfo();
    auto* mdata = game->tryGetMetaData();
    assert(map);
    assert(mdata);

    metaDataEditModel_->setAndOverwrite({map}, {mdata});
    metaDataEditModel_->startNextGame();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameEnded(rfcommon::Session* game)
{
    metaDataEditModel_->clear();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingStarted(rfcommon::Session* training)
{
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingEnded(rfcommon::Session* training)
{
}

}
