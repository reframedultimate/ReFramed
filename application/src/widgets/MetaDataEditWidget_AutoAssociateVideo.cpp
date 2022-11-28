#include "application/models/MetaDataEditModel.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/widgets/MetaDataEditWidget_AutoAssociateVideo.hpp"

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_AutoAssociateVideo::MetaDataEditWidget_AutoAssociateVideo(MetaDataEditModel* model, ActiveSessionManager* activeSessionManager, QWidget* parent)
    : MetaDataEditWidget(model, parent)
    , activeSessionManager_(activeSessionManager)
{
    setTitle("Auto-Associate Video");

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

    connect(checkBox_enable, &QCheckBox::toggled, this, &MetaDataEditWidget_AutoAssociateVideo::onCheckBoxEnableToggled);
    connect(toolButton_chooseDir, &QToolButton::released, this, &MetaDataEditWidget_AutoAssociateVideo::onToolButtonChooseDirectoryReleased);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_AutoAssociateVideo::~MetaDataEditWidget_AutoAssociateVideo()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_AutoAssociateVideo::onCheckBoxEnableToggled(bool enable)
{
    label_chooseDir->setEnabled(enable);
    lineEdit_dir->setEnabled(enable);
    toolButton_chooseDir->setEnabled(enable);

    activeSessionManager_->setAutoAssociateVideoDirectory(enable ?
        lineEdit_dir->text() : "");
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_AutoAssociateVideo::onToolButtonChooseDirectoryReleased()
{
    QString vidDir = QFileDialog::getExistingDirectory(this, "Choose video directory", "", QFileDialog::ShowDirsOnly);
    if (vidDir.isEmpty())
        return;

    lineEdit_dir->setText(vidDir);
    activeSessionManager_->setAutoAssociateVideoDirectory(vidDir);
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_AutoAssociateVideo::onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_AutoAssociateVideo::onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata)
{
}

void MetaDataEditWidget_AutoAssociateVideo::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataTournamentDetailsChanged() {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataEventDetailsChanged() {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataGameDetailsChanged() {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataPlayerDetailsChanged() {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_AutoAssociateVideo::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
