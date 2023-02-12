#include "overextension/models/OverextensionModel.hpp"
#include "overextension/listeners/OverextensionListener.hpp"

#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/MappingInfo.hpp"

// Shield
static const auto FIGHTER_STATUS_KIND_GUARD_DAMAGE = rfcommon::FighterStatus::fromValue(30);

// Grabs
static const auto FIGHTER_STATUS_KIND_CAPTURE_PULLED = rfcommon::FighterStatus::fromValue(65);
static const auto FIGHTER_STATUS_KIND_THROWN = rfcommon::FighterStatus::fromValue(70);

// Tech/No Techs
static const auto FIGHTER_STATUS_KIND_DOWN = rfcommon::FighterStatus::fromValue(80);
static const auto FIGHTER_STATUS_KIND_DOWN_DAMAGE = rfcommon::FighterStatus::fromValue(90);
static const auto FIGHTER_STATUS_KIND_PASSIVE = rfcommon::FighterStatus::fromValue(103);  // This occurs during teching?

// Dodges
static const auto FIGHTER_STATUS_KIND_ESCAPE_AIR = rfcommon::FighterStatus::fromValue(34);

// Ledge
static const auto FIGHTER_STATUS_KIND_CLIFF_CATCH = rfcommon::FighterStatus::fromValue(118);
static const auto FIGHTER_STATUS_KIND_CLIFF_WAIT = rfcommon::FighterStatus::fromValue(119);

// ----------------------------------------------------------------------------
// Bury and grab moves don't influence hitstun so it's possible for a player to
// be in a state where they are being combo'd, but their hitstun can drop to 0
// if the combo involves a grab or bury. To solve this, we "modify" the hitstun
// by returning a non-zero value if they are in those states.
static float modifiedHitstun(const rfcommon::FighterState& state)
{
    for (auto status : {
        FIGHTER_STATUS_KIND_GUARD_DAMAGE,
        FIGHTER_STATUS_KIND_CAPTURE_PULLED,
        FIGHTER_STATUS_KIND_THROWN,
        FIGHTER_STATUS_KIND_DOWN,
        FIGHTER_STATUS_KIND_DOWN_DAMAGE,
        FIGHTER_STATUS_KIND_PASSIVE
    }){
        if (state.status() == status)
            return 1.0;  // Any value over 0.0 is fine, we choose 1 because some
                         // checks do before > after and this has the effect of
                         // "holding" the player at the lowest possible hitstun
                         // value
    }

    return state.hitstun();
}

