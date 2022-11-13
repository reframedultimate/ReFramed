#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/ReplayFileParts.hpp"
#include "rfcommon/TrainingMetaData.hpp"
#include <cctype>
#include <ctime>

namespace rfcommon {

// ----------------------------------------------------------------------------
ReplayFileParts::ReplayFileParts(
        String originalFileName,
        SmallVector<String, 2>&& playerNames,
        SmallVector<String, 2>&& characterNames,
        String date,
        String time,
        String stage,
        EventType event,
        Round round,
        SetFormat format,
        ScoreCount score,
        uint8_t loserSide)
    : originalFileName_(std::move(originalFileName))
    , playerNames_(std::move(playerNames))
    , fighterNames_(std::move(characterNames))
    , date_(date)
    , time_(time)
    , stage_(stage)
    , event_(event)
    , score_(score)
    , round_(round)
    , format_(format)
    , loserSide_(loserSide)
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
static int parseEvent(int pos, const char* fn, String* event)
{
    const char* s_event;
    const char* p;

    p = fn + pos;

    while (*p && (*p == ' ' || *p == '-'))
        ++p;
    if (!*p)
        return -1;

    s_event = p;
    for (; *p; ++p)
        if ((*p == ' ' && *(p+1) == '-') || *p == '-')
        {
            *event = String(s_event, p - s_event);
            return p - fn;
        }

    return -1;
}

// ----------------------------------------------------------------------------
static int parseSetFormatAndRound(int pos, const char* fn, String* setFormat, String* round)
{
    const char* s_format;
    const char* s_round;
    const char* p;
    int format_len, round_len;

    p = fn + pos;

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

    while (*p && *p != '(')
        ++p;
    if (!*p)
        return -1;

    s_round = p + 1;

    for (; *p; ++p)
        if (*p == ')')
        {
            round_len = p - s_round;
            break;
        }
    if (!*p)
        return -1;

    *setFormat = String(s_format, format_len);
    *round = String(s_round, round_len);
    return p - fn;
}

// ----------------------------------------------------------------------------
static int parsePlayers(int pos, const char* fn, SmallVector<String, 2>* players, SmallVector<String, 2>* fighters, uint8_t* loserSide)
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
    while (*p && *p == ' ')
        ++p;

