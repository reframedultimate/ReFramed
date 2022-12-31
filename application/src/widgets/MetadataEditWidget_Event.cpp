#include "application/models/MetadataEditModel.hpp"
#include "application/widgets/MetadataEditWidget_Event.hpp"

#include "rfcommon/BracketType.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Profiler.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>
#include <QComboBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetadataEditWidget_Event::MetadataEditWidget_Event(MetadataEditModel* model, QWidget* parent)
    : MetadataEditWidget(model, parent)
    , comboBox_bracketType_(new QComboBox)
    , label_bracketURL_(new QLabel("Bracket URL:"))
    , lineEdit_bracketURL_(new QLineEdit)
    , lineEdit_otherBracketType_(new QLineEdit)
{
    setTitle(QIcon::fromTheme(""), "Event");

#define X(type, name) comboBox_bracketType_->addItem(name);
    BRACKET_TYPE_LIST
#undef X
    comboBox_bracketType_->setCurrentIndex(rfcommon::BracketType::FRIENDLIES);
    comboBox_bracketType_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    label_bracketURL_->setVisible(false);
    lineEdit_bracketURL_->setVisible(false);
    lineEdit_otherBracketType_->setVisible(false);
    lineEdit_otherBracketType_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(new QLabel("Bracket Type:"), 0, 0);
    layout->addWidget(comboBox_bracketType_, 0, 1);
    layout->addWidget(lineEdit_otherBracketType_, 0, 2);
    layout->addWidget(label_bracketURL_, 1, 0);
    layout->addWidget(lineEdit_bracketURL_, 1, 1);

    contentWidget()->setLayout(layout);
    updateSize();

    connect(comboBox_bracketType_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetadataEditWidget_Event::onComboBoxBracketTypeChanged);
    connect(lineEdit_bracketURL_, &QLineEdit::textChanged, this, &MetadataEditWidget_Event::onLineEditBracketURLChanged);
    connect(lineEdit_otherBracketType_, &QLineEdit::textChanged, this, &MetadataEditWidget_Event::onLineEditOtherChanged);
}

