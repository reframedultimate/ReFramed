#include "application/models/MetaDataEditModel.hpp"
#include "application/widgets/MetaDataEditWidget_Tournament.hpp"

#include "rfcommon/GameMetaData.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Tournament::MetaDataEditWidget_Tournament(MetaDataEditModel* model, QWidget* parent)
    : MetaDataEditWidget(model, parent)
    , lineEdit_name_(new QLineEdit)
    , lineEdit_website_(new QLineEdit)
    , layout_TOs_(new QVBoxLayout)
    , layout_sponsors_(new QVBoxLayout)
{
    setTitle("Tournament");

    QToolButton* addTO = new QToolButton;
    addTO->setText("+");
    layout_TOs_->addWidget(addTO);

    QToolButton* addSponsor = new QToolButton;
    addSponsor->setText("+");
    layout_sponsors_->addWidget(addSponsor);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(new QLabel("Tournlament name:"), 0, 0);
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
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
                mdata->asGame()->setTournamentName(text.toUtf8().constData());
    });
    connect(lineEdit_website_, &QLineEdit::textChanged, [this](const QString& text){
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
                mdata->asGame()->setTournamentWebsite(text.toUtf8().constData());
    });
    connect(addTO, &QToolButton::released, this, &MetaDataEditWidget_Tournament::onAddTOReleased);
    connect(addSponsor, &QToolButton::released, this, &MetaDataEditWidget_Tournament::onAddSponsorReleased);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Tournament::~MetaDataEditWidget_Tournament()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::addTOUI(const QString& name, const QString& social, const QString& pronouns)
{
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

    QGroupBox* g = new QGroupBox;
    g->setTitle(name.isEmpty() ? "TO #" + QString::number(layout_TOs_->count()) : name);
    g->setLayout(layout);

    QLayoutItem* addButtonItem = layout_TOs_->takeAt(layout_TOs_->count() - 1);
    layout_TOs_->addWidget(g);
    layout_TOs_->addItem(addButtonItem);

    updateSize();

    auto indexInLayout = [this](QGroupBox* g) -> int {
        for (int i = 0; i != layout_TOs_->count(); ++i)
        {
            QLayoutItem* item = layout_TOs_->itemAt(i);
            if (item->widget() == g)
                return i;
        }
        return -1;
    };

    // Remove TO
    connect(removeButton, &QToolButton::released, [this, g, indexInLayout] {
        int i = indexInLayout(g);

        ignoreSelf_ = true;
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
                mdata->asGame()->removeTournamentOrganizer(i);
        ignoreSelf_ = false;

        organizerWidgets_.erase(organizerWidgets_.begin() + i);

        QLayoutItem* item = layout_TOs_->takeAt(i);
        delete item->widget();
        delete item;
        updateSize();
    });

    // Change TO name
    connect(widgets.name, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);

        if (text.isEmpty())
            g->setTitle("TO #" + QString::number(i+1));
        else
            g->setTitle(text);

        ignoreSelf_ = true;
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
            {
                rfcommon::GameMetaData* m = mdata->asGame();
                m->setTournamentOrganizer(i,
                        text.toUtf8().constData(),
                        m->tournamentOrganizerSocial(i).cStr(),
                        m->tournamentOrganizerPronouns(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change TO social
    connect(widgets.social, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
            {
                rfcommon::GameMetaData* m = mdata->asGame();
                m->setTournamentOrganizer(i,
                        m->tournamentOrganizerName(i).cStr(),
                        text.toUtf8().constData(),
                        m->tournamentOrganizerPronouns(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change TO pronouns
    connect(widgets.pronouns, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
            {
                rfcommon::GameMetaData* m = mdata->asGame();
                m->setTournamentOrganizer(i,
                        m->tournamentOrganizerName(i).cStr(),
                        m->tournamentOrganizerSocial(i).cStr(),
                        text.toUtf8().constData());
            }
        ignoreSelf_ = false;
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::addSponsorUI(const QString& name, const QString& website)
{
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

    QGroupBox* g = new QGroupBox;
    g->setTitle(name.isEmpty() ? "Sponsor #" + QString::number(layout_sponsors_->count()) : name);
    g->setLayout(layout);

    QLayoutItem* addButtonItem = layout_sponsors_->takeAt(layout_sponsors_->count() - 1);
    layout_sponsors_->addWidget(g);
    layout_sponsors_->addItem(addButtonItem);

    updateSize();

    auto indexInLayout = [this](QGroupBox* g) -> int {
        for (int i = 0; i != layout_sponsors_->count(); ++i)
        {
            QLayoutItem* item = layout_sponsors_->itemAt(i);
            if (item->widget() == g)
                return i;
        }
        return -1;
    };

    // Remove sponsor
    connect(removeButton, &QToolButton::released, [this, g, indexInLayout] {
        int i = indexInLayout(g);

        ignoreSelf_ = true;
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
                mdata->asGame()->removeSponsor(i);
        ignoreSelf_ = false;

        sponsorWidgets_.erase(sponsorWidgets_.begin() + i);

        QLayoutItem* item = layout_sponsors_->takeAt(i);
        delete item->widget();
        delete item;
        updateSize();
    });

    // Change sponsor name
    connect(widgets.name, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);

        if (text.isEmpty())
            g->setTitle("Sponsor #" + QString::number(i+1));
        else
            g->setTitle(text);

        ignoreSelf_ = true;
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
            {
                rfcommon::GameMetaData* m = mdata->asGame();
                m->setSponsor(i,
                        text.toUtf8().constData(),
                        m->sponsorWebsite(i).cStr());
            }
        ignoreSelf_ = false;
    });

    // Change sponsor website
    connect(widgets.website, &QLineEdit::textChanged, [this, g, indexInLayout](const QString& text) {
        int i = indexInLayout(g);
        ignoreSelf_ = true;
        for (auto& mdata : model_->metaData())
            if (mdata->type() == rfcommon::MetaData::GAME)
            {
                rfcommon::GameMetaData* m = mdata->asGame();
                m->setSponsor(i,
                        m->sponsorWebsite(i).cStr(),
                        text.toUtf8().constData());
            }
        ignoreSelf_ = false;
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::onAddTOReleased()
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->addTournamentOrganizer("", "", "he/him");
    ignoreSelf_ = false;

    addTOUI("", "", "he/him");
}


// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::onAddSponsorReleased()
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metaData())
        if (mdata->type() == rfcommon::MetaData::GAME)
            mdata->asGame()->addSponsor("", "");
    ignoreSelf_ = false;

    addSponsorUI("", "");
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{
    QString tournamentName, tournamentWebsite;
    QStringList organizerNames, organizerSocials, organizerPronouns;
    QStringList sponsorNames, sponsorWebsites;

    bool first = true;
    for (auto& m : mdata)
    {
        switch (m->type())
        {
            case rfcommon::MetaData::GAME: {
                rfcommon::GameMetaData* g = m->asGame();

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

            case rfcommon::MetaData::TRAINING:
                break;
        }
        first = false;
    }

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
void MetaDataEditWidget_Tournament::onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{
    ignoreSelf_ = true;

    for (auto& m : mdata)
    {
        switch (m->type())
        {
            case rfcommon::MetaData::GAME: {
                rfcommon::GameMetaData* g = m->asGame();

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

            case rfcommon::MetaData::TRAINING:
                break;
        }
    }

    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Tournament::onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata) {}
void MetaDataEditWidget_Tournament::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetaDataEditWidget_Tournament::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Tournament::onMetaDataTournamentDetailsChanged()
{
    if (ignoreSelf_)
        return;
    onAdoptMetaData(model_->mappingInfo(), model_->metaData());
}
void MetaDataEditWidget_Tournament::onMetaDataEventDetailsChanged() {}
void MetaDataEditWidget_Tournament::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_Tournament::onMetaDataGameDetailsChanged() {}
void MetaDataEditWidget_Tournament::onMetaDataPlayerDetailsChanged() {}
void MetaDataEditWidget_Tournament::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Tournament::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
