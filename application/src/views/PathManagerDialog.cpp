#include "application/views/PathManagerDialog.hpp"
#include "application/widgets/ProgressDialog.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/FilePathResolver.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VideoMeta.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QScrollArea>
#include <QFileDialog>
#include <QLayoutItem>
#include <QRadioButton>
#include <QDirIterator>
#include <QSet>

namespace rfapp {

// ----------------------------------------------------------------------------
static QWidget* createReplayTab(QVBoxLayout* replayPathsLayout, QPushButton* searchForReplays, QPushButton* removeUnusedReplayPaths)
{
    QWidget* replayPathsContainer = new QWidget;
    replayPathsContainer->setLayout(replayPathsLayout);

    QScrollArea* replayPathsScrollArea = new QScrollArea;
    replayPathsScrollArea->setWidget(replayPathsContainer);
    replayPathsScrollArea->setWidgetResizable(true);

    QHBoxLayout* replayToolsLayout = new QHBoxLayout;
    replayToolsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    replayToolsLayout->addWidget(searchForReplays);
    replayToolsLayout->addWidget(removeUnusedReplayPaths);

    QVBoxLayout* replayPathsTabLayout = new QVBoxLayout;
    replayPathsTabLayout->addWidget(replayPathsScrollArea);
    replayPathsTabLayout->addLayout(replayToolsLayout);

    QWidget* replayPathsTab = new QWidget;
    replayPathsTab->setLayout(replayPathsTabLayout);

    return replayPathsTab;
}

// ----------------------------------------------------------------------------
static QWidget* createVideoTab(QVBoxLayout* videoPathsLayout, QPushButton* findMissingVideos, QPushButton* removeUnusedVideoPaths)
{
    QWidget* videoPathsContainer = new QWidget;
    videoPathsContainer->setLayout(videoPathsLayout);

    QScrollArea* videoPathsScrollArea = new QScrollArea;
    videoPathsScrollArea->setWidget(videoPathsContainer);
    videoPathsScrollArea->setWidgetResizable(true);

    QHBoxLayout* videoToolsLayout = new QHBoxLayout;
    videoToolsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    videoToolsLayout->addWidget(findMissingVideos);
    videoToolsLayout->addWidget(removeUnusedVideoPaths);

    QVBoxLayout* videoPathsTabLayout = new QVBoxLayout;
    videoPathsTabLayout->addWidget(videoPathsScrollArea);
    videoPathsTabLayout->addLayout(videoToolsLayout);

    QWidget* videoPathsTab = new QWidget;
    videoPathsTab->setLayout(videoPathsTabLayout);

    return videoPathsTab;
}

// ----------------------------------------------------------------------------
PathManagerDialog::PathManagerDialog(ReplayManager* replayManager, rfcommon::FilePathResolver* pathResolver, QWidget* parent)
    : QDialog(parent)
    , replayManager_(replayManager)
    , pathResolver_(pathResolver)
    , replayPathsLayout_(new QVBoxLayout)
    , videoPathsLayout_(new QVBoxLayout)
{
    setWindowTitle("Search Path Manager");

    QToolButton* addReplayPathButton = new QToolButton;
    addReplayPathButton->setIcon(QIcon::fromTheme("plus"));
    replayPathsLayout_->addWidget(addReplayPathButton);
    replayPathsLayout_->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    QToolButton* addVideoPathButton = new QToolButton;
    addVideoPathButton->setIcon(QIcon::fromTheme("plus"));
    videoPathsLayout_->addWidget(addVideoPathButton);
    videoPathsLayout_->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    for (int i = 0; i != replayManager_->gamePathCount(); ++i)
        addReplayPath(replayManager_->gamePath(i).absolutePath());
    for (int i = 0; i != replayManager_->videoPathCount(); ++i)
        addVideoPath(replayManager_->videoPath(i).absolutePath());
    defaultReplayPath_ = replayManager_->defaultGamePath().absolutePath();

    QPushButton* searchForReplays = new QPushButton("Search for replays...");
    QPushButton* removeUnusedReplayPaths = new QPushButton("Remove unused paths...");

    QPushButton* findMissingVideos = new QPushButton("Find missing videos...");
    QPushButton* removeUnusedVideoPaths = new QPushButton("Remove unused paths...");

    QTabWidget* tabWidget = new QTabWidget;
    tabWidget->addTab(createReplayTab(replayPathsLayout_, searchForReplays, removeUnusedReplayPaths), "Replay Search Paths");
    tabWidget->addTab(createVideoTab(videoPathsLayout_, findMissingVideos, removeUnusedVideoPaths), "Video Search Paths");

    QPushButton* saveButton = new QPushButton("Save");
    QPushButton* cancelButton = new QPushButton("Cancel");

    QHBoxLayout* saveCancelLayout = new QHBoxLayout;
    saveCancelLayout->addWidget(saveButton);
    saveCancelLayout->addWidget(cancelButton);
    saveCancelLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(tabWidget);
    l->addLayout(saveCancelLayout);

    setLayout(l);

    connect(addReplayPathButton, &QToolButton::released, [this] {
        QString path = QFileDialog::getExistingDirectory(this, "Choose replay directory");
        if (path.isEmpty())
            return;
        addReplayPath(path);
    });

    connect(addVideoPathButton, &QToolButton::released, [this] {
        QString path = QFileDialog::getExistingDirectory(this, "Choose video directory");
        if (path.isEmpty())
            return;
        addReplayPath(path);
    });

    connect(searchForReplays, &QPushButton::released, this, &PathManagerDialog::onFindReplaysReleased);
    connect(removeUnusedReplayPaths, &QPushButton::released, this, &PathManagerDialog::onRemoveUnusedReplayPathsReleased);
    connect(findMissingVideos, &QPushButton::released, this, &PathManagerDialog::onFindMissingVideosReleased);
    connect(removeUnusedVideoPaths, &QPushButton::released, this, &PathManagerDialog::onRemoveUnusedVideoPathsReleased);
    connect(saveButton, &QPushButton::released, this, &PathManagerDialog::onSaveReleased);
    connect(cancelButton, &QPushButton::released, this, &PathManagerDialog::close);
}

// ----------------------------------------------------------------------------
PathManagerDialog::~PathManagerDialog()
{
}

// ----------------------------------------------------------------------------
void PathManagerDialog::onFindReplaysReleased()
{
    QString root = QFileDialog::getExistingDirectory(this, "Choose replay directory");
    if (root.isEmpty())
        return;

    ProgressDialog progress("Searching", "Recursively searching for replay files");
    progress.show();
    progress.raise();

    QSet<QString> paths;
    QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
    int updateCounter = 0;
    while (it.hasNext())
    {
        it.next();

        const QFileInfo& fi = it.fileInfo();
        if (updateCounter++ > 100)
        {
            updateCounter = 0;
            progress.setPercent(100, "Scanning " + fi.absolutePath());
        }

        if (fi.fileName().endsWith(".rfr", Qt::CaseInsensitive))
            paths.insert(fi.absolutePath());
    }

    auto haveReplayPath = [this](const QString& path) -> bool {
        for (const auto& p : replayPaths_)
            if (p == path)
                return true;
        return false;
    };

    for (const auto& path : paths)
        if (haveReplayPath(path) == false)
            addReplayPath(path);
}

// ----------------------------------------------------------------------------
void PathManagerDialog::onRemoveUnusedReplayPathsReleased()
{
    QStringList newList;
    for (const auto& path : replayPaths_)
        if (QDir(path).entryList({ "*.rfr" }, QDir::Files).size() || path == defaultReplayPath_)
            newList.push_back(path);

    QLayoutItem* spacerItem = replayPathsLayout_->takeAt(replayPathsLayout_->count() - 1);
    QLayoutItem* addItem = replayPathsLayout_->takeAt(replayPathsLayout_->count() - 1);
    clearLayout(replayPathsLayout_);
    replayPathsLayout_->addItem(addItem);
    replayPathsLayout_->addItem(spacerItem);

    replayPaths_.clear();
    for (const auto& path : newList)
        addReplayPath(path);
}

// ----------------------------------------------------------------------------
void PathManagerDialog::onFindMissingVideosReleased()
{
    QString root = QFileDialog::getExistingDirectory(this, "Choose directory to search");
    if (root.isEmpty())
        return;

    ProgressDialog progress("Searching", "Recursively searching for video files");
    progress.show();
    progress.raise();

    ReplayGroup* all = replayManager_->allReplayGroup();
    QSet<QString> missingVideoFileNames;
    int counter = 0;
    int i = 0;
    for (const auto& fileName : all->files())
    {
        assert(QDir(fileName).isRelative());

        if (counter++ > 10)
        {
            counter = 0;
            progress.setPercent(100 * i / all->files().size(), "Gathering video metadata from replay " + fileName);
        }
        i++;

        rfcommon::String sessionFilePathUtf8 = pathResolver_->resolveGameFile(fileName.toUtf8().constData());
        rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(pathResolver_, sessionFilePathUtf8.cStr());
        if (session.isNull())
            continue;

        rfcommon::Reference<rfcommon::VideoMeta> vmeta = session->tryGetVideoMeta();
        if (vmeta.isNull())
            continue;
        if (strlen(vmeta->fileName()) == 0)
            continue;

        rfcommon::String videoFilePathUtf8 = pathResolver_->resolveVideoFile(vmeta->fileName());
        if (videoFilePathUtf8.length() == 0)
            missingVideoFileNames.insert(QString::fromUtf8(vmeta->fileName()));
    }

    QSet<QString> paths;
    QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();

        const QFileInfo& fi = it.fileInfo();
        if (counter++ > 100)
        {
            counter = 0;
            progress.setPercent(100, "Scanning " + fi.absolutePath());
        }

        if (missingVideoFileNames.contains(fi.fileName()))
            paths.insert(fi.absolutePath());
    }

