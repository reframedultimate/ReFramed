#include "application/widgets/MetaDataEditWidget_Commentators.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Commentators::MetaDataEditWidget_Commentators(QWidget* parent)
    : MetaDataEditWidget(parent)
    , commentatorsLayout_(new QVBoxLayout)
{
    setTitle("Commentators");

    QToolButton* addCommentator = new QToolButton;
    addCommentator->setText("+");
    commentatorsLayout_->addWidget(addCommentator);

    contentWidget()->setLayout(commentatorsLayout_);
    updateSize();

    connect(addCommentator, &QToolButton::released, this, &MetaDataEditWidget_Commentators::onAddCommentatorReleased);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Commentators::~MetaDataEditWidget_Commentators()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onAddCommentatorReleased()
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", new QLineEdit);
    layout->addRow("Social:", new QLineEdit);
    layout->addRow("Pronouns:", new QLineEdit("he/him"));
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle("Commentator #" + QString::number(commentatorsLayout_->count()));
    g->setLayout(layout);

    QLayoutItem* addButtonItem = commentatorsLayout_->takeAt(commentatorsLayout_->count() - 1);
    commentatorsLayout_->addWidget(g);
    commentatorsLayout_->addItem(addButtonItem);

    updateSize();

    connect(removeButton, &QToolButton::released, [this, g] {
        for (int i = 0; i != commentatorsLayout_->count(); ++i)
        {
            QLayoutItem* item = commentatorsLayout_->itemAt(i);
            if (item->widget() == g)
            {
                item = commentatorsLayout_->takeAt(i);
                delete item->widget();
                delete item;
                updateSize();
                return;
            }
        }
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) {}
void MetaDataEditWidget_Commentators::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Commentators::onMetaDataPlayerNameChanged(int fighterIdx, const char* name) {}
void MetaDataEditWidget_Commentators::onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) {}
void MetaDataEditWidget_Commentators::onMetaDataTournamentNameChanged(const char* name) {}
void MetaDataEditWidget_Commentators::onMetaDataEventNameChanged(const char* name) {}
void MetaDataEditWidget_Commentators::onMetaDataRoundNameChanged(const char* name) {}
void MetaDataEditWidget_Commentators::onMetaDataCommentatorsChanged(const rfcommon::SmallVector<rfcommon::String, 2>& names) {}
void MetaDataEditWidget_Commentators::onMetaDataSetNumberChanged(rfcommon::SetNumber number) {}
void MetaDataEditWidget_Commentators::onMetaDataGameNumberChanged(rfcommon::GameNumber number) {}
void MetaDataEditWidget_Commentators::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) {}
void MetaDataEditWidget_Commentators::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Commentators::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) {}

}
