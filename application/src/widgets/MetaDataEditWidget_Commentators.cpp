#include "application/models/MetaDataEditModel.hpp"
#include "application/widgets/MetaDataEditWidget_Commentators.hpp"

#include "rfcommon/GameMetaData.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Commentators::MetaDataEditWidget_Commentators(MetaDataEditModel* model, QWidget* parent)
    : MetaDataEditWidget(model, parent)
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
void MetaDataEditWidget_Commentators::addCommentatorUI(const char* name, const char* social, const char* pronouns)
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    commentatorWidgets_.push_back({});
    auto& widgets = commentatorWidgets_.back();
    widgets.name = new QLineEdit(QString::fromUtf8(name));
    widgets.social = new QLineEdit(QString::fromUtf8(social));
    widgets.pronouns = new QLineEdit(QString::fromUtf8(pronouns));

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", widgets.name);
    layout->addRow("Social:", widgets.social);
    layout->addRow("Pronouns:", widgets.pronouns);
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle(strlen(name) > 0 ? QString::fromUtf8(name) : "Commentator #" + QString::number(commentatorsLayout_->count()));
    g->setLayout(layout);

    QLayoutItem* addButtonItem = commentatorsLayout_->takeAt(commentatorsLayout_->count() - 1);
    commentatorsLayout_->addWidget(g);
    commentatorsLayout_->addItem(addButtonItem);

    updateSize();

    ignoreSelf_ = true;
    if (model_->metaData() && model_->metaData()->type() == rfcommon::MetaData::GAME)
        model_->metaData()->asGame()->addCommentator(name, social, pronouns);
    ignoreSelf_ = false;

    auto indexInLayout = [this](QGroupBox* g) -> int {
        for (int i = 0; i != commentatorsLayout_->count(); ++i)
        {
            QLayoutItem* item = commentatorsLayout_->itemAt(i);
            if (item->widget() == g)
                return i;
        }
        return -1;
    };

    // Remove commentator
    connect(removeButton, &QToolButton::released, [this, g, indexInLayout] {
        int i = indexInLayout(g);

        ignoreSelf_ = true;
        if (model_->metaData() && model_->metaData()->type() == rfcommon::MetaData::GAME)
            model_->metaData()->asGame()->removeCommentator(i);
        ignoreSelf_ = false;

        commentatorWidgets_.erase(commentatorWidgets_.begin() + i);

        QLayoutItem* item = commentatorsLayout_->takeAt(i);
        delete item->widget();
        delete item;
        updateSize();
    });

    // Change name
    connect(widgets.name, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);

        if (text.isEmpty())
            g->setTitle("Commentator #" + QString::number(i+1));
        else
            g->setTitle(text);

        if (model_->metaData() == nullptr || model_->metaData()->type() != rfcommon::MetaData::GAME)
            return;

        rfcommon::GameMetaData* m = model_->metaData()->asGame();
        ignoreSelf_ = true;
        m->setCommentator(i,
                text.toUtf8().constData(),
                m->commentatorSocial(i).cStr(),
                m->commentatorPronouns(i).cStr());
        ignoreSelf_ = false;
    });

    // Change social
    connect(widgets.social, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        if (model_->metaData() == nullptr || model_->metaData()->type() != rfcommon::MetaData::GAME)
            return;

        int i = indexInLayout(g);
        rfcommon::GameMetaData* m = model_->metaData()->asGame();
        ignoreSelf_ = true;
        m->setCommentator(i,
                m->commentatorName(i).cStr(),
                text.toUtf8().constData(),
                m->commentatorPronouns(i).cStr());
        ignoreSelf_ = false;
    });

    // Change pronouns
    connect(widgets.pronouns, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        if (model_->metaData() == nullptr || model_->metaData()->type() != rfcommon::MetaData::GAME)
            return;

        int i = indexInLayout(g);
        rfcommon::GameMetaData* m = model_->metaData()->asGame();
        ignoreSelf_ = true;
        m->setCommentator(i,
                m->commentatorName(i).cStr(),
                m->commentatorSocial(i).cStr(),
                text.toUtf8().constData());
        ignoreSelf_ = false;
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onAddCommentatorReleased()
{
    addCommentatorUI("", "", "he/him");
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onAdoptMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    switch (mdata->type())
    {
        case rfcommon::MetaData::GAME: {
            rfcommon::GameMetaData* g = mdata->asGame();

            while (commentatorsLayout_->count() > 1)
            {
                QLayoutItem* item = commentatorsLayout_->takeAt(0);
                delete item->widget();
                delete item;
            }

            for (int i = 0; i != g->commentatorCount(); ++i)
                addCommentatorUI(
                        g->commentatorName(i).cStr(),
                        g->commentatorSocial(i).cStr(),
                        g->commentatorPronouns(i).cStr());
        } break;

        case rfcommon::MetaData::TRAINING:
            break;
    }
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onOverwriteMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    ignoreSelf_ = true;

    switch (mdata->type())
    {
        case rfcommon::MetaData::GAME: {
            rfcommon::GameMetaData* g = mdata->asGame();

            while (g->commentatorCount())
                g->removeCommentator(0);
            for (int i = 0; i != commentatorWidgets_.size(); ++i)
                g->addCommentator(
                        commentatorWidgets_[i].name->text().toUtf8().constData(),
                        commentatorWidgets_[i].social->text().toUtf8().constData(),
                        commentatorWidgets_[i].pronouns->text().toUtf8().constData());
        } break;

        case rfcommon::MetaData::TRAINING:
            break;
    }

    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onMetaDataCleared(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{

}

void MetaDataEditWidget_Commentators::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetaDataEditWidget_Commentators::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Commentators::onMetaDataTournamentDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataEventDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataCommentatorsChanged()
{
    if (ignoreSelf_)
        return;
    onAdoptMetaData(model_->mappingInfo(), model_->metaData());
}
void MetaDataEditWidget_Commentators::onMetaDataGameDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataPlayerDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Commentators::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