    auto haveVideoPath = [this](const QString& path) -> bool {
        for (const auto& p : videoPaths_)
            if (p == path)
                return true;
        return false;
    };

    for (const auto& path : paths)
        if (haveVideoPath(path) == false)
            addVideoPath(path);
}

// ----------------------------------------------------------------------------
void PathManagerDialog::onRemoveUnusedVideoPathsReleased()
{

}

// ----------------------------------------------------------------------------
void PathManagerDialog::onSaveReleased()
{
    auto haveReplayPath = [this](const QString& path) -> bool {
        for (const auto& p : replayPaths_)
            if (p == path)
                return true;
        return false;
    };

    auto haveVideoPath = [this](const QString& path) -> bool {
        for (const auto& p : videoPaths_)
            if (p == path)
                return true;
        return false;
    };

    QStringList replayPathsToRemove;
    for (int i = 0; i != replayManager_->gamePathCount(); ++i)
    {
        QString path = replayManager_->gamePath(i).absolutePath();
        if (haveReplayPath(path) == false)
            replayPathsToRemove.push_back(path);
    }

    QStringList videoPathsToRemove;
    for (int i = 0; i != replayManager_->videoPathCount(); ++i)
    {
        QString path = replayManager_->videoPath(i).absolutePath();
        if (haveVideoPath(path) == false)
            videoPathsToRemove.push_back(path);
    }

    for (const auto& path : replayPathsToRemove)
        replayManager_->removeGamePath(path);
    for (const auto& path : videoPathsToRemove)
        replayManager_->removeVideoPath(path);

    for (const auto& path : replayPaths_)
        replayManager_->addGamePath(path);
    for (const auto& path : videoPaths_)
        replayManager_->addVideoPath(path);

    if (replayManager_->defaultGamePath().absolutePath() != defaultReplayPath_)
        replayManager_->setDefaultGamePath(defaultReplayPath_);

    close();
}

