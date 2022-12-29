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

namespace rfapp {

// ----------------------------------------------------------------------------
MetadataEditWidget_AutoAssociateVideo::MetadataEditWidget_AutoAssociateVideo(MetadataEditModel* model, ActiveSessionManager* activeSessionManager, QWidget* parent)
    : MetadataEditWidget(model, parent)
    , activeSessionManager_(activeSessionManager)
{
    setTitle(QIcon::fromTheme("film"), "Auto-Associate Video");

    const QString& vidDir = activeSessionManager_->autoAssociateVideoDirectory();

    QCheckBox* checkBox_enable = new QCheckBox("Search for video to auto-associate after each game");
    checkBox_enable->setChecked(!vidDir.isEmpty());

    label_chooseDir = new QLabel("Directory:");
    label_chooseDir->setEnabled(!vidDir.isEmpty());

    lineEdit_dir = new QLineEdit;
    lineEdit_dir->setReadOnly(true);
    lineEdit_dir->setText(vidDir);
    lineEdit_dir->setEnabled(!vidDir.isEmpty());

    toolButton_chooseDir = new QToolButton;
    toolButton_chooseDir->setText("...");
    toolButton_chooseDir->setEnabled(!vidDir.isEmpty());

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(checkBox_enable, 0, 0, 1, 3);
    layout->addWidget(label_chooseDir, 1, 0);
    layout->addWidget(lineEdit_dir, 1, 1);
    layout->addWidget(toolButton_chooseDir, 1, 2);

    contentWidget()->setLayout(layout);
    updateSize();

    connect(checkBox_enable, &QCheckBox::toggled, this, &MetadataEditWidget_AutoAssociateVideo::onCheckBoxEnableToggled);
    connect(toolButton_chooseDir, &QToolButton::released, this, &MetadataEditWidget_AutoAssociateVideo::onToolButtonChooseDirectoryReleased);
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

    activeSessionManager_->setAutoAssociateVideoDirectory(enable ?
        lineEdit_dir->text() : "");
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
