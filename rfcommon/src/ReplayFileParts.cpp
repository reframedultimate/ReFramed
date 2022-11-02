#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/ReplayFileParts.hpp"
#include "rfcommon/TrainingMetaData.hpp"
#include <cctype>
#include <ctime>

namespace rfcommon {

// ----------------------------------------------------------------------------
ReplayFileParts::ReplayFileParts(
        SmallVector<String, 2>&& playerNames,
        SmallVector<String, 2>&& characterNames,
        String date,
        String time,
        String stage,
        SetNumber setNumber,
        GameNumber gameNumber,
        SetFormat format)
    : playerNames_(std::move(playerNames))
    , characterNames_(std::move(characterNames))
    , date_(date)
    , time_(time)
    , stage_(stage)
    , setNumber_(setNumber)
    , gameNumber_(gameNumber)
    , format_(format)
{}
ReplayFileParts::~ReplayFileParts()
{}

// ----------------------------------------------------------------------------
int parseDateTime(const char* fn, String* date, String* time)
{
    const char* s_date = fn;
    const char* s_time = fn;
    const char* p;

retry:
    for (; *s_date; ++s_date)
        if (std::isdigit(*s_date))
            break;
    if (!*s_date)
        return -1;

    p = s_date;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) goto retry;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) goto retry;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) goto retry;
    ++p; if (!*p) return -1; if (*p != '-') goto retry;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) goto retry;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) goto retry;
    ++p; if (!*p) return -1; if (*p != '-') goto retry;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) goto retry;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) goto retry;

    *date = String(s_date, p - s_date + 1);

    ++p; if (!*p) return -1; if (*p != '_') return p - fn;
    s_time = p + 1;

    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) return s_time - fn;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) return s_time - fn;
    ++p; if (!*p) return -1; if (*p != '-') return s_time - fn;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) return s_time - fn;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) return s_time - fn;
    ++p; if (!*p) return -1; if (*p != '-') return s_time - fn;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) return s_time - fn;
    ++p; if (!*p) return -1; if (std::isdigit(*p) == false) return s_time - fn;

    *time = String(s_time, p - s_time + 1).replaceAll('-', ':');
    return p - fn;
}

// ----------------------------------------------------------------------------
static int parseSetFormatAndNumber(int pos, const char* fn, String* setFormat, int* setNumber)
{
    const char* s_format;
    const char* s_number;
    const char* p;
    int format_len;

    p = fn + pos;

retry:
    for (; *p; ++p)
        if (std::isalnum(*p))
            break;
    if (!*p)
        return -1;

    s_format = p;
    for (; *p; ++p)
        if (std::isalnum(*p) == false)
        {
            format_len = p - s_format;
            break;
        }
    if (!*p)
        return -1;

    for (; *p; ++p)
        if (std::isspace(*p) == false && *p != '(')
            break;
    if (!*p)
        return -1;

    if (std::isdigit(*p) == false)
        goto retry;

    s_number = p;
    for (; *p; ++p)
        if (std::isdigit(*p) == false)
        {
            if (*p != ')')
                goto retry;
            break;
        }
    if (!*p)
        return -1;

    *setFormat = String(s_format, format_len);
    *setNumber = std::atoi(String(s_number, p - s_number).cStr());
    return p - fn;
}

// ----------------------------------------------------------------------------
static int parsePlayers(int pos, const char* fn, SmallVector<String, 2>* players, SmallVector<String, 2>* fighters)
{
    const char* s_player;
    const char* s_fighter;
    const char* p;
    int player_len, fighter_len;

    // Skip " - "
    p = fn + pos;
    for (; *p; ++p)
        if (*p < 0 || std::isalnum(*p))
            break;
    if (!*p)
        return -1;

next_player:
    s_player = p;
    for (; *p; ++p)
        if (
                (*p == '(') ||
                (*p == ' ' && *(p+1) == '(') ||
                (*p == ' ' && *(p+1) == 'v' && *(p+2) == 's'))
        {
            player_len = p - s_player;
            break;
        }

    while (*p && std::isspace(*p))
        ++p;
    if (!*p)
        return -1;

    if (*p != '(')
    {
        s_fighter = "(unknown fighter)";
        fighter_len = sizeof("(unknown fighter)");
        goto skip_fighter;
    }

    while (*p && *p == '(')
        ++p;
    if (!*p)
        return -1;

    if (std::isalnum(*p) == false)
        return -1;

    s_fighter = p;
    for (; *p; ++p)
        if (*p == ')')
        {
            fighter_len = p - s_fighter;
            break;
        }
    if (*p != ')')
        return -1;
    while (*p && *p == ')')
        ++p;

skip_fighter:
    players->emplace(s_player, player_len);
    fighters->emplace(s_fighter, fighter_len);

    while (*p && std::isspace(*p))
        ++p;
    if (*p == 'v' && *(p+1) == 's')
    {
        p += 2;
        while (*p >= 0 && std::isspace(*p))
            ++p;
        if (!*p)
            return -1;
        goto next_player;
    }

    return p - fn;
}

