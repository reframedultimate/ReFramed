#include "application/models/MetadataEditModel.hpp"
#include "application/widgets/MetadataEditWidget_Commentators.hpp"

#include "rfcommon/GameMetadata.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetadataEditWidget_Commentators::MetadataEditWidget_Commentators(MetadataEditModel* model, QWidget* parent)
    : MetadataEditWidget(model, parent)
    , commentatorsLayout_(new QVBoxLayout)
{
    setTitle("Commentators");

    QToolButton* addCommentator = new QToolButton;
    addCommentator->setText("+");
    commentatorsLayout_->addWidget(addCommentator);

    contentWidget()->setLayout(commentatorsLayout_);
    updateSize();

    connect(addCommentator, &QToolButton::released, this, &MetadataEditWidget_Commentators::onAddCommentatorReleased);
}

// ----------------------------------------------------------------------------
MetadataEditWidget_Commentators::~MetadataEditWidget_Commentators()
{}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Commentators::addCommentatorUI(const QString& name, const QString& social, const QString& pronouns)
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    commentatorWidgets_.push_back({});
    auto& widgets = commentatorWidgets_.back();
    widgets.name = new QLineEdit(name);
    widgets.social = new QLineEdit(social);
    widgets.pronouns = new QLineEdit(pronouns);

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", widgets.name);
    layout->addRow("Social:", widgets.social);
    layout->addRow("Pronouns:", widgets.pronouns);
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle(name.isEmpty() ? "Commentator #" + QString::number(commentatorsLayout_->count()) : name);
    g->setLayout(layout);

    QLayoutItem* addButtonItem = commentatorsLayout_->takeAt(commentatorsLayout_->count() - 1);
    commentatorsLayout_->addWidget(g);
    commentatorsLayout_->addItem(addButtonItem);

    updateSize();

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
        for (auto& mdata : model_->metadata())
            if (mdata == rfcommon::Metadata::GAME)
                mdata->asGame()->removeCommentator(i);
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

        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata == rfcommon::Metadata::GAME)
            {
                rfcommon::GameMetadata* g = mdata->asGame();
                g->setCommentator(i,
                        text.toUtf8().constData(),
                        g->commentatorSocial(i).cStr(),
                        g->commentatorPronouns(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change social
    connect(widgets.social, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata == rfcommon::Metadata::GAME)
            {
                rfcommon::GameMetadata* g = mdata->asGame();
                g->setCommentator(i,
                        g->commentatorName(i).cStr(),
                        text.toUtf8().constData(),
                        g->commentatorPronouns(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change pronouns
    connect(widgets.pronouns, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata == rfcommon::Metadata::GAME)
            {
                rfcommon::GameMetadata* g = mdata->asGame();
                g->setCommentator(i,
                        g->commentatorName(i).cStr(),
                        g->commentatorSocial(i).cStr(),
                        text.toUtf8().constData());
            }
        ignoreSelf_ = false;
    });
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Commentators::onAddCommentatorReleased()
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->addCommentator("", "");
    ignoreSelf_ = false;

    onMetadataCommentatorsChanged();
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Commentators::onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    for (auto& m : mdata)
    {
        switch (m->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = m->asGame();
/*
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
                            g->commentatorPronouns(i).cStr());*/
            } break;

            case rfcommon::Metadata::TRAINING:
                break;
        }
    }
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Commentators::onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    ignoreSelf_ = true;

    for (auto& m : mdata)
    {
        switch (m->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = m->asGame();
    /*
                while (g->commentatorCount())
                    g->removeCommentator(0);
                for (int i = 0; i != commentatorWidgets_.size(); ++i)
                    g->addCommentator(
                            commentatorWidgets_[i].name->text().toUtf8().constData(),
                            commentatorWidgets_[i].social->text().toUtf8().constData(),
                            commentatorWidgets_[i].pronouns->text().toUtf8().constData());*/
            } break;

            case rfcommon::Metadata::TRAINING:
                break;
        }
    }

    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Commentators::onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata)
{
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Commentators::onNextGameStarted()
{
}

void MetadataEditWidget_Commentators::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetadataEditWidget_Commentators::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetadataEditWidget_Commentators::onMetadataTournamentDetailsChanged() {}
void MetadataEditWidget_Commentators::onMetadataEventDetailsChanged() {}
void MetadataEditWidget_Commentators::onMetadataCommentatorsChanged()
{
    if (ignoreSelf_)
        return;
    onAdoptMetadata(model_->mappingInfo(), model_->metadata());
}
void MetadataEditWidget_Commentators::onMetadataGameDetailsChanged() {}
void MetadataEditWidget_Commentators::onMetadataPlayerDetailsChanged() {}
void MetadataEditWidget_Commentators::onMetadataWinnerChanged(int winnerPlayerIdx) {}
void MetadataEditWidget_Commentators::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
