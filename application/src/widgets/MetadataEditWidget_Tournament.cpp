#include "application/models/MetadataEditModel.hpp"
#include "application/widgets/MetadataEditWidget_Tournament.hpp"

#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Profiler.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetadataEditWidget_Tournament::MetadataEditWidget_Tournament(MetadataEditModel* model, QWidget* parent)
    : MetadataEditWidget(model, parent)
    , lineEdit_name_(new QLineEdit)
    , lineEdit_website_(new QLineEdit)
    , layout_TOs_(new QVBoxLayout)
    , layout_sponsors_(new QVBoxLayout)
{
    setTitle(QIcon::fromTheme(""), "Tournament");

    QToolButton* addTO = new QToolButton;
    addTO->setText("+");
    layout_TOs_->addWidget(addTO);

    QToolButton* addSponsor = new QToolButton;
    addSponsor->setText("+");
    layout_sponsors_->addWidget(addSponsor);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(new QLabel("Tournament name:"), 0, 0);
    layout->addWidget(lineEdit_name_, 0, 1);
    layout->addWidget(new QLabel("Website:"), 1, 0);
    layout->addWidget(lineEdit_website_, 1, 1);
    layout->addWidget(new QLabel("Tournament Organizers:"), 2, 0, 1, 2);
    layout->addLayout(layout_TOs_, 3, 0, 1, 2);
    layout->addWidget(new QLabel("Sponsors:"), 4, 0, 1, 2);
    layout->addLayout(layout_sponsors_, 5, 0, 1, 2);

    contentWidget()->setLayout(layout);
    updateSize();

    connect(lineEdit_name_, &QLineEdit::textChanged, [this](const QString& text) {
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
                mdata->asGame()->setTournamentName(text.toUtf8().constData());
        ignoreSelf_ = false;
    });
    connect(lineEdit_website_, &QLineEdit::textChanged, [this](const QString& text){
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
                mdata->asGame()->setTournamentWebsite(text.toUtf8().constData());
        ignoreSelf_ = false;
    });
    connect(addTO, &QToolButton::released, this, &MetadataEditWidget_Tournament::onAddTOReleased);
    connect(addSponsor, &QToolButton::released, this, &MetadataEditWidget_Tournament::onAddSponsorReleased);
}

