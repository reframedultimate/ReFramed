#include "application/widgets/MetaDataEditWidget_Event.hpp"

#include "rfcommon/EventType.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>
#include <QComboBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Event::MetaDataEditWidget_Event(QWidget* parent)
    : MetaDataEditWidget(parent)
{
    setTitle("Event");

    QComboBox* bracketType = new QComboBox;
#define X(type, name) bracketType->addItem(name);
    EVENT_TYPE_LIST
#undef X

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Bracket Type:", bracketType);
    layout->addWidget(new QLineEdit);
    layout->addRow("Bracket URL:", new QLineEdit);

    contentWidget()->setLayout(layout);
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Event::~MetaDataEditWidget_Event()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::adoptMetaData()
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::overwriteMetaData()
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Event::onMetaDataTournamentDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataEventDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_Event::onMetaDataGameDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataPlayerDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Event::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
