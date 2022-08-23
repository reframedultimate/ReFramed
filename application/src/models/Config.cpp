#include "application/models/Config.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Log.hpp"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>

namespace rfapp {

// ----------------------------------------------------------------------------
Config::Config()
{
    load();
}

// ----------------------------------------------------------------------------
Config::~Config()
{
    save();
}

// ----------------------------------------------------------------------------
void Config::load()
{
    PROFILE(Config, load);

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QString fileName = dir.absoluteFilePath("config.json");

    rfcommon::Log::root()->info("Opening file %s", fileName.toUtf8().constData());
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
    {
        rfcommon::Log::root()->error("Failed to open file %s. Can't load settings.", fileName.toUtf8().constData());
        return;
    }
    root = QJsonDocument::fromJson(f.readAll()).object();
    rfcommon::Log::root()->info("Settings loaded");
}

// ----------------------------------------------------------------------------
void Config::save()
{
    PROFILE(Config, save);

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QFileInfo pathInfo(dir.path());
    if (!pathInfo.exists())
        QDir().mkdir(pathInfo.filePath());
    QString fileName = dir.absoluteFilePath("config.json");

    rfcommon::Log::root()->info("Saving settings to file %s", fileName.toUtf8().constData());
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {
        rfcommon::Log::root()->error("Failed to open file %s for writing. Can't save settings.", fileName.toUtf8().constData());
        return;
    }
    f.write(QJsonDocument(root).toJson());
    rfcommon::Log::root()->info("Settings saved");
}

}