    if (*p == '[')
    {
        int idx = players->count();
        if (*(p+1) == 'L' && *(p+2) == ']')
            (*loserSide) |= (1 << idx);
        else
            return -1;

        p += 3;
    }

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
static int parseGameAndScore(int pos, const char* fn, int* game, int* p1Score, int* p2Score)
{
    const char* s_game;
    const char* p;
    int game_len;

    p = fn + pos - 1;

retry:
    ++p; if (!*p) return -1; if (*p != 'G') goto retry;
    ++p; if (!*p) return -1; if (*p != 'a') goto retry;
    ++p; if (!*p) return -1; if (*p != 'm') goto retry;
    ++p; if (!*p) return -1; if (*p != 'e') goto retry;
    ++p; if (!*p) return -1; if (*p != ' ') goto retry;

    ++p;
    if (!*p || std::isdigit(*p) == false)
        return -1;

    s_game = p;
    while (*p && std::isdigit(*p))
        ++p;
    if (!*p)
        return -1;
    game_len = p - s_game;

    while (*p && *p == ' ')
        ++p;
    if (!*p)
        return -1;

    if (*p == '(')
    {
        const char* s_left;
        const char* s_right;
        int left_len, right_len;

        if (!*++p)
            return -1;

        s_left = p;
        while (*p && *p != '-')
            ++p;
        left_len = p - s_left;
        if (*p != '-')
            return -1;
        ++p;

        s_right = p;
        while (*p && *p != ')')
            ++p;
        right_len = p - s_right;
        if (!*p)
            return -1;

        *p1Score = atoi(String(s_left, left_len).cStr());
        *p2Score = atoi(String(s_right, right_len).cStr());
    }

    *game = atoi(String(s_game, game_len).cStr());
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
    //   2020-05-23_19-45-01 - Singles - Bo3 (WR2) - TheComet (Pika) vs TAEL (Falcon) - Game 3 (1-1) - Kalos.rfr

    SmallVector<String, 2> players;
    SmallVector<String, 2> characters;
    String date, time, event, stage, setFormat, round;
    int gameNumber = 1, p1Score = -1, p2Score = -1;
    uint8_t loserSide = 0;

    int pos = parseDateTime(fileName, &date, &time);
    if (pos >= 0)
        pos = parseEvent(pos, fileName, &event);
    if (pos >= 0)
        pos = parseSetFormatAndRound(pos, fileName, &setFormat, &round);
    if (pos >= 0)
        pos = parsePlayers(pos, fileName, &players, &characters, &loserSide);
    if (pos >= 0)
        pos = parseGameAndScore(pos, fileName, &gameNumber, &p1Score, &p2Score);
    if (pos >= 0)
        pos = parseStage(pos, fileName, &stage);

    auto roundObj = Round::fromDescription(round.cStr());
    if (loserSide > 2 && roundObj.number().value() == 1)
        roundObj = Round::fromType(roundObj.type(), SessionNumber::fromValue(2));

    return ReplayFileParts(
                fileName,
                std::move(players),
                std::move(characters),
                date, time, stage,
                EventType::fromDescription(event.cStr()),
                roundObj,
                SetFormat::fromDescription(setFormat.cStr()),
                p1Score == -1 ? ScoreCount::fromGameNumber(GameNumber::fromValue(gameNumber)) : ScoreCount::fromScore(p1Score, p2Score),
                loserSide);
}

// ----------------------------------------------------------------------------
ReplayFileParts ReplayFileParts::fromMetaData(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata)
{
    ReplayFileParts parts("", {}, {}, "", "", "",
            EventType::fromType(EventType::OTHER),
            Round::makeFree(),
            SetFormat::fromType(SetFormat::FREE),
            ScoreCount::fromScore(0, 0),
            0x00);

    parts.updateFromMetaData(map, mdata);
    return parts;
}

// ----------------------------------------------------------------------------
void ReplayFileParts::updateFromMetaData(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata)
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
            playerNames_ = { mdata->playerTag(0), "CPU" };
            fighterNames_ = { map->fighter.toName(mdata->playerFighterID(0)), map->fighter.toName(mdata->playerFighterID(1)) };
            date_ = date;
            time_ = time;
            stage_ = map->stage.toName(mdata->stageID());
            score_ = ScoreCount::fromScore(0, 0);
            round_ = Round::fromSessionNumber(mdata->asTraining()->sessionNumber());
            format_ = SetFormat::fromDescription("Training");
        } break;

        case MetaData::GAME: {
            auto gdata = mdata->asGame();

            playerNames_.clear();
            fighterNames_.clear();
            for (int i = 0; i != gdata->fighterCount(); ++i)
            {
                playerNames_.emplace(gdata->playerName(i));
                fighterNames_.emplace(map->fighter.toName(gdata->playerFighterID(i)));
            }

            date_ = date;
            time_ = time;
            stage_ = map->stage.toName(gdata->stageID());
            score_ = gdata->score();
            round_ = gdata->round();
            format_ = gdata->setFormat();

            loserSide_ = 0;
            for (int i = 0; i != gdata->fighterCount(); ++i)
            {
                if (gdata->playerIsLoserSide(i))
                    loserSide_ |= (1 << i);
            }
        } break;
    }
}

// ----------------------------------------------------------------------------
String ReplayFileParts::toFileName() const
{
    // Examples:
    //   2020-05-23_19-45-01 - Singles - Bo3 (WR2) - TheComet (Pika) vs TAEL (Falcon) - Game 3 (1-1) - Kalos.rfr
    //   2020-05-23_19-45-01 - Friendlies - Free (5) - TheComet (Pika) vs TAEL (Falcon) - Game 2 (0-1) - Kalos.rfr

    String playerChars;
    for (int i = 0; i != playerNames_.count(); ++i)
    {
        if (i != 0)
            playerChars += " vs ";
        playerChars += playerNames_[i] + " (" + fighterNames_[i] + ")";
        if (loserSide_ & (1<<i))
            playerChars += " [L]";
    }

    return date_ + "_"
            + time_.copy().replaceAll(':', '-') + " - "
            + event_.description() + " - "
            + format_.shortDescription() + " ("
            + round_.shortDescription() + ") - "
            + playerChars + " - "
            + "Game " + String::decimal(score_.gameNumber().value()) + " ("
            + String::decimal(score_.left()) + "-" + String::decimal(score_.right()) + ") - "
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

    if (fighterNames_.count() == 0)
        return true;
    for (const auto& name : fighterNames_)
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
