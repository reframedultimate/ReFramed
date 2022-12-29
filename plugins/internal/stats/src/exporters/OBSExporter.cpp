#include "rfcommon/Profiler.hpp"
#include "stats/exporters/OBSExporter.hpp"
#include "stats/models/SettingsModel.hpp"
#include "stats/util/StatsFormatter.hpp"

// ----------------------------------------------------------------------------
OBSExporter::OBSExporter(
        const PlayerMeta* playerMeta,
        const StatsCalculator* stats,
        const SettingsModel* settings)
    : playerMeta_(playerMeta)
    , stats_(stats)
    , settings_(settings)
{
}

// ----------------------------------------------------------------------------
bool OBSExporter::exportEmptyValues() const
{
    PROFILE(OBSExporter, exportEmptyValues);

    return 
        writeNames() &&
        writeScene() &&
        writePlayerCharacters() &&
        writePlayerTags() &&
        writePlayerStatsEmpty();
}

// ----------------------------------------------------------------------------
bool OBSExporter::exportStatistics() const
{
    PROFILE(OBSExporter, exportStatistics);

    return
        writeNames() &&
        writeScene() &&
        writePlayerCharacters() &&
        writePlayerTags() &&
        writePlayerStats();
}

// ----------------------------------------------------------------------------
void OBSExporter::setPlayerTag(int idx, const QString& tag)
{
    PROFILE(OBSExporter, setPlayerTag);

    while (tags_.size() <= idx)
        tags_.push_back("--");

    tags_[idx] = tag;
}

// ----------------------------------------------------------------------------
void OBSExporter::setPlayerCharacter(int idx, const QString& character)
{
    PROFILE(OBSExporter, setPlayerCharacter);

    while (chars_.size() <= idx)
        chars_.push_back("--");

    chars_[idx] = character;
}

// ----------------------------------------------------------------------------
bool OBSExporter::writeNames() const
{
    PROFILE(OBSExporter, writeNames);

    QFile names(settings_->obsDestinationFolder().absoluteFilePath("names.txt"));
    if (names.open(QIODevice::WriteOnly) == false)
        return false;

    for (int i = 0; i != settings_->numStatsEnabled(); ++i)
    {
        // Additional newlines
        if (i > 0)
            for (int n = 0; n != settings_->obsAdditionalNewlines(); ++n)
                names.write("\n");

        StatType type = settings_->statAtIndex(i);
        names.write(statTypeToString(type));
        names.write("\n");
    }

    return true;
}

// ----------------------------------------------------------------------------
bool OBSExporter::writeScene() const
{
    PROFILE(OBSExporter, writeScene);

    QFile scene(settings_->obsDestinationFolder().absoluteFilePath("obsscene.txt"));
    if (scene.open(QIODevice::WriteOnly) == false)
        return false;

    scene.write("Statistics\n");
    return true;
}

// ----------------------------------------------------------------------------
bool OBSExporter::writePlayerCharacters() const
{
    PROFILE(OBSExporter, writePlayerCharacters);

    QFile p1char(settings_->obsDestinationFolder().absoluteFilePath("p1char.txt"));
    QFile p2char(settings_->obsDestinationFolder().absoluteFilePath("p2char.txt"));

    if (p1char.open(QIODevice::WriteOnly) == false)
        return false;
    if (p2char.open(QIODevice::WriteOnly) == false)
        return false;

    QString p1str = chars_.count() > 0 ? chars_[0] : "--";
    QByteArray p1utf8 = p1str.toUtf8();
    p1char.write(p1utf8.constData(), p1utf8.length());

    QString p2str = chars_.count() > 1 ? chars_[1] : "--";
    QByteArray p2utf8 = p2str.toUtf8();
    p2char.write(p2utf8.constData(), p2utf8.length());

    return true;
}

// ----------------------------------------------------------------------------
bool OBSExporter::writePlayerTags() const
{
    PROFILE(OBSExporter, writePlayerTags);

    QFile p1tag(settings_->obsDestinationFolder().absoluteFilePath("p1tag.txt"));
    QFile p2tag(settings_->obsDestinationFolder().absoluteFilePath("p2tag.txt"));

    if (p1tag.open(QIODevice::WriteOnly) == false)
        return false;
    if (p2tag.open(QIODevice::WriteOnly) == false)
        return false;

    QString p1str = tags_.count() > 0 ? tags_[0] : "Player 1";
    QByteArray p1utf8 = p1str.toUtf8();
    p1tag.write(p1utf8.constData(), p1utf8.length());

    QString p2str = tags_.count() > 1 ? tags_[1] : "Player 2";
    QByteArray p2utf8 = p2str.toUtf8();
    p2tag.write(p2utf8.constData(), p2utf8.length());

    return true;
}

// ----------------------------------------------------------------------------
bool OBSExporter::writePlayerStats() const
{
    PROFILE(OBSExporter, writePlayerStats);

    QFile p1stats(settings_->obsDestinationFolder().absoluteFilePath("p1stats.txt"));
    QFile p2stats(settings_->obsDestinationFolder().absoluteFilePath("p2stats.txt"));

    if (p1stats.open(QIODevice::WriteOnly) == false)
        return false;
    if (p2stats.open(QIODevice::WriteOnly) == false)
        return false;

    StatsFormatter formatter(stats_, playerMeta_);
    for (int i = 0; i != settings_->numStatsEnabled(); ++i)
    {
        // Additional newlines
        if (i > 0)
            for (int n = 0; n != settings_->obsAdditionalNewlines(); ++n)
            {
                p1stats.write("\n");
                p2stats.write("\n");
            }

        StatType type = settings_->statAtIndex(i);

        QString s1 = formatter.playerStatAsString(0, type);
        QByteArray ba1 = s1.toUtf8();
        p1stats.write(ba1.constData(), ba1.length());
        p1stats.write("\n");
        
        QString s2 = formatter.playerStatAsString(1, type);
        QByteArray ba2 = s2.toUtf8();
        p2stats.write(ba2.constData(), ba2.length());
        p2stats.write("\n");
    }

    return true;
}

// ----------------------------------------------------------------------------
bool OBSExporter::writePlayerStatsEmpty() const
{
    PROFILE(OBSExporter, writePlayerStatsEmpty);

    QFile p1stats(settings_->obsDestinationFolder().absoluteFilePath("p1stats.txt"));
    QFile p2stats(settings_->obsDestinationFolder().absoluteFilePath("p2stats.txt"));

    if (p1stats.open(QIODevice::WriteOnly) == false)
        return false;
    if (p2stats.open(QIODevice::WriteOnly) == false)
        return false;

    for (int i = 0; i != settings_->numStatsEnabled(); ++i)
    {
        // Additional newlines
        if (i > 0)
            for (int n = 0; n != settings_->obsAdditionalNewlines(); ++n)
            {
                p1stats.write("\n");
                p2stats.write("\n");
            }

        p1stats.write("--\n");
        p2stats.write("--\n");
    }

    return true;
}