// ----------------------------------------------------------------------------
void PathManagerDialog::addReplayPath(const QString& path)
{
    QLayoutItem* spacerItem = replayPathsLayout_->takeAt(replayPathsLayout_->count() - 1);
    QLayoutItem* addItem = replayPathsLayout_->takeAt(replayPathsLayout_->count() - 1);

    QLineEdit* lineEdit = new QLineEdit;
    lineEdit->setReadOnly(true);
    lineEdit->setText(path);
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QToolButton* choosePath = new QToolButton;
    choosePath->setIcon(QIcon::fromTheme("more-horizontal"));

    QToolButton* removePath = new QToolButton;
    removePath->setIcon(QIcon::fromTheme("x"));

    QRadioButton* defaultReplayPath = new QRadioButton;
    defaultReplayPath->setToolTip("Set as default path. Replays are saved to this location.");
    if (replayManager_->defaultGamePath().absolutePath() == path)
    {
        defaultReplayPath->setChecked(true);
        removePath->setEnabled(false);
    }

    QHBoxLayout* containerLayout = new QHBoxLayout;
    containerLayout->addWidget(defaultReplayPath);
    containerLayout->addWidget(lineEdit);
    containerLayout->addWidget(choosePath);
    containerLayout->addWidget(removePath);

    replayPathsLayout_->addLayout(containerLayout);
    replayPathsLayout_->addItem(addItem);
    replayPathsLayout_->addItem(spacerItem);

    replayPaths_.push_back(path);

    connect(choosePath, &QToolButton::released, [this, containerLayout, lineEdit] {
        QString path = QFileDialog::getExistingDirectory(this, "Choose path with replays");
        if (path.isEmpty())
            return;
        lineEdit->setText(path);

        for (int i = 0; i != replayPathsLayout_->count(); ++i)
        {
            QLayoutItem* item = replayPathsLayout_->itemAt(i);
            if (item->layout() == containerLayout)
            {
                replayPaths_[i] = path;
                break;
            }
        }
    });

    connect(removePath, &QToolButton::released, [this, containerLayout] {
        for (int i = 0; i != replayPathsLayout_->count(); ++i)
        {
            QLayoutItem* item = replayPathsLayout_->itemAt(i);
            if (item->layout() == containerLayout)
            {
                item = replayPathsLayout_->takeAt(i);
                clearLayout(item->layout());
                delete item;

                replayPaths_.erase(replayPaths_.begin() + i);
                break;
            }
        }
    });

    connect(defaultReplayPath, &QRadioButton::toggled, [this, removePath, lineEdit](int checked) {
        removePath->setEnabled(!checked);
        if (checked)
            defaultReplayPath_ = lineEdit->text();
    });
}

