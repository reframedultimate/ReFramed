#include "rfcommon/Profiler.hpp"
#include "application/models/MetadataEditModel.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/widgets/MetadataEditWidget_AutoAssociateVideo.hpp"

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QSpinBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetadataEditWidget_AutoAssociateVideo::MetadataEditWidget_AutoAssociateVideo(MetadataEditModel* model, ActiveSessionManager* activeSessionManager, QWidget* parent)
    : MetadataEditWidget(model, parent)
    , activeSessionManager_(activeSessionManager)
{
    setTitle(QIcon::fromTheme("film"), "Auto-Associate Video");

    QCheckBox* checkBox_enable = new QCheckBox("Search for video to auto-associate after each game");
    checkBox_enable->setChecked(activeSessionManager_->autoAssociateVideoEnabled());

    label_chooseDir = new QLabel("Directory:");
    label_chooseDir->setEnabled(activeSessionManager_->autoAssociateVideoEnabled());

    lineEdit_dir = new QLineEdit;
    lineEdit_dir->setReadOnly(true);
    lineEdit_dir->setText(activeSessionManager_->autoAssociateVideoDirectory());
    lineEdit_dir->setEnabled(activeSessionManager_->autoAssociateVideoEnabled());

    toolButton_chooseDir = new QToolButton;
    toolButton_chooseDir->setIcon(QIcon::fromTheme("more-horizontal"));
    toolButton_chooseDir->setEnabled(activeSessionManager_->autoAssociateVideoEnabled());

    label_frameOffset = new QLabel("Frame offset correction:");
    label_frameOffset->setEnabled(activeSessionManager_->autoAssociateVideoEnabled());
    spinBox_frameOffset = new QSpinBox;
    spinBox_frameOffset->setMinimum(-10000);
    spinBox_frameOffset->setMaximum(10000);
    spinBox_frameOffset->setValue(activeSessionManager_->autoAssociateVideoFrameOffset());
    spinBox_frameOffset->setEnabled(activeSessionManager_->autoAssociateVideoEnabled());

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(checkBox_enable, 0, 0, 1, 3);
    layout->addWidget(label_chooseDir, 1, 0);
    layout->addWidget(lineEdit_dir, 1, 1);
    layout->addWidget(toolButton_chooseDir, 1, 2);
    layout->addWidget(label_frameOffset, 2, 0, 1, 1);
    layout->addWidget(spinBox_frameOffset, 2, 1, 1, 2);

    contentWidget()->setLayout(layout);
    updateSize();

    connect(checkBox_enable, &QCheckBox::toggled, this, &MetadataEditWidget_AutoAssociateVideo::onCheckBoxEnableToggled);
    connect(toolButton_chooseDir, &QToolButton::released, this, &MetadataEditWidget_AutoAssociateVideo::onToolButtonChooseDirectoryReleased);
    connect(spinBox_frameOffset, qOverload<int>(&QSpinBox::valueChanged), this, &MetadataEditWidget_AutoAssociateVideo::onSpinBoxFrameOffsetValueChanged);
}

// ----------------------------------------------------------------------------
MetadataEditWidget_AutoAssociateVideo::~MetadataEditWidget_AutoAssociateVideo()
{}

// ----------------------------------------------------------------------------
void MetadataEditWidget_AutoAssociateVideo::onCheckBoxEnableToggled(bool enable)
{
    PROFILE(MetadataEditWidget_AutoAssociateVideo, onCheckBoxEnableToggled);

    label_chooseDir->setEnabled(enable);
    lineEdit_dir->setEnabled(enable);
    toolButton_chooseDir->setEnabled(enable);
    label_frameOffset->setEnabled(enable);
    spinBox_frameOffset->setEnabled(enable);

    activeSessionManager_->setAutoAssociateVideoEnabled(enable);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_AutoAssociateVideo::onToolButtonChooseDirectoryReleased()
{
    PROFILE(MetadataEditWidget_AutoAssociateVideo, onToolButtonChooseDirectoryReleased);

    QString vidDir = QFileDialog::getExistingDirectory(this, "Choose video directory", "", QFileDialog::ShowDirsOnly);
    if (vidDir.isEmpty())
        return;

    lineEdit_dir->setText(vidDir);
    activeSessionManager_->setAutoAssociateVideoDirectory(vidDir);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_AutoAssociateVideo::onSpinBoxFrameOffsetValueChanged(int value)
{
    activeSessionManager_->setAutoAssociateVideoFrameOffset(value);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget_AutoAssociateVideo::onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_AutoAssociateVideo, onAdoptMetadata);

}

// ----------------------------------------------------------------------------
void MetadataEditWidget_AutoAssociateVideo::onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_AutoAssociateVideo, onOverwriteMetadata);

}

// ----------------------------------------------------------------------------
void MetadataEditWidget_AutoAssociateVideo::onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata)
{
    PROFILE(MetadataEditWidget_AutoAssociateVideo, onMetadataCleared);

}

// ----------------------------------------------------------------------------
void MetadataEditWidget_AutoAssociateVideo::onNextGameStarted()
{
    PROFILE(MetadataEditWidget_AutoAssociateVideo, onNextGameStarted);

}

void MetadataEditWidget_AutoAssociateVideo::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataTournamentDetailsChanged() {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataEventDetailsChanged() {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataCommentatorsChanged() {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataGameDetailsChanged() {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataPlayerDetailsChanged() {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataWinnerChanged(int winnerPlayerIdx) {}
void MetadataEditWidget_AutoAssociateVideo::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
