#include "application/models/ReplayMetadataCache.hpp"
#include "application/models/ReplayGroup.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/FilePathResolver.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TrainingMetadata.hpp"

#include <QStandardPaths>
#include <QDir>
#include <QDataStream>

namespace rfapp {

static ReplayMetadataCache::Entry dummy = {
    "", "", "", "", "", "", "", "", "",
    rfcommon::FighterID::makeInvalid(), rfcommon::FighterID::makeInvalid(),
    rfcommon::Costume::makeDefault(), rfcommon::Costume::makeDefault()
};

// ----------------------------------------------------------------------------
ReplayMetadataCache::ReplayMetadataCache(ReplayManager* manager, rfcommon::FilePathResolver* pathResolver)
    : manager_(manager)
    , pathResolver_(pathResolver)
{
    load();

    // We only need to listen to events of the "All" group, because replays that
    // are added to user-created groups must necessarily also exist in the "All"
    // group
    manager_->allReplayGroup()->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ReplayMetadataCache::~ReplayMetadataCache()
{
    manager_->allReplayGroup()->dispatcher.removeListener(this);

    save();
}

// ----------------------------------------------------------------------------
void ReplayMetadataCache::load()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QString filePath = dir.absoluteFilePath("replayCache.dat");

    rfcommon::Log::root()->info("Opening file %s", filePath.toUtf8().constData());
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
    {
        rfcommon::Log::root()->error("Failed to open file %s. Can't load replay metadata.", filePath.toUtf8().constData());
        return;
    }

    QDataStream s(&f);
    quint16 version;
    s >> version;
    if (version != 0x0000)
    {
        rfcommon::Log::root()->notice("Replay cache has an older format. Won't load. Initial startup will be slower...");
        return;
    }

    s.setVersion(QDataStream::Qt_5_15);

    quint32 replayCount;
    s >> replayCount;
    while (replayCount--)
    {
        QString fileName;
        Entry entry = dummy;
        quint8 p1fighter, p2fighter;
        quint8 p1costume, p2costume;
        s >> fileName;
        s >> entry.date;
        s >> entry.time;
        s >> entry.p1name;
        s >> entry.p2name;
        s >> entry.stage;
        s >> entry.round;
        s >> entry.format;
        s >> entry.score;
        s >> entry.game;
        s >> p1fighter;
        s >> p2fighter;
        s >> p1costume;
        s >> p2costume;
        entry.p1fighterID = rfcommon::FighterID::fromValue(p1fighter);
        entry.p2fighterID = rfcommon::FighterID::fromValue(p2fighter);
        entry.p1costume = rfcommon::Costume::fromValue(p1fighter);
        entry.p2costume = rfcommon::Costume::fromValue(p2fighter);

        entries_.insert(fileName, entry);
    }
}

// ----------------------------------------------------------------------------
void ReplayMetadataCache::save()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QString filePath = dir.absoluteFilePath("replayCache.dat");

    rfcommon::Log::root()->info("Saving file %s", filePath.toUtf8().constData());
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly))
    {
        rfcommon::Log::root()->error("Failed to open file %s. Can't save replay metadata.", filePath.toUtf8().constData());
        return;
    }

    QDataStream s(&f);
    quint16 version = 0x0000;
    s << version;

    s.setVersion(QDataStream::Qt_5_15);

    s << (quint32)entries_.size();
    for (auto it = entries_.begin(); it != entries_.end(); ++it)
    {
        s << it.key();  // filename
        s << it.value().date;
        s << it.value().time;
        s << it.value().p1name;
        s << it.value().p2name;
        s << it.value().stage;
        s << it.value().round;
        s << it.value().format;
        s << it.value().score;
        s << it.value().game;
        s << (quint8)it.value().p1fighterID.value();
        s << (quint8)it.value().p2fighterID.value();
        s << (quint8)it.value().p1costume.value();
        s << (quint8)it.value().p2costume.value();
    }
}

// ----------------------------------------------------------------------------
const ReplayMetadataCache::Entry* ReplayMetadataCache::lookupFilename(const QString& filename)
{
    auto it = entries_.find(filename);
    if (it != entries_.end())
        return &it.value();

    const rfcommon::String filePathUtf8 = pathResolver_->resolveGameFile(filename.toUtf8().constData());
    if (filePathUtf8.length() == 0)
        return &dummy;

    rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(pathResolver_, filePathUtf8.cStr());
    if (session.isNull())
        return &dummy;

    if (auto map = session->tryGetMappingInfo())
        if (auto mdata = session->tryGetMetadata())
            return newEntry(filename, map, mdata);

    return &dummy;
}

// ----------------------------------------------------------------------------
const ReplayMetadataCache::Entry* ReplayMetadataCache::newEntry(const QString& filename, rfcommon::MappingInfo* map, rfcommon::Metadata* mdata)
{
    char date[11], time[9];
    const auto stampMs = mdata->timeStarted().millisSinceEpoch();
    std::time_t t = (std::time_t)(stampMs / 1000);
    std::tm* tm = std::localtime(&t);
    std::strftime(date, 11, "%Y-%m-%d", tm);
    std::strftime(time, 9, "%H-%M-%S", tm);

    switch (mdata->type())
    {
    case rfcommon::Metadata::GAME: {
        rfcommon::GameMetadata* g = mdata->asGame();

        assert(g->playerFighterID(0).isValid());
        assert(g->playerFighterID(1).isValid());

        auto it = entries_.insert(filename, Entry{
            date, time,
            QString::fromUtf8(g->playerName(0).cStr()),
            QString::fromUtf8(g->playerName(1).cStr()),
            QString::fromUtf8(map->stage.toName(mdata->stageID())),
            QString::fromUtf8(g->round().shortDescription().cStr()),
            QString::fromUtf8(g->setFormat().shortDescription()),
            QString::number(g->score().left()) + "-" + QString::number(g->score().right()),
            QString::number(g->score().gameNumber().value()),
            mdata->playerFighterID(0),
            mdata->playerFighterID(1),
            mdata->playerCostume(0),
            mdata->playerCostume(1),
        });
        return &it.value();
    } break;

    case rfcommon::Metadata::TRAINING: {
        rfcommon::TrainingMetadata* t = mdata->asTraining();
        auto it = entries_.insert(filename, Entry{
            date, time,
            QString::fromUtf8(mdata->playerTag(0).cStr()),
            QString::fromUtf8("CPU"),
            QString::fromUtf8(map->stage.toName(mdata->stageID())),
            QString::number(t->sessionNumber().value()),
            "Training",
            "",
            "",
            t->humanFighterID(),
            t->cpuFighterID(),
            mdata->playerCostume(0),
            mdata->playerCostume(1),
        });
        return &it.value();
    } break;
    }

    return &dummy;
}

// ----------------------------------------------------------------------------
void ReplayMetadataCache::onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName)
{
}
void ReplayMetadataCache::onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName)
{
    auto it = entries_.find(fileName);
    if (it == entries_.end())
        return;

    entries_.erase(it);

    // When a file is removed we have to save, otherwise it could be that
    // the app crashes before it gets to save stuff and then the next time
    // it is run, the cache will contain the previous state of that replay.
    save();
}

}
