#include "application/ui_MetaDataEditWidget_Game.h"
#include "application/widgets/MetaDataEditWidget_Game.hpp"

#include "rfcommon/Round.hpp"
#include "rfcommon/SetFormat.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::MetaDataEditWidget_Game(QWidget* parent)
    : MetaDataEditWidget(parent)
    , ui_(new Ui::MetaDataEditWidget_Game)
{
    ui_->setupUi(contentWidget());

#define X(type, shortstr, longstr) if (strlen(shortstr) > 0) ui_->comboBox_roundType->addItem(longstr);
    ROUND_TYPES_LIST
#undef X
#define X(type, shortstr, longstr) ui_->comboBox_setFormat->addItem(longstr);
    SET_FORMAT_LIST
#undef X

    updateSize();

    setTitle("Game");
    setExpanded(true);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::~MetaDataEditWidget_Game()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::adoptMetaData()
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::overwriteMetaData()
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Game::onMetaDataTournamentDetailsChanged() {}
void MetaDataEditWidget_Game::onMetaDataEventDetailsChanged() {}
void MetaDataEditWidget_Game::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_Game::onMetaDataGameDetailsChanged() {}
void MetaDataEditWidget_Game::onMetaDataPlayerDetailsChanged() {}
void MetaDataEditWidget_Game::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Game::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
