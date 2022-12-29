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
    , label_bracketURL_(new QLabel("Bracket URL:"))
    , lineEdit_bracketURL_(new QLineEdit)
    , lineEdit_otherBracketType_(new QLineEdit)
{
    setTitle(QIcon::fromTheme(""), "Event");

    QComboBox* bracketType = new QComboBox;
#define X(type, name) bracketType->addItem(name);
    BRACKET_TYPE_LIST
#undef X
    bracketType->setCurrentIndex(rfcommon::BracketType::FRIENDLIES);
    bracketType->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    label_bracketURL_->setVisible(false);
    lineEdit_bracketURL_->setVisible(false);
    lineEdit_otherBracketType_->setVisible(false);
    lineEdit_otherBracketType_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(new QLabel("Bracket Type:"), 0, 0);
    layout->addWidget(bracketType, 0, 1);
    layout->addWidget(lineEdit_otherBracketType_, 0, 2);
    layout->addWidget(label_bracketURL_, 1, 0);
    layout->addWidget(lineEdit_bracketURL_, 1, 1);

    contentWidget()->setLayout(layout);
    updateSize();

    connect(bracketType, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetadataEditWidget_Event::onComboBoxBracketTypeChanged);
}

// ----------------------------------------------------------------------------
MetadataEditWidget_Event::~MetadataEditWidget_Event()
{}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onComboBoxBracketTypeChanged(int index)
{
    PROFILE(MetadataEditWidget_Event, onComboBoxBracketTypeChanged);

    rfcommon::BracketType type = index == rfcommon::BracketType::OTHER ?
                rfcommon::BracketType::makeOther(lineEdit_otherBracketType_->text().toUtf8().constData()) :
                rfcommon::BracketType::fromIndex(index);

    switch (type.type())
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

    lineEdit_otherBracketType_->setVisible(index == rfcommon::BracketType::OTHER);
    updateSize();

    model_->dispatcher.dispatch(&MetadataEditListener::onBracketTypeChangedUI, type);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Event, onAdoptMetadata);

}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_Event, onOverwriteMetadata);

}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onNextGameStarted() {}

// ----------------------------------------------------------------------------
void MetadataEditWidget_Event::onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata) {}
void MetadataEditWidget_Event::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetadataEditWidget_Event::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetadataEditWidget_Event::onMetadataTournamentDetailsChanged() {}
void MetadataEditWidget_Event::onMetadataEventDetailsChanged() {}
void MetadataEditWidget_Event::onMetadataCommentatorsChanged() {}
void MetadataEditWidget_Event::onMetadataGameDetailsChanged() {}
void MetadataEditWidget_Event::onMetadataPlayerDetailsChanged() {}
void MetadataEditWidget_Event::onMetadataWinnerChanged(int winnerPlayerIdx) {}
void MetadataEditWidget_Event::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