// ----------------------------------------------------------------------------
static bool canAct(const rfcommon::FighterState& state)
{
    for (auto status : {
        FIGHTER_STATUS_KIND_GUARD_DAMAGE,
        FIGHTER_STATUS_KIND_CAPTURE_PULLED,
        FIGHTER_STATUS_KIND_THROWN,
        FIGHTER_STATUS_KIND_DOWN,
        FIGHTER_STATUS_KIND_DOWN_DAMAGE,
        FIGHTER_STATUS_KIND_PASSIVE })
    {
        if (state.status() == status)
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
const char* OverextensionModel::categoryName(Category category)
{
    static const char* names[] = {
        "True combos",
        "Combos",
        "Winning overextensions",
        "Losing overextensions",
        "Trading overextensions"
    };
    return names[category];
}

// ----------------------------------------------------------------------------
void OverextensionModel::startNewSession(const rfcommon::MappingInfo* map, const rfcommon::GameMetadata* mdata)
{
    assert(mdata->fighterCount() == 2);

    // Ensure we have a slot allocated in the players array
    // Map the fighter indices into our players array
    for (int i = 0; i != mdata->fighterCount(); ++i)
    {
        for (int j = 0; j != players_.count(); ++j)
            if (players_[j].name == mdata->playerName(i))
            {
                playerMap_[i] = j;
                goto skip_add;
            }

        playerMap_[i] = players_.count();
        players_.emplace(
            mdata->playerName(i),
            mdata->playerTag(i),
            mdata->playerFighterID(i),
            map->fighter.toName(mdata->playerFighterID(i)));
        opponentEarliestEscapeOptions_.insertIfNew(players_.back().fighter, 3);
    skip_add:;
    }

    dispatcher.dispatch(&OverextensionListener::onPlayersChanged);
}

// ----------------------------------------------------------------------------
void OverextensionModel::addFrameNoNotify(int frameIdx, const rfcommon::FrameData* fdata)
{
    assert(fdata->fighterCount() == 2);

    for (int p = 0; p != fdata->fighterCount(); ++p)
    {
        const auto& me = fdata->stateAt(p, frameIdx);
        const auto& mePrev = frameIdx > 0 ? fdata->stateAt(p, frameIdx - 1) : me;
        const auto& them = fdata->stateAt(1 - p, frameIdx);
        const auto& themPrev = frameIdx > 0 ? fdata->stateAt(1 - p, frameIdx - 1) : them;

        // Opponent was hit by a new move, or we were hit
        if (modifiedHitstun(them) > modifiedHitstun(themPrev) ||
            (modifiedHitstun(me) > 0.0 && modifiedHitstun(mePrev) == 0.0))
        {
            // Search backwards and find the previous move the opponent was hit with
            // We limit the search up to a maximum "gap" between being in hitstun.
            int gap;
            for (gap = 0; gap < upperGapThreshold_; ++gap)
            {
                if (frameIdx - gap - 1 < 0)  // No more frames to look back
                    goto not_enough_history;
                const auto& state = fdata->stateAt(1 - p, frameIdx - gap - 1);
                if (modifiedHitstun(state) > 0.0)  // Opponent is in hitstun again, success
                    break;
            }
            if (gap < upperGapThreshold_)
            {
                // Follow hitstun backwards until we get to the beginning of the move
                // This lets us figure out what move "me" used to create the hitstun.
                int startFrame = frameIdx - gap - 1;
                for (;; startFrame--)
                {
                    if (startFrame < 1)
                        goto not_enough_history;

                    const auto& after  = fdata->stateAt(1 - p, startFrame);
                    const auto& before = fdata->stateAt(1 - p, startFrame - 1);
                    if (before.hitstun() < after.hitstun())
                        break;
                }

                auto& player = players_[playerMap_[p]];
                if (modifiedHitstun(me) > 0.0)
                {
                    if (modifiedHitstun(them) > 0.0)
                        player.category[TRADING_OVEREXTENSION].push(player.moves.count());
                    else
                        player.category[LOSING_OVEREXTENSION].push(player.moves.count());
                }
                else if (gap == 0)
                    player.category[TRUE_COMBO].push(player.moves.count());
                else if (gap < opponentEarliestEscapeOptions_.findKey(player.fighter)->value())
                    player.category[COMBO].push(player.moves.count());
                else
                    player.category[WINNING_OVEREXTENSION].push(player.moves.count());

                players_[playerMap_[p]].moves.emplace(
                    fdata->stateAt(p, startFrame).motion(),
                    me.motion(),
                    rfcommon::FrameIndex::fromValue(startFrame),
                    me.frameIndex(),
                    gap
                );
            }
        }

    not_enough_history:;
    }
}

// ---------------------------------------------------------------------------
void OverextensionModel::addFrame(int frameIdx, const rfcommon::FrameData* fdata)
{
    addFrameNoNotify(frameIdx, fdata);
    dispatcher.dispatch(&OverextensionListener::onDataChanged);
}

// ----------------------------------------------------------------------------
void OverextensionModel::addAllFrames(const rfcommon::FrameData* fdata)
{
    for (int i = 0; i != fdata->frameCount(); ++i)
        addFrameNoNotify(i, fdata);
    dispatcher.dispatch(&OverextensionListener::onDataChanged);
}

// ----------------------------------------------------------------------------
void OverextensionModel::clearAll()
{
    players_.clear();

    dispatcher.dispatch(&OverextensionListener::onPlayersChanged);
    dispatcher.dispatch(&OverextensionListener::onDataChanged);
}

// ----------------------------------------------------------------------------
int OverextensionModel::fighterCount() const
{
    return players_.count();
}

// ----------------------------------------------------------------------------
int OverextensionModel::currentFighter() const
{
    return currentFighter_;
}

// ----------------------------------------------------------------------------
void OverextensionModel::setCurrentFighter(int fighterIdx)
{
    currentFighter_ = fighterIdx;
    dispatcher.dispatch(&OverextensionListener::onCurrentFighterChanged, fighterIdx);
}

// ----------------------------------------------------------------------------
const rfcommon::String& OverextensionModel::playerName(int fighterIdx) const
{
    return players_[fighterIdx].name;
}

// ----------------------------------------------------------------------------
const rfcommon::String& OverextensionModel::fighterName(int fighterIdx) const
{
    return players_[fighterIdx].fighter;
}

// ----------------------------------------------------------------------------
rfcommon::FighterID OverextensionModel::fighterID(int fighterIdx) const
{
    return players_[fighterIdx].fighterID;
}

// ----------------------------------------------------------------------------
void OverextensionModel::setOpponentEarliestEscape(int fighterIdx, int frame)
{
    opponentEarliestEscapeOptions_.insertAlways(players_[fighterIdx].fighter, frame);

    // Any combos that are over the new earliest escape frame now count
    // as overextensions instead of combos
    players_[fighterIdx].category[COMBO].retain([this, frame, fighterIdx](int i) -> bool {
        if (players_[fighterIdx].moves[i].gap >= frame)
        {
            players_[fighterIdx].category[WINNING_OVEREXTENSION].push(i);
            return false;
        }
        return true;
    });

    // Any overextensions that are under the new earliest escape frame now
    // count as combos instead of overextensions
    players_[fighterIdx].category[WINNING_OVEREXTENSION].retain([this, frame, fighterIdx](int i) -> bool {
        if (players_[fighterIdx].moves[i].gap < frame)
        {
            players_[fighterIdx].category[COMBO].push(i);
            return false;
        }
        return true;
    });

    dispatcher.dispatch(&OverextensionListener::onDataChanged);
}

// ----------------------------------------------------------------------------
int OverextensionModel::opponentEarliestEscape(int fighterIdx) const
{
    auto it = opponentEarliestEscapeOptions_.findKey(players_[fighterIdx].fighter);
    assert (it != opponentEarliestEscapeOptions_.end());
    return it->value();
}
