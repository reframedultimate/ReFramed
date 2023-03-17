#include "application/models/Config.hpp"

#include "rfcommon/Profiler.hpp"
#include "rfcommon/Log.hpp"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>

namespace rfapp {

using namespace nlohmann;

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

    QByteArray ba = f.readAll();
    root = json::parse(ba.begin(), ba.end(), nullptr, false);
    if (root.is_discarded())
    {
        rfcommon::Log::root()->error("Failed to parse config.json");
        return;
    }

    rfcommon::Log::root()->info("Settings loaded");
}

// ----------------------------------------------------------------------------
void Config::save()
{
    PROFILE(Config, save);

    auto log = rfcommon::Log::root();

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QFileInfo pathInfo(dir.path());
    if (!pathInfo.exists())
        QDir().mkdir(pathInfo.filePath());
    QString fileName = dir.absoluteFilePath("config.json");

    log->info("Saving settings to file %s", fileName.toUtf8().constData());
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {
        log->error("Failed to open file %s for writing. Can't save settings.", fileName.toUtf8().constData());
        return;
    }

    const std::string jsonStr = root.dump(2);
    if (f.write(jsonStr.data(), jsonStr.length()) != (int)jsonStr.length())
    {
        log->error("Failed to write data to config.json");
        return;
    }

    log->info("Settings saved");
}

}
