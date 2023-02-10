#include "overextension/models/OverextensionModel.hpp"
#include "overextension/listeners/OverextensionListener.hpp"

#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/MappingInfo.hpp"

static const auto FIGHTER_STATUS_KIND_CAPTURE_PULLED = rfcommon::FighterStatus::fromValue(65);
static const auto FIGHTER_STATUS_KIND_THROWN = rfcommon::FighterStatus::fromValue(70);

// ----------------------------------------------------------------------------
// Bury and grab moves don't influence hitstun so it's possible for a player to
// be in a state where they are being combo'd, but their hitstun can drop to 0
// if the combo involves a grab or bury. To solve this, we "modify" the hitstun
// by returning a non-zero value if they are in those states.
static float modifiedHitstun(const rfcommon::FighterState& state)
{
    for (auto status : {
        FIGHTER_STATUS_KIND_CAPTURE_PULLED,
        FIGHTER_STATUS_KIND_THROWN })
    {
        if (state.status() == status)
            return 1.0;  // Any value over 0.0 is fine, we choose 1 because some
                         // checks do before > after and this has the effect of
                         // "holding" the player at the lowest possible hitstun
                         // value
    }

    return state.hitstun();
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
            map->fighter.toName(mdata->playerFighterID(i)));
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
                        player.tradingOverextensions.push(player.moves.count());
                    else
                        player.losingOverextensions.push(player.moves.count());
                }
                else if (gap == 0)
                    player.trueCombos.push(player.moves.count());
                else if (gap < player.opponentEarliestEscape)
                    player.combos.push(player.moves.count());
                else
                    player.winningOverextensions.push(player.moves.count());

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
void OverextensionModel::setOpponentEarliestEscape(int fighterIdx, int frame)
{
    players_[fighterIdx].opponentEarliestEscape = frame;

    // Any combos that are over the new earliest escape frame now count
    // as overextensions instead of combos
    players_[fighterIdx].combos.retain([this, frame, fighterIdx](int i) -> bool {
        if (players_[fighterIdx].moves[i].gap >= frame)
        {
            players_[fighterIdx].winningOverextensions.push(i);
            return false;
        }
        return true;
    });

    // Any overextensions that are under the new earliest escape frame now
    // count as combos instead of overextensions
    players_[fighterIdx].winningOverextensions.retain([this, frame, fighterIdx](int i) -> bool {
        if (players_[fighterIdx].moves[i].gap < frame)
        {
            players_[fighterIdx].combos.push(i);
            return false;
        }
        return true;
    });

    dispatcher.dispatch(&OverextensionListener::onDataChanged);
}

// ----------------------------------------------------------------------------
int OverextensionModel::opponentEarliestEscape(int fighterIdx) const
{
    return players_[fighterIdx].opponentEarliestEscape;
}

// ----------------------------------------------------------------------------
int OverextensionModel::numTotal(int fighterIdx) const
{
    return players_[fighterIdx].moves.count();
}

// ----------------------------------------------------------------------------
int OverextensionModel::numTrueCombos(int fighterIdx) const
{
    return players_[fighterIdx].trueCombos.count();
}

// ----------------------------------------------------------------------------
int OverextensionModel::numCombos(int fighterIdx) const
{
    return players_[fighterIdx].combos.count();
}

// ----------------------------------------------------------------------------
int OverextensionModel::numWinningOverextensions(int fighterIdx) const
{
    return players_[fighterIdx].winningOverextensions.count();
}

// ----------------------------------------------------------------------------
int OverextensionModel::numLosingOverextensions(int fighterIdx) const
{
    return players_[fighterIdx].losingOverextensions.count();
}

// ----------------------------------------------------------------------------
int OverextensionModel::numTradingOverextensions(int fighterIdx) const
{
    return players_[fighterIdx].tradingOverextensions.count();
}
