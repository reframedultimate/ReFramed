#include "application/widgets/MetaDataEditWidget_Tournament.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Tournament::MetaDataEditWidget_Tournament(QWidget* parent)
    : MetaDataEditWidget(parent)
    , TOLayout_(new QVBoxLayout)
    , sponsorsLayout_(new QVBoxLayout)
{
    setTitle("Tournament");

    QToolButton* addTO = new QToolButton;
    addTO->setText("+");
    TOLayout_->addWidget(addTO);

    QToolButton* addSponsor = new QToolButton;
    addSponsor->setText("+");
    sponsorsLayout_->addWidget(addSponsor);

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Tournament Name:", new QLineEdit);
    layout->addRow("Website:", new QLineEdit);
    layout->addRow("Organizers:", TOLayout_);
    layout->addRow("Sponsors:", sponsorsLayout_);

    contentWidget()->setLayout(layout);
    updateSize();

    connect(addTO, &QToolButton::released, this, &MetaDataEditWidget_Tournament::onAddTOReleased);
    connect(addSponsor, &QToolButton::released, this, &MetaDataEditWidget_Tournament::onAddSponsorReleased);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Tournament::~MetaDataEditWidget_Tournament()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::onAddTOReleased()
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", new QLineEdit);
    layout->addRow("Social:", new QLineEdit);
    layout->addRow("Pronouns:", new QLineEdit("he/him"));
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle("TO #" + QString::number(TOLayout_->count()));
    g->setLayout(layout);

    QLayoutItem* addButtonItem = TOLayout_->takeAt(TOLayout_->count() - 1);
    TOLayout_->addWidget(g);
    TOLayout_->addItem(addButtonItem);

    updateSize();

    connect(removeButton, &QToolButton::released, [this, g] {
        for (int i = 0; i != TOLayout_->count(); ++i)
        {
            QLayoutItem* item = TOLayout_->itemAt(i);
            if (item->widget() == g)
            {
                item = TOLayout_->takeAt(i);
                delete item->widget();
                delete item;
                updateSize();
                return;
            }
        }
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::onAddSponsorReleased()
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", new QLineEdit);
    layout->addRow("Website:", new QLineEdit);
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle("Sponsor #" + QString::number(sponsorsLayout_->count()));
    g->setLayout(layout);

    QLayoutItem* addButtonItem = sponsorsLayout_->takeAt(sponsorsLayout_->count() - 1);
    sponsorsLayout_->addWidget(g);
    sponsorsLayout_->addItem(addButtonItem);

    updateSize();

    connect(removeButton, &QToolButton::released, [this, g] {
        for (int i = 0; i != TOLayout_->count(); ++i)
        {
            QLayoutItem* item = sponsorsLayout_->itemAt(i);
            if (item->widget() == g)
            {
                item = sponsorsLayout_->takeAt(i);
                delete item->widget();
                delete item;
                updateSize();
                return;
            }
        }
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) {}
void MetaDataEditWidget_Tournament::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Tournament::onMetaDataPlayerNameChanged(int fighterIdx, const char* name) {}
void MetaDataEditWidget_Tournament::onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) {}
void MetaDataEditWidget_Tournament::onMetaDataTournamentNameChanged(const char* name) {}
void MetaDataEditWidget_Tournament::onMetaDataEventNameChanged(const char* name) {}
void MetaDataEditWidget_Tournament::onMetaDataRoundNameChanged(const char* name) {}
void MetaDataEditWidget_Tournament::onMetaDataCommentatorsChanged(const rfcommon::SmallVector<rfcommon::String, 2>& names) {}
void MetaDataEditWidget_Tournament::onMetaDataSetNumberChanged(rfcommon::SetNumber number) {}
void MetaDataEditWidget_Tournament::onMetaDataGameNumberChanged(rfcommon::GameNumber number) {}
void MetaDataEditWidget_Tournament::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) {}
void MetaDataEditWidget_Tournament::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Tournament::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) {}

}