// ----------------------------------------------------------------------------
static int parseGame(int pos, const char* fn, int* game)
{
    const char* s_game;
    const char* p;

    p = fn + pos - 1;

retry:
    ++p; if (!*p) return -1; if (*p != 'G') goto retry;
    ++p; if (!*p) return -1; if (*p != 'a') goto retry;
    ++p; if (!*p) return -1; if (*p != 'm') goto retry;
    ++p; if (!*p) return -1; if (*p != 'e') goto retry;
    ++p; if (!*p) return -1; if (std::isspace(*p) == false) goto retry;

    ++p;
    if (!*p || std::isdigit(*p) == false)
        return -1;

    s_game = p;
    while (*p && std::isdigit(*p))
        ++p;
    if (!*p)
        return -1;

    *game = atoi(String(s_game, p - s_game).cStr());
    return p - fn;
}

// ----------------------------------------------------------------------------
static int parseStage(int pos, const char* fn, String* stage)
{
    const char* s_stage;
    const char* p;

    // Skip " - "
    p = fn + pos;
    for (; *p; ++p)
        if (std::isalnum(*p))
            break;
    if (!*p)
        return -1;

    s_stage = p;
    for (; *p; ++p)
        if (*p == '.' && (*(p+1) == 'r' || *(p+1) == 'R'))
        {
            *stage = String(s_stage, p - s_stage);
            return p - fn;
        }

    return -1;
}

// ----------------------------------------------------------------------------
ReplayFileParts ReplayFileParts::fromFileName(const char* fileName)
{
    // Example:
    //   2020-05-23_19-45-01 - Bo3 (2) - TheComet (Pika) vs TAEL (Falcon) - Game 3 - Kalos.rfr

    SmallVector<String, 2> players;
    SmallVector<String, 2> characters;
    String date, time, stage, setFormat;
    int setNumber = 1, gameNumber = 1;

    int pos = parseDateTime(fileName, &date, &time);
    if (pos >= 0)
        pos = parseSetFormatAndNumber(pos, fileName, &setFormat, &setNumber);
    if (pos >= 0)
        pos = parsePlayers(pos, fileName, &players, &characters);
    if (pos >= 0)
        pos = parseGame(pos, fileName, &gameNumber);
    if (pos >= 0)
        pos = parseStage(pos, fileName, &stage);

    return ReplayFileParts(
                std::move(players),
                std::move(characters),
                date, time, stage,
                SetNumber::fromValue(setNumber),
                GameNumber::fromValue(gameNumber),
                SetFormat::fromDescription(setFormat.cStr()));
}

// ----------------------------------------------------------------------------
ReplayFileParts ReplayFileParts::fromMetaData(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata)
{
    const auto stampMs = mdata->timeStarted().millisSinceEpoch();
    std::time_t t = (std::time_t)(stampMs / 1000);
    std::tm* tm = std::localtime(&t);
    char date[11];
    char time[9];
    std::strftime(date, 11, "%Y-%m-%d", tm);
    std::strftime(time, 9, "%H:%M:%S", tm);

    switch (mdata->type())
    {
        case MetaData::TRAINING: {
            return ReplayFileParts(
                        {mdata->name(0), "CPU"},
                        {map->fighter.toName(mdata->fighterID(0)), map->fighter.toName(mdata->fighterID(1))},
                        date,
                        time,
                        map->stage.toName(mdata->stageID()),
                        SetNumber::fromValue(1),
                        mdata->asTraining()->sessionNumber(),
                        SetFormat::fromDescription("Training"));
        } break;

        case MetaData::GAME: {
            SmallVector<String, 2> names, fighters;
            for (int i = 0; i != mdata->fighterCount(); ++i)
            {
                names.emplace(mdata->name(i));
                fighters.emplace(map->fighter.toName(mdata->fighterID(i)));
            }

            auto gdata = mdata->asGame();
            return ReplayFileParts(
                        std::move(names),
                        std::move(fighters),
                        date,
                        time,
                        map->stage.toName(mdata->stageID()),
                        gdata->setNumber(),
                        gdata->gameNumber(),
                        gdata->setFormat());
        } break;
    }

    std::terminate();
}

// ----------------------------------------------------------------------------
String ReplayFileParts::toFileName() const
{
    // Example:
    //   2020-05-23_19-45-01 - Bo3 (2) - TheComet (Pika) vs TAEL (Falcon) Game 3 - Kalos.rfr

    String playerChars;
    for (int i = 0; i != playerNames_.count(); ++i)
    {
        if (i != 0)
            playerChars += " vs ";
        playerChars += playerNames_[i] + " (" + characterNames_[i] + ")";
    }

    return date_ + "_"
            + time_.copy().replaceAll(':', '-') + " - "
            + format_.shortDescription() + " ("
            + String::decimal(setNumber_.value()) + ") - "
            + playerChars + " - "
            + "Game " + String::decimal(gameNumber_.value()) + " - "
            + stage_ +
            + ".rfr";
}

// ----------------------------------------------------------------------------
bool ReplayFileParts::hasMissingInfo() const
{
    if (playerNames_.count() == 0)
        return true;
    for (const auto& name : playerNames_)
        if (name.length() == 0)
            return true;

    if (characterNames_.count() == 0)
        return true;
    for (const auto& name : characterNames_)
        if (name.length() == 0)
            return true;

    if (date_.length() == 0 ||
        time_.length() == 0 ||
        stage_.length() == 0)
    {
        return true;
    }

    return false;
}

}
