#include "application/ui_ImportReplayPackDialog.h"
#include "application/models/ReplayManager.hpp"
#include "application/models/ReplayGroup.hpp"
#include "application/views/ImportReplayPackDialog.hpp"
#include "application/widgets/ProgressDialog.hpp"

#include "rfcommon/Deserializer.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Utf8.hpp"

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

    ui_->comboBox_selectGroup->addItem("Don't add to group");
    ui_->comboBox_selectGroup->addItem("Create new group...");
    ui_->label_newGroupName->setVisible(false);
    ui_->lineEdit_newGroupName->setVisible(false);

    for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
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
    PROFILE(ImportReplayPackDialog, checkEnableImportButton);

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
    PROFILE(ImportReplayPackDialog, onSelectReplayPack);

    QString packFileName = QFileDialog::getOpenFileName(this, "Open Replay Pack", "", "Replay Pack (*.rfp)");
    if (packFileName.length() == 0)
        return;

    rfcommon::MappedFile file;
    if (file.open(packFileName.toUtf8().constData()) == false)
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

    uint32_t numFiles = deserializer.readLU32();
    bool hasVideos = false;
    for (int i = 0; i != numFiles; ++i)
    {
        char type[4];
        memcpy(type, deserializer.readFromPtr(4), 4);
        uint64_t offset = deserializer.readLU64();
        uint64_t size = deserializer.readLU64();
        uint8_t fileNameLen = deserializer.readU8();
        deserializer.readFromPtr(fileNameLen);

        if (memcmp(type, "VIDE", 4) == 0)
        {
            hasVideos = true;
            break;
        }
    }

    if (hasVideos == false)
        ui_->radioButton_noVideos->setChecked(true);

    ui_->lineEdit_packFileName->setText(packFileName);
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onSelectReplayDir()
{
    PROFILE(ImportReplayPackDialog, onSelectReplayDir);

    QString replayDir = QFileDialog::getExistingDirectory(this, "Extract Replays To...");
    if (replayDir.length() == 0)
        return;

    ui_->lineEdit_replayPath->setText(replayDir);
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onSelectVideoDir()
{
    PROFILE(ImportReplayPackDialog, onSelectVideoDir);

    QString videoDir = QFileDialog::getExistingDirectory(this, "Extract Videos To...");
    if (videoDir.length() == 0)
        return;

    ui_->lineEdit_videoPath->setText(videoDir);
    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onExtractVideoSettingChanged()
{
    PROFILE(ImportReplayPackDialog, onExtractVideoSettingChanged);

    checkEnableImportButton();

    bool needVideoPath = ui_->radioButton_extractVideos->isChecked();
    ui_->lineEdit_videoPath->setEnabled(needVideoPath);
    ui_->toolButton_videoPath->setEnabled(needVideoPath);
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onTargetReplayGroupChanged(int index)
{
    PROFILE(ImportReplayPackDialog, onTargetReplayGroupChanged);

    ui_->label_newGroupName->setVisible(index == 1);
    ui_->lineEdit_newGroupName->setVisible(index == 1);

    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onNewGroupNameChanged(const QString& name)
{
    PROFILE(ImportReplayPackDialog, onNewGroupNameChanged);

    checkEnableImportButton();
}

// ----------------------------------------------------------------------------
void ImportReplayPackDialog::onImport()
{
    PROFILE(ImportReplayPackDialog, onImport);

    ProgressDialog progress("Importing", "Importing data from \"" + ui_->lineEdit_packFileName->text() + "\"");
    progress.show();
    progress.raise();

    QDir replayDir(ui_->lineEdit_replayPath->text());
    QDir videoDir(ui_->lineEdit_videoPath->text());

    ReplayGroup* group = nullptr;
    if (ui_->comboBox_selectGroup->currentIndex() == 1 && ui_->lineEdit_newGroupName->text().length() > 0)
        group = replayManager_->addReplayGroup(ui_->lineEdit_newGroupName->text());
    else if (ui_->comboBox_selectGroup->currentIndex() > 1)
        group = replayManager_->replayGroup(ui_->comboBox_selectGroup->currentText());

    rfcommon::MappedFile file;
    if (file.open(ui_->lineEdit_packFileName->text().toUtf8().constData()) == false)
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
    bool overwriteAll = false;
    bool skipAll = false;
    uint32_t numFiles = deserializer.readLU32();
    for (int i = 0; i != numFiles; ++i)
    {
        char type[4];
        memcpy(type, deserializer.readFromPtr(4), 4);
        uint64_t offset = deserializer.readLU64();
        uint64_t size = deserializer.readLU64();
        uint8_t fileNameLen = deserializer.readU8();
        rfcommon::String fileNameUtf8(
            reinterpret_cast<const char*>(deserializer.readFromPtr(fileNameLen)),
            fileNameLen);
        QString fileName = QString::fromUtf8(fileNameUtf8.cStr());

        QDir dir;
        if (memcmp(type, "REPL", 4) == 0)
            dir = replayDir;
        else if (memcmp(type, "VIDE", 4) == 0)
        {
            // Skip if user doesn't want to extract videos
            if (ui_->radioButton_noVideos->isChecked())
                continue;

            dir = videoDir;
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

        if (dir.exists(fileName) && overwriteAll == false)
        {
            if (skipAll)
                continue;

            switch (QMessageBox::question(this,
                "File Exists",
                "The file \"" + fileName + "\" already exists\n\n"
                "Do you want to overwrite this file?",
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel | QMessageBox::YesToAll | QMessageBox::NoToAll))
            {
                case QMessageBox::Yes: break;
                case QMessageBox::No: continue;
                case QMessageBox::Cancel: return;
                case QMessageBox::YesToAll: overwriteAll = true; break;
                case QMessageBox::NoToAll: skipAll = true; continue;
                default: return;
            }
        }

        QString filePath = dir.absoluteFilePath(fileName);
        QByteArray filePathUtf8 = filePath.toUtf8();
        FILE* fp = rfcommon::utf8_fopen_write(filePathUtf8.constData(), filePathUtf8.length());
        if (fp == nullptr)
        {
            QMessageBox::critical(this,
                "File Error",
                "Failed to open file \"" + fileName + "\"\n\n" + strerror(errno));
            return;
        }

        if (fwrite(file.addressAtOffset(offset), 1, size, fp) != size)
        {
            QMessageBox::critical(this,
                "Write Error",
                "Failed to write data to file \"" + fileName + "\"\n\n" + strerror(errno));
            fclose(fp);
            remove(fileName.toUtf8().constData());
            return;
        }

        fclose(fp);

        if (memcmp(type, "REPL", 4) == 0)
        {
            replayManager_->allReplayGroup()->addFile(fileName);
            if (group)
                group->addFile(fileName);
        }

        progress.setPercent(i * 100 / numFiles, QString("Unpacking ") + fileName);
    }

    replayManager_->addGamePath(ui_->lineEdit_replayPath->text());

    if (videoWasExtracted)
        replayManager_->addVideoPath(ui_->lineEdit_videoPath->text());

    if (ui_->comboBox_selectGroup->currentIndex() == 1)

    progress.close();
    QMessageBox::information(this, "Success", "Import successful!");
    close();
}

}
