#include "rfcommon/Profiler.hpp"
#include "application/models/Config.hpp"

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
    QFile f(dir.absoluteFilePath("config.json"));
    if (!f.open(QIODevice::ReadOnly))
        return;
    root = QJsonDocument::fromJson(f.readAll()).object();
}

// ----------------------------------------------------------------------------
void Config::save()
{
    PROFILE(Config, save);

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QFileInfo pathInfo(dir.path());
    if (!pathInfo.exists())
        QDir().mkdir(pathInfo.filePath());

    QFile f(dir.absoluteFilePath("config.json"));
    if (!f.open(QIODevice::WriteOnly))
        return;
    f.write(QJsonDocument(root).toJson());
}

}
