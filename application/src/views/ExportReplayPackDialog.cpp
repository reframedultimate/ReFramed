#include "application/ui_ExportReplayPackDialog.h"
#include "application/views/ExportReplayPackDialog.hpp"

#include "rfcommon/Endian.hpp"
#include "rfcommon/FilePathResolver.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Serializer.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/VideoMeta.hpp"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressBar>

namespace rfapp {

ProgressDialog::ProgressDialog(const QString& title, const QString& text, QWidget* parent)
    : QWidget(parent)
    , bar_(new QProgressBar)
{
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));
    setWindowTitle(title);
    //setWindowFlags(Qt::WindowStaysOnTopHint);

    bar_->setMinimum(0);
    bar_->setMaximum(100);
    bar_->setValue(0);

    QLabel* label = new QLabel(text);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(bar_);
    setLayout(layout);
}
ProgressDialog::~ProgressDialog()
{}
void ProgressDialog::setPercent(int percent)
{
    bar_->setValue(percent);
    qApp->processEvents();
}

// ----------------------------------------------------------------------------
ExportReplayPackDialog::ExportReplayPackDialog(rfcommon::FilePathResolver* pathResolver, const QStringList& replayNames, const QStringList& replayFileNames, QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::ExportReplayPackDialog)
    , pathResolver_(pathResolver)
    , replayNames_(replayNames)
    , replayFileNames_(replayFileNames)
{
    ui_->setupUi(this);

    // Window icon
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

    for (const auto& name : replayNames)
        ui_->listWidget_replays->addItem(name);
    ui_->listWidget_replays->selectAll();

    connect(ui_->pushButton_next, &QPushButton::released, [this] { ui_->stackedWidget->setCurrentIndex(1); });
    connect(ui_->pushButton_back, &QPushButton::released, [this] { ui_->stackedWidget->setCurrentIndex(0); });
    connect(ui_->listWidget_replays, &QListWidget::itemSelectionChanged, this, &ExportReplayPackDialog::onSelectionChanged);
    connect(ui_->toolButton_packFileName, &QToolButton::released, this, &ExportReplayPackDialog::onChoosePackFile);
    connect(ui_->pushButton_export, &QPushButton::released, this, &ExportReplayPackDialog::onExport);
}

// ----------------------------------------------------------------------------
ExportReplayPackDialog::~ExportReplayPackDialog()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ExportReplayPackDialog::onSelectionChanged()
{
    bool canProceed = ui_->listWidget_replays->selectedItems().size() > 0;
    ui_->pushButton_next->setEnabled(canProceed);
}

// ----------------------------------------------------------------------------
void ExportReplayPackDialog::onChoosePackFile()
{
    QString destination = QFileDialog::getSaveFileName(this, "Replay Pack Destination", "", "Replay Pack (*.rfp)");
    if (destination.length() == 0)
        return;
    ui_->lineEdit_packFileName->setText(destination);

    bool canExport = QDir(QFileInfo(ui_->lineEdit_packFileName->text()).absolutePath()).exists();
    ui_->pushButton_export->setEnabled(canExport);
}