// ----------------------------------------------------------------------------
MetadataEditWidget_Tournament::~MetadataEditWidget_Tournament()
{}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Tournament::addTOUI(const QString& name, const QString& social, const QString& pronouns)
{
    PROFILE(MetadataEditWidget_Tournament, addTOUI);

    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    organizerWidgets_.push_back({});
    auto& widgets = organizerWidgets_.back();
    widgets.name = new QLineEdit(name);
    widgets.social = new QLineEdit(social);
    widgets.pronouns = new QLineEdit(pronouns);

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", widgets.name);
    layout->addRow("Social:", widgets.social);
    layout->addRow("Pronouns:", widgets.pronouns);
    layout->addRow(removeButton);

    QGroupBox* gb = new QGroupBox;
    gb->setTitle(name.isEmpty() || name == "*" ? "TO #" + QString::number(layout_TOs_->count()) : name);
    gb->setLayout(layout);

    QLayoutItem* addButtonItem = layout_TOs_->takeAt(layout_TOs_->count() - 1);
    layout_TOs_->addWidget(gb);
    layout_TOs_->addItem(addButtonItem);

    updateSize();

    auto indexInLayout = [this](QGroupBox* gb) -> int {
        for (int i = 0; i != layout_TOs_->count(); ++i)
        {
            QLayoutItem* item = layout_TOs_->itemAt(i);
            if (item->widget() == gb)
                return i;
        }
        return -1;
    };

    // Remove TO
    connect(removeButton, &QToolButton::released, [this, gb, indexInLayout] {
        int i = indexInLayout(gb);

        // Update models
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
                if (i < mdata->asGame()->tournamentOrganizerCount())  // In the case of multiple replays, there could be less in one than in the other
                    mdata->asGame()->removeTournamentOrganizer(i);
        ignoreSelf_ = false;

        // Update UI
        organizerWidgets_.erase(organizerWidgets_.begin() + i);

        QLayoutItem* item = layout_TOs_->takeAt(i);
        delete item->widget();
        delete item;
        updateSize();
    });

    // Change TO name
    connect(widgets.name, &QLineEdit::textChanged, [this, gb, indexInLayout](const QString& text) {
        int i = indexInLayout(gb);

        if (text.isEmpty())
            gb->setTitle("TO #" + QString::number(i+1));
        else
            gb->setTitle(text);

        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
            {
                // In the case of multiple replays, if one replay has a gap,
                // we just add an empty commentator in its place
                rfcommon::GameMetadata* g = mdata->asGame();
                while (i >= g->tournamentOrganizerCount())
                    g->addTournamentOrganizer("", "", "");
                g->setTournamentOrganizer(i,
                        text.toUtf8().constData(),
                        g->tournamentOrganizerSocial(i).cStr(),
                        g->tournamentOrganizerPronouns(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change TO social
    connect(widgets.social, &QLineEdit::textChanged, [this, gb, indexInLayout](const QString& text) {
        int i = indexInLayout(gb);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
            {
                // In the case of multiple replays, if one replay has a gap,
                // we just add an empty commentator in its place
                rfcommon::GameMetadata* g = mdata->asGame();
                while (i >= g->tournamentOrganizerCount())
                    g->addTournamentOrganizer("", "", "");
                g->setTournamentOrganizer(i,
                        g->tournamentOrganizerName(i).cStr(),
                        text.toUtf8().constData(),
                        g->tournamentOrganizerPronouns(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change TO pronouns
    connect(widgets.pronouns, &QLineEdit::textChanged, [this, gb, indexInLayout](const QString& text) {
        int i = indexInLayout(gb);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
            {
                // In the case of multiple replays, if one replay has a gap,
                // we just add an empty commentator in its place
                rfcommon::GameMetadata* g = mdata->asGame();
                while (i >= g->tournamentOrganizerCount())
                    g->addTournamentOrganizer("", "", "");
                g->setTournamentOrganizer(i,
                        g->tournamentOrganizerName(i).cStr(),
                        g->tournamentOrganizerSocial(i).cStr(),
                        text.toUtf8().constData());
            }
        ignoreSelf_ = false;
    });
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Tournament::addSponsorUI(const QString& name, const QString& website)
{
    PROFILE(MetadataEditWidget_Tournament, addSponsorUI);

    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    sponsorWidgets_.push_back({});
    auto& widgets = sponsorWidgets_.back();
    widgets.name = new QLineEdit(name);
    widgets.website = new QLineEdit(website);

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", widgets.name);
    layout->addRow("Website:", widgets.website);
    layout->addRow(removeButton);

    QGroupBox* gb = new QGroupBox;
    gb->setTitle(name.isEmpty() || name == "*" ? "Sponsor #" + QString::number(layout_sponsors_->count()) : name);
    gb->setLayout(layout);

    QLayoutItem* addButtonItem = layout_sponsors_->takeAt(layout_sponsors_->count() - 1);
    layout_sponsors_->addWidget(gb);
    layout_sponsors_->addItem(addButtonItem);

    updateSize();

    auto indexInLayout = [this](QGroupBox* gb) -> int {
        for (int i = 0; i != layout_sponsors_->count(); ++i)
        {
            QLayoutItem* item = layout_sponsors_->itemAt(i);
            if (item->widget() == gb)
                return i;
        }
        return -1;
    };

    // Remove sponsor
    connect(removeButton, &QToolButton::released, [this, gb, indexInLayout] {
        int i = indexInLayout(gb);

        // Update models
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
                if (i < mdata->asGame()->sponsorCount())  // In the case of multiple replays, there could be less in one than in the other
                    mdata->asGame()->removeSponsor(i);
        ignoreSelf_ = false;

        // Update UI
        sponsorWidgets_.erase(sponsorWidgets_.begin() + i);

        QLayoutItem* item = layout_sponsors_->takeAt(i);
        delete item->widget();
        delete item;
        updateSize();
    });

    // Change sponsor name
    connect(widgets.name, &QLineEdit::textChanged, [this, gb, indexInLayout](const QString& text) {
        int i = indexInLayout(gb);

        if (text.isEmpty())
            gb->setTitle("Sponsor #" + QString::number(i+1));
        else
            gb->setTitle(text);

        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
            {
                // In the case of multiple replays, if one replay has a gap,
                // we just add an empty commentator in its place
                rfcommon::GameMetadata* g = mdata->asGame();
                while (i >= g->sponsorCount())
                    g->addSponsor("", "");
                g->setSponsor(i,
                        text.toUtf8().constData(),
                        g->sponsorWebsite(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change sponsor website
    connect(widgets.website, &QLineEdit::textChanged, [this, gb, indexInLayout](const QString& text) {
        int i = indexInLayout(gb);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metadata())
            if (mdata->type() == rfcommon::Metadata::GAME)
            {
                // In the case of multiple replays, if one replay has a gap,
                // we just add an empty commentator in its place
                rfcommon::GameMetadata* g = mdata->asGame();
                while (i >= g->sponsorCount())
                    g->addSponsor("", "");
                g->setSponsor(i,
                        g->sponsorWebsite(i).cStr(),
                        text.toUtf8().constData());
            }
        ignoreSelf_ = false;
    });
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Tournament::onAddTOReleased()
{
    PROFILE(MetadataEditWidget_Tournament, onAddTOReleased);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->addTournamentOrganizer("", "", "");
    ignoreSelf_ = false;

    addTOUI("", "", "");
}


// ----------------------------------------------------------------------------
void MetadataEditWidget_Tournament::onAddSponsorReleased()
{
    PROFILE(MetadataEditWidget_Tournament, onAddSponsorReleased);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->addSponsor("", "");
    ignoreSelf_ = false;

    addSponsorUI("", "");
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Tournament::onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Tournament, onAdoptMetadata);

    QString tournamentName, tournamentWebsite;
    QStringList organizerNames, organizerSocials, organizerPronouns;
    QStringList sponsorNames, sponsorWebsites;

    bool first = true;
    for (auto& m : mdata)
    {
        switch (m->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = m->asGame();

                if (first)
                {
                    tournamentName = QString::fromUtf8(g->tournamentName().cStr());
                    tournamentWebsite = QString::fromUtf8(g->tournamentWebsite().cStr());
                    for (int i = 0; i != g->tournamentOrganizerCount(); ++i)
                    {
                        organizerNames += QString::fromUtf8(g->tournamentOrganizerName(i).cStr());
                        organizerSocials += QString::fromUtf8(g->tournamentOrganizerSocial(i).cStr());
                        organizerPronouns += QString::fromUtf8(g->tournamentOrganizerPronouns(i).cStr());
                    }
                    for (int i = 0; i != g->sponsorCount(); ++i)
                    {
                        sponsorNames += QString::fromUtf8(g->sponsorName(i).cStr());
                        sponsorWebsites += QString::fromUtf8(g->sponsorWebsite(i).cStr());
                    }
                }
                else
                {
                    if (tournamentName != QString::fromUtf8(g->tournamentName().cStr()))
                        tournamentName = "*";
                    if (tournamentWebsite != QString::fromUtf8(g->tournamentWebsite().cStr()))
                        tournamentWebsite = "*";

                    // If any replays have missing TOs, mark them as "*"
                    for (int i = 0; i != organizerNames.size(); ++i)
                        if (i >= g->tournamentOrganizerCount())
                        {
                            organizerNames[i] = "*";
                            organizerSocials[i] = "*";
                            organizerPronouns[i] = "*";
                        }

                    // If existing TOs are different, mark them as "*"
                    for (int i = 0; i != g->tournamentOrganizerCount(); ++i)
                    {
                        if (i >= organizerNames.size())
                            organizerNames += "*";
                        else if (organizerNames[i] != QString::fromUtf8(g->tournamentOrganizerName(i).cStr()))
                            organizerNames[i] = "*";

                        if (i >= organizerSocials.size())
                            organizerSocials += "*";
                        else if (organizerSocials[i] != QString::fromUtf8(g->tournamentOrganizerSocial(i).cStr()))
                            organizerSocials[i] = "*";

                        if (i >= organizerPronouns.size())
                            organizerPronouns += "*";
                        else if (organizerPronouns[i] != QString::fromUtf8(g->tournamentOrganizerPronouns(i).cStr()))
                            organizerPronouns[i] = "*";
                    }

                    // If any replays have missing TOs, mark them as "*"
                    for (int i = 0; i != sponsorNames.size(); ++i)
                        if (i >= g->sponsorCount())
                        {
                            sponsorNames[i] = "*";
                            sponsorWebsites[i] = "*";
                        }

                    // If existing sponsors are different, mark them as "*"
                    for (int i = 0; i != g->sponsorCount(); ++i)
                    {
                        if (i >= sponsorNames.size())
                            sponsorNames += "*";
                        else if (sponsorNames[i] != QString::fromUtf8(g->sponsorName(i).cStr()))
                            sponsorNames[i] = "*";

                        if (i >= sponsorWebsites.size())
                            sponsorWebsites += "*";
                        else if (sponsorWebsites[i] != QString::fromUtf8(g->sponsorWebsite(i).cStr()))
                            sponsorWebsites[i] = "*";
                    }
                }
            } break;

            case rfcommon::Metadata::TRAINING:
                break;
        }
        first = false;
    }

    const QSignalBlocker blockName(lineEdit_name_);
    const QSignalBlocker blockWebsite(lineEdit_website_);
    lineEdit_name_->setText(tournamentName);
    lineEdit_website_->setText(tournamentWebsite);

    while (layout_TOs_->count() > 1)
    {
        QLayoutItem* item = layout_TOs_->takeAt(0);
        delete item->widget();
        delete item;
    }
    for (int i = 0; i != organizerNames.size(); ++i)
        addTOUI(organizerNames[i], organizerSocials[i], organizerPronouns[i]);

    while (layout_sponsors_->count() > 1)
    {
        QLayoutItem* item = layout_sponsors_->takeAt(0);
        delete item->widget();
        delete item;
    }
    for (int i = 0; i != sponsorNames.size(); ++i)
        addSponsorUI(sponsorNames[i], sponsorWebsites[i]);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Tournament::onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Tournament, onOverwriteMetadata);

    ignoreSelf_ = true;

    for (auto& m : mdata)
    {
        switch (m->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = m->asGame();

                g->setTournamentName(lineEdit_name_->text().toUtf8().constData());
                g->setTournamentWebsite(lineEdit_website_->text().toUtf8().constData());

                while (g->tournamentOrganizerCount())
                    g->removeTournamentOrganizer(0);
                for (int i = 0; i != organizerWidgets_.size(); ++i)
                    g->addTournamentOrganizer(
                            organizerWidgets_[i].name->text().toUtf8().constData(),
                            organizerWidgets_[i].social->text().toUtf8().constData(),
                            organizerWidgets_[i].pronouns->text().toUtf8().constData());

                while (g->sponsorCount())
                    g->removeSponsor(0);
                for (int i = 0; i != sponsorWidgets_.size(); ++i)
                    g->addSponsor(
                            sponsorWidgets_[i].name->text().toUtf8().constData(),
                            sponsorWidgets_[i].website->text().toUtf8().constData());
            } break;

            case rfcommon::Metadata::TRAINING:
                break;
        }
    }

    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Tournament::onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata) {}
void MetadataEditWidget_Tournament::onNextGameStarted(){}

void MetadataEditWidget_Tournament::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetadataEditWidget_Tournament::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetadataEditWidget_Tournament::onMetadataTournamentDetailsChanged()
{
    PROFILE(MetadataEditWidget_Tournament, onMetadataTournamentDetailsChanged);

    if (ignoreSelf_)
        return;
    onAdoptMetadata(model_->mappingInfo(), model_->metadata());
}
void MetadataEditWidget_Tournament::onMetadataEventDetailsChanged() {}
void MetadataEditWidget_Tournament::onMetadataCommentatorsChanged() {}
void MetadataEditWidget_Tournament::onMetadataGameDetailsChanged() {}
void MetadataEditWidget_Tournament::onMetadataPlayerDetailsChanged() {}
void MetadataEditWidget_Tournament::onMetadataWinnerChanged(int winnerPlayerIdx) {}
void MetadataEditWidget_Tournament::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