// ----------------------------------------------------------------------------
void PathManagerDialog::addVideoPath(const QString& path)
{
    QLayoutItem* spacerItem = videoPathsLayout_->takeAt(videoPathsLayout_->count() - 1);
    QLayoutItem* addItem = videoPathsLayout_->takeAt(videoPathsLayout_->count() - 1);

    QLineEdit* lineEdit = new QLineEdit;
    lineEdit->setReadOnly(true);
    lineEdit->setText(path);

    QToolButton* choosePath = new QToolButton;
    choosePath->setIcon(QIcon::fromTheme("more-horizontal"));

    QToolButton* removePath = new QToolButton;
    removePath->setIcon(QIcon::fromTheme("x"));

    QHBoxLayout* containerLayout = new QHBoxLayout;
    containerLayout->addWidget(lineEdit);
    containerLayout->addWidget(choosePath);
    containerLayout->addWidget(removePath);

    videoPathsLayout_->addLayout(containerLayout);
    videoPathsLayout_->addItem(addItem);
    videoPathsLayout_->addItem(spacerItem);

    videoPaths_.push_back(path);

    connect(choosePath, &QToolButton::released, [this, containerLayout, lineEdit] {
        QString path = QFileDialog::getExistingDirectory(this, "Choose path with replays");
        if (path.isEmpty())
            return;
        lineEdit->setText(path);

        for (int i = 0; i != videoPathsLayout_->count(); ++i)
        {
            QLayoutItem* item = videoPathsLayout_->itemAt(i);
            if (item->layout() == containerLayout)
            {
                videoPaths_[i] = path;
                break;
            }
        }
    });

    connect(removePath, &QToolButton::released, [this, containerLayout] {
        for (int i = 0; i != videoPathsLayout_->count(); ++i)
        {
            QLayoutItem* item = videoPathsLayout_->itemAt(i);
            if (item->layout() == containerLayout)
            {
                item = videoPathsLayout_->takeAt(i);
                clearLayout(item->layout());
                delete item;

                videoPaths_.erase(videoPaths_.begin() + i);
                break;
            }
        }
    });
}

}