// ----------------------------------------------------------------------------
void ExportReplayPackDialog::onExport()
{
    struct SessionWithName
    {
        rfcommon::Reference<rfcommon::Session> session;
        QString name;
    };
    rfcommon::Vector<SessionWithName> sessions;

    ProgressDialog progress("Exporting", "Exporting data to \"" + ui_->lineEdit_packFileName->text() + "\"");
    progress.show();
    progress.raise();

    for (int i = 0; i != ui_->listWidget_replays->count(); ++i)
    {
        QListWidgetItem* item = ui_->listWidget_replays->item(i);
        if (ui_->listWidget_replays->isItemSelected(item) == false)
            continue;

        SessionWithName swn;
        assert(QDir(replayFileNames_[i]).isRelative());
        auto absFilePath = pathResolver_->resolveGameFile(replayFileNames_[i].toLocal8Bit().constData());
        swn.session = rfcommon::Session::load(pathResolver_, absFilePath.cStr());
        if (swn.session == nullptr)
        {
            if (QMessageBox::question(this,
                "Error loading file",
                "Failed to load replay file \"" + replayFileNames_[i] + "\"\n"
                "Absolute path was: \"" + absFilePath.cStr() + "\"\n\n"
                "Would you like to continue without including this file?") == QMessageBox::Yes)
            {
                continue;
            }
            return;
        }

        swn.name = QFileInfo(replayFileNames_[i]).fileName();
        sessions.push(std::move(swn));
    }
    progress.setPercent(5);

    FILE* fp = fopen(ui_->lineEdit_packFileName->text().toLocal8Bit().constData(), "wb");
    if (fp == nullptr)
    {
        QMessageBox::critical(this, "Failed to open file", "Failed to open file \"" + ui_->lineEdit_packFileName->text() + "\" for writing\n\n" + strerror(errno));
        return;
    }

    // Create a list of all video files. Multiple replay files can reference
    // the same video file, so we need to make sure to only include each
    // file once
    rfcommon::HashMap<rfcommon::String, rfcommon::VideoEmbed*> videoFiles;
    if (ui_->checkBox_includeVideos->isChecked())
    {
        for (const auto& swn : sessions)
        {
            if (swn.session->existsInContentTable(rfcommon::Session::Flags::VideoMeta) == false)
                continue;
            rfcommon::VideoMeta* vmeta = swn.session->tryGetVideoMeta();
            if (vmeta == nullptr)
            {
                if (QMessageBox::question(this,
                    "Failed to load data",
                    "Failed to load video meta-data from replay file \"" + ui_->lineEdit_packFileName->text() + "\"\n\n"
                    "Would you like to continue by only exporting the replay without video?") == QMessageBox::Yes)
                {
                    continue;
                }
                fclose(fp);
                return;
            }

            rfcommon::VideoEmbed* video = swn.session->tryGetVideo();
            if (video == nullptr)
            {
                if (QMessageBox::question(this,
                    "Failed to load data",
                    "Failed to load video associated with replay file \"" + ui_->lineEdit_packFileName->text() + "\"\n\n"
                    "Would you like to continue by only exporting the replay without video?") == QMessageBox::Yes)
                {
                    continue;
                }
                fclose(fp);
                return;
            }

            videoFiles.insertIfNew(vmeta->fileName(), video);
        }
    }
    progress.setPercent(10);

    struct Entry
    {
        char type[4];
        uint64_t offset;
        uint64_t size;
        uint8_t fileNameLen;
        rfcommon::String fileName;
    };
    rfcommon::Vector<Entry> table;

    // Calculate space we need for the content table, as this is written later
    const int headerSize = 4 + 2 + 4;  // magic, version, entry count
    const int contentTableEntries = sessions.count() + videoFiles.count();
    int contentTableSize = headerSize;
    for (const auto& swn : sessions)
        contentTableSize += 4 + 8 + 8 + 1 + swn.name.toUtf8().size();
    for (auto it : videoFiles)
        contentTableSize += 4 + 8 + 8 + 1 + + it.key().length();

    auto buffer = rfcommon::SmallVector<char, 256>::makeResized(contentTableSize);
    rfcommon::Serializer header(buffer.data(), buffer.count());

    uint64_t offset = contentTableSize;
    if (fseek(fp, offset, SEEK_SET) != 0)
        goto fail_with_error;

    for (int i = 0; i != sessions.count(); ++i)
    {
        // If replays have embedded video files, make sure to strip them
        // from the replay when adding it to the pack, as they are saved
        // separately
        uint8_t saveFlags = rfcommon::Session::Flags::All;
        saveFlags &= ~rfcommon::Session::Flags::VideoEmbed;

        Entry entry;
        memcpy(entry.type, "REPL", 4);
        entry.offset = offset;
        entry.size = sessions[i].session->save(fp, saveFlags);
        if (entry.size == 0)
        {
            if (QMessageBox::question(this,
                "Failed to save replay",
                "Failed to save the replay file \"" + sessions[i].name + "\"\n\n"
                "Would you like to continue by only exporting the replay without video?") == QMessageBox::Yes)
            {
                continue;
            }
            goto out;
        }

        QByteArray ba = sessions[i].name.toUtf8();
        entry.fileNameLen = ba.size();
        entry.fileName = ba.constData();

        offset += entry.size;
        table.push(entry);

        // Save embedded video
        if (sessions[i].session->existsInContentTable(rfcommon::Session::Flags::VideoEmbed))
        {
            auto vmeta = sessions[i].session->tryGetVideoMeta();
            auto embed = sessions[i].session->tryGetVideoEmbed();
            if (vmeta == nullptr || embed == nullptr)
            {
                if (QMessageBox::question(this,
                    "Failed to load embedded video",
                    "Failed to load embedded video in the replay file \"" + sessions[i].name + "\"\n\n"
                    "Would you skip saving this video?") == QMessageBox::Yes)
                {
                    continue;
                }
                goto out;
            }

            memcpy(entry.type, "VIDE", 4);
            entry.offset = offset;
            entry.size = fwrite(embed->address(), 1, embed->size(), fp);
            if (entry.size != embed->size())
            {
                if (QMessageBox::question(this,
                    "Failed to save video",
                    QString("Failed to save the video file \"") + vmeta->fileName() + "\"\n\n"
                    "Would you like to continue and exclude this video?") == QMessageBox::Yes)
                {
                    continue;
                }
                goto out;
            }

            entry.fileName = vmeta->fileName();
            entry.fileNameLen = entry.fileName.length();

            offset += entry.size;
            table.push(entry);
        }

        progress.setPercent(10 + i * 10 / sessions.count());
    }

    int i = 0;
    for (auto it : videoFiles)
    {
        Entry entry;
        memcpy(entry.type, "VIDE", 4);
        entry.offset = offset;
        entry.size = fwrite(it.value()->address(), 1, it.value()->size(), fp);
        if (entry.size != it.value()->size())
        {
            if (QMessageBox::question(this,
                "Failed to save video",
                QString("Failed to save the video file \"") + it->key().cStr() + "\"\n\n"
                "Would you like to continue and exclude this video?") == QMessageBox::Yes)
            {
                continue;
            }
            goto out;
        }

        for (auto& swn : sessions)
            if (swn.session.notNull() && swn.session->tryGetVideo() == it.value())
            {
                swn.session.drop();
                break;
            }

        entry.fileNameLen = it.key().length();
        entry.fileName = it.key();

        offset += entry.size;
        table.push(entry);

        progress.setPercent(20 + i * 80 / videoFiles.count());
        i++;
    }

    // Rewind to write header
    if (fseek(fp, 0, SEEK_SET) != 0)
        goto fail_with_error;

    // Write header
    memcpy(header.writeToPtr(4), "RFP1", 4);  // Magic
    header.writeU8(1);  // Major
    header.writeU8(1);  // Minor
    
    // Write content table
    header.writeLU32(table.count());  // Table entries
    for (const auto& entry : table)
    {
        header.write(entry.type, 4);
        header.writeLU64(entry.offset);
        header.writeLU64(entry.size);
        header.writeU8(entry.fileNameLen);
        memcpy(header.writeToPtr(entry.fileNameLen), entry.fileName.cStr(), entry.fileNameLen);
    }

    if (fwrite(header.data(), 1, header.bytesWritten(), fp) != header.bytesWritten())
        goto fail_with_error;

    fclose(fp);
    progress.close();
    QMessageBox::information(this, "Success", "Export successful!");
    close();
    return;

fail_with_error:
    QMessageBox::critical(this, "Write error", "Failed to write data to file \"" + ui_->lineEdit_packFileName->text() + "\"\n\n" + strerror(errno));
out: 
    fclose(fp);
    close();
}

}