// ----------------------------------------------------------------------------
MetadataEditWidget_Event::~MetadataEditWidget_Event()
{}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::updateUIVisibility(rfcommon::BracketType bracketType)
{
    switch (bracketType.type())
    {
        case rfcommon::BracketType::SINGLES:
        case rfcommon::BracketType::DOUBLES:
        case rfcommon::BracketType::AMATEURS:
        case rfcommon::BracketType::SIDE:
            label_bracketURL_->setVisible(true);
            lineEdit_bracketURL_->setVisible(true);
            break;

        case rfcommon::BracketType::MONEYMATCH:
        case rfcommon::BracketType::PRACTICE:
        case rfcommon::BracketType::FRIENDLIES:
        case rfcommon::BracketType::OTHER:
            label_bracketURL_->setVisible(false);
            lineEdit_bracketURL_->setVisible(false);
            break;
    }

    lineEdit_otherBracketType_->setVisible(bracketType.type() == rfcommon::BracketType::OTHER);

    updateSize();
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onComboBoxBracketTypeChanged(int index)
{
    PROFILE(MetadataEditWidget_Event, onComboBoxBracketTypeChanged);

    rfcommon::BracketType type = index == rfcommon::BracketType::OTHER ?
                rfcommon::BracketType::makeOther(lineEdit_otherBracketType_->text().toUtf8().constData()) :
                rfcommon::BracketType::fromIndex(index);

    updateUIVisibility(type);

    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setBracketType(type);
    ignoreSelf_ = false;

    model_->dispatcher.dispatch(&MetadataEditListener::onBracketTypeChangedUI, type);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onLineEditBracketURLChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setBracketURL(text.toUtf8().constData());
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onLineEditOtherChanged(const QString& text)
{
    ignoreSelf_ = true;
    for (auto& mdata : model_->metadata())
        if (mdata->type() == rfcommon::Metadata::GAME)
            mdata->asGame()->setBracketType(rfcommon::BracketType::makeOther(text.toUtf8().constData()));
    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Event, onAdoptMetadata);

    int bracketType = -1;
    QString bracketURL, otherName;

    bool first = true;
    for (int i = 0; i != mdata.count(); ++i)
    {
        switch (mdata[i]->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = mdata[i]->asGame();
                if (first)
                {
                    bracketType = g->bracketType().index();
                    bracketURL = QString::fromUtf8(g->bracketURL().cStr());
                    otherName = QString::fromUtf8(g->bracketType().description());

                    updateUIVisibility(g->bracketType());
                    model_->dispatcher.dispatch(&MetadataEditListener::onBracketTypeChangedUI, g->bracketType());
                }
                else
                {
                    if (bracketType != g->bracketType().index())
                        bracketType = -1;
                    if (bracketURL != QString::fromUtf8(g->bracketURL().cStr()))
                        bracketURL = "*";
                    if (otherName != QString::fromUtf8(g->bracketType().description()))
                        otherName = "*";
                }
            } break;

            case rfcommon::Metadata::TRAINING: {

            } break;
        }
        first = false;
    }

    const QSignalBlocker blockBracketType(comboBox_bracketType_);
    const QSignalBlocker blockBracketURL(lineEdit_bracketURL_);
    const QSignalBlocker blockOther(lineEdit_otherBracketType_);

    if (bracketType == -1)
    {
        comboBox_bracketType_->setCurrentIndex(-1);
        comboBox_bracketType_->setPlaceholderText("*");

        lineEdit_bracketURL_->setText("");
        lineEdit_otherBracketType_->setText("");
    }
    else
    {
        comboBox_bracketType_->setCurrentIndex(bracketType);

        switch (bracketType)
        {
            case rfcommon::BracketType::SINGLES:
            case rfcommon::BracketType::DOUBLES:
            case rfcommon::BracketType::AMATEURS:
            case rfcommon::BracketType::SIDE:
                lineEdit_bracketURL_->setText(bracketURL);
                break;

            case rfcommon::BracketType::MONEYMATCH:
            case rfcommon::BracketType::PRACTICE:
            case rfcommon::BracketType::FRIENDLIES:
            case rfcommon::BracketType::OTHER:
                lineEdit_bracketURL_->setText("");
                break;
        }

        if (bracketType == rfcommon::BracketType::OTHER)
            lineEdit_otherBracketType_->setText(otherName);
        else
            lineEdit_otherBracketType_->setText("");
    }
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Event, onOverwriteMetadata);

    assert(map.count() == mdata.count());

    ignoreSelf_ = true;

    for (int i = 0; i != map.count(); ++i)
    {
        switch (mdata[i]->type())
        {
            case rfcommon::Metadata::GAME: {
                rfcommon::GameMetadata* g = mdata[i]->asGame();

                rfcommon::BracketType bracketType = comboBox_bracketType_->currentIndex() == rfcommon::BracketType::OTHER ?
                            rfcommon::BracketType::makeOther(lineEdit_otherBracketType_->text().toUtf8().constData()) :
                            rfcommon::BracketType::fromIndex(comboBox_bracketType_->currentIndex());

                g->setBracketType(bracketType);
                g->setBracketURL(lineEdit_bracketURL_->text().toUtf8().constData());
            } break;

            case rfcommon::Metadata::TRAINING: {
            } break;
        }
    }

    ignoreSelf_ = false;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onNextGameStarted() {}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata) {}
void MetadataEditWidget_Event::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetadataEditWidget_Event::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetadataEditWidget_Event::onMetadataTournamentDetailsChanged() {}
void MetadataEditWidget_Event::onMetadataEventDetailsChanged()
{
    if (ignoreSelf_)
        return;
    onAdoptMetadata(model_->mappingInfo(), model_->metadata());
}
void MetadataEditWidget_Event::onMetadataCommentatorsChanged() {}
void MetadataEditWidget_Event::onMetadataGameDetailsChanged() {}
void MetadataEditWidget_Event::onMetadataPlayerDetailsChanged() {}
void MetadataEditWidget_Event::onMetadataWinnerChanged(int winnerPlayerIdx) {}
void MetadataEditWidget_Event::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
