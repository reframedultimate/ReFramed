#include "rfcommon/Profiler.hpp"
#include "stats/util/Paths.hpp"

#include <QStandardPaths>
#include <QDebug>

// ----------------------------------------------------------------------------
bool ensureDataDirExists()
{
    PROFILE(PathsGlobal, ensureDataDirExists);

    QDir dir = dataDir();
    if (dir.exists() == false)
        dir.mkpath(dir.absolutePath());

    qDebug() << dir;

    return dir.exists();
}

// ----------------------------------------------------------------------------
const QDir dataDir()
{
    PROFILE(PathsGlobal, dataDir);

    const QString datadir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return QDir(datadir).filePath("stats");
}
