#include "application/ui_ImportReplayPackDialog.h"
#include "application/views/ImportReplayPackDialog.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/models/ReplayGroup.hpp"

#include "rfcommon/Deserializer.hpp"
#include "rfcommon/MappedFile.hpp"

#include <QFileDialog>
#include <QMessageBox>

namespace rfapp {

// ----------------------------------------------------------------------------
ImportReplayPackDialog::ImportReplayPackDialog(ReplayManager* replayManager, QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::ImportReplayPackDialog)
    , replayManager_(replayManager)
{
    ui_->setupUi(this);

    // Window icon
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

    ui_->comboBox_selectGroup->addItem("Don't add to group");
    ui_->comboBox_selectGroup->addItem("Create new group...");
    ui_->label_newGroupName->setVisible(false);
    ui_->lineEdit_newGroupName->setVisible(false);

    for (int i = 0; i != replayManager_->replayGroupsCount(); ++i)
        ui_->comboBox_selectGroup->addItem(replayManager_->replayGroup(i)->name());

    ui_->lineEdit_replayPath->setText(replayManager_->defaultGamePath().absolutePath());

    connect(ui_->toolButton_choosePack, &QToolButton::released, this, &ImportReplayPackDialog::onSelectReplayPack);
    connect(ui_->toolButton_replayPath, &QToolButton::released, this, &ImportReplayPackDialog::onSelectReplayDir);
    connect(ui_->toolButton_videoPath, &QToolButton::released, this, &ImportReplayPackDialog::onSelectVideoDir);
    connect(ui_->radioButton_embedVideos, &QRadioButton::toggled, [this](bool enable) { if (enable) onExtractVideoSettingChanged();  });
    connect(ui_->radioButton_extractVideos, &QRadioButton::toggled, [this](bool enable) { if (enable) onExtractVideoSettingChanged();  });
    connect(ui_->radioButton_noVideos, &QRadioButton::toggled, [this](bool enable) { if (enable) onExtractVideoSettingChanged();  });
    connect(ui_->comboBox_selectGroup, qOverload<int>(&QComboBox::currentIndexChanged), this, &ImportReplayPackDialog::onTargetReplayGroupChanged);
    connect(ui_->lineEdit_newGroupName, &QLineEdit::textChanged, this, &ImportReplayPackDialog::onNewGroupNameChanged);
    
    connect(ui_->pushButton_import, &QPushButton::released, this, &ImportReplayPackDialog::onImport);
}

// ----------------------------------------------------------------------------
ImportReplayPackDialog::~ImportReplayPackDialog()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::checkEnableImportButton()
{
    QFileInfo packInfo(ui_->lineEdit_packFileName->text());
    QString replayDir = ui_->lineEdit_replayPath->text();
    QString videoDir = ui_->lineEdit_videoPath->text();

    bool canImport = packInfo.exists() && packInfo.isFile();
    canImport &= QDir(replayDir).exists() && QFileInfo(replayDir).isDir();

    if (ui_->radioButton_extractVideos->isChecked())
        canImport &= QDir(videoDir).exists() && QFileInfo(videoDir).isDir();

    if (ui_->comboBox_selectGroup->currentIndex() == 1)  // Create new group
        canImport &= ui_->lineEdit_newGroupName->text().length() > 0;

    ui_->pushButton_import->setEnabled(canImport);
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onSelectReplayPack()
{
    QString packFileName = QFileDialog::getOpenFileName(this, "Open Replay Pack", "", "Replay Pack (*.rfp)");
    if (packFileName.length() == 0)
        return;

    ui_->lineEdit_packFileName->setText(packFileName);
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onSelectReplayDir()
{
    QString replayDir = QFileDialog::getExistingDirectory(this, "Extract Replays To...");
    if (replayDir.length() == 0)
        return;

    ui_->lineEdit_replayPath->setText(replayDir);
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onSelectVideoDir()
{
    QString videoDir = QFileDialog::getExistingDirectory(this, "Extract Videos To...");
    if (videoDir.length() == 0)
        return;

    ui_->lineEdit_videoPath->setText(videoDir);
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onExtractVideoSettingChanged()
{
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onTargetReplayGroupChanged(int index)
{
    ui_->label_newGroupName->setVisible(index == 1);
    ui_->lineEdit_newGroupName->setVisible(index == 1);

    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onNewGroupNameChanged(const QString& name)
{
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onImport()
{
    QDir replayDir(ui_->lineEdit_replayPath->text());
    QDir videoDir(ui_->lineEdit_videoPath->text());

    rfcommon::MappedFile file;
    if (file.open(ui_->lineEdit_packFileName->text().toLocal8Bit().constData()) == false)
    {
        QMessageBox::critical(this, 
            "File Error", 
            "Failed to open file \"" + ui_->lineEdit_packFileName->text() + "\"");
        return;
    }

    rfcommon::Deserializer deserializer(file.address(), file.size());
    if (const void* magic = deserializer.readFromPtr(4); magic == nullptr || memcmp(magic, "RFP1", 4) != 0)
    {
        QMessageBox::critical(this,
            "Format Error",
            "File has wrong magic bytes at beginning. Aborting.");
        return;
    }

    const uint8_t major = deserializer.readU8();
    const uint8_t minor = deserializer.readU8();
    if (major != 1 || minor != 1)
    {
        QMessageBox::critical(this,
            "Version Error",
            "File has format version " + QString::number(major) + "." + QString::number(minor) + ", "
            "but this version of ReFramed only supports up to 1.1");
        return;
    }

    bool videoWasExtracted = false;
    uint32_t numFiles = deserializer.readLU32();
    for (int i = 0; i != numFiles; ++i)
    {
        char type[4];
        memcpy(type, deserializer.readFromPtr(4), 4);
        uint64_t offset = deserializer.readLU64();
        uint64_t size = deserializer.readLU64();
        uint8_t fileNameLen = deserializer.readU8();
        rfcommon::String fileName(
            reinterpret_cast<const char*>(deserializer.readFromPtr(fileNameLen)),
            fileNameLen);

        QString absFileName;
        if (memcmp(type, "REPL", 4) == 0)
            absFileName = replayDir.absoluteFilePath(fileName.cStr());
        else if (memcmp(type, "VIDE", 4) == 0)
        {
            // Skip if user doesn't want to extract videos
            if (ui_->radioButton_noVideos->isChecked())
                continue;

            absFileName = videoDir.absoluteFilePath(fileName.cStr());
            videoWasExtracted = true;
        }
        else
        {
            char buf[5];
            memcpy(buf, type, 4);
            buf[4] = 0;
            if (QMessageBox::question(this,
                "Unknown Chunk",
                QString("An unknown chunk type \"") + buf + "\" was encountered while unpacking\n\n"
                "Do you want to skip it and continue?") == QMessageBox::Yes)
            {
                continue;
            }
            return;
        }

        if (QFileInfo(absFileName).exists())
        {
            switch (QMessageBox::question(this,
                "File Exists",
                "The file \"" + absFileName + "\" already exists"
                "Do you want to overwrite this file?"),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)
            {
                case QMessageBox::Yes: break;
                case QMessageBox::No: continue;
                case QMessageBox::Cancel: return;
                default: return;
            }
        }

        FILE* fp = fopen(absFileName.toLocal8Bit().constData(), "wb");
        if (fp == nullptr)
        {
            QMessageBox::critical(this,
                "File Error",
                "Failed to open file \"" + absFileName + "\"\n\n" + strerror(errno));
            return;
        }

        if (fwrite(file.addressAtOffset(offset), 1, size, fp) != size)
        {
            QMessageBox::critical(this,
                "Write Error",
                "Failed to write data to file \"" + absFileName + "\"\n\n" + strerror(errno));
            fclose(fp);
            remove(absFileName.toLocal8Bit().constData());
            return;
        }

        fclose(fp);

        if (memcmp(type, "REPL", 4) == 0)
            replayManager_->allReplayGroup()->addFile(fileName.cStr());
    }

    int i = 2;
    QString name = QFileInfo(ui_->lineEdit_packFileName->text()).fileName().remove(".rfp");
    while (replayManager_->gamePathNameExists(name) || replayManager_->videoPathNameExists(name))
        name = ui_->lineEdit_packFileName->text().remove(".rfp") + QString::number(i++);

    if (replayManager_->gamePathExists(ui_->lineEdit_replayPath->text()) == false)
        replayManager_->addGamePath(name, ui_->lineEdit_replayPath->text());

    if (videoWasExtracted && ui_->radioButton_extractVideos->isChecked())
        if (replayManager_->videoPathExists(ui_->lineEdit_videoPath->text()) == false)
            replayManager_->addVideoPath(name, ui_->lineEdit_videoPath->text());

    QMessageBox::information(this, "Success", "Import successful!");
    close();
}

}
