#pragma once

#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/LinearMap.hpp"
#include "rfcommon/PluginSharedData.hpp"

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class GameMetadata;
}

class OverextensionListener;

class OverextensionModel
{
public:
    enum Category
    {
        TRUE_COMBO,
        COMBO,
        WINNING_OVEREXTENSION,
        LOSING_OVEREXTENSION,
        TRADING_OVEREXTENSION
    };

    static const char* categoryName(Category category);

    void startNewSession(const rfcommon::MappingInfo* map, const rfcommon::GameMetadata* mdata);
    void addFrameNoNotify(int frameIdx, const rfcommon::FrameData* fdata);
    void addFrame(int frameIdx, const rfcommon::FrameData* fdata);
    void addAllFrames(const rfcommon::FrameData* fdata);
    void clearAll();

    int fighterCount() const;
    int currentFighter() const;
    void setCurrentFighter(int fighterIdx);
    const rfcommon::String& playerName(int fighterIdx) const;
    const rfcommon::String& fighterName(int fighterIdx) const;
    rfcommon::FighterID fighterID(int fighterIdx) const;

    void setOpponentEarliestEscape(int fighterIdx, int frame);
    int opponentEarliestEscape(int fighterIdx) const;

    int moveCount(int fighterIdx, Category cat) const { return players_[fighterIdx].category[cat].count(); }
    rfcommon::FighterMotion moveBefore(int fighterIdx, int moveIdx, Category cat) const
    {
        int i = players_[fighterIdx].category[cat][moveIdx];
        return players_[fighterIdx].moves[i].before;
    }
    rfcommon::FighterMotion moveAfter(int fighterIdx, int moveIdx, Category cat) const
    {
        int i = players_[fighterIdx].category[cat][moveIdx];
        return players_[fighterIdx].moves[i].after;
    }
    rfcommon::FrameIndex moveStart(int fighterIdx, int moveIdx, Category cat) const
    {
        int i = players_[fighterIdx].category[cat][moveIdx];
        return players_[fighterIdx].moves[i].start;
    }
    rfcommon::FrameIndex moveEnd(int fighterIdx, int moveIdx, Category cat) const
    {
        int i = players_[fighterIdx].category[cat][moveIdx];
        return players_[fighterIdx].moves[i].end;
    }
    int moveGap(int fighterIdx, int moveIdx, Category cat) const
    {
        int i = players_[fighterIdx].category[cat][moveIdx];
        return players_[fighterIdx].moves[i].gap;
    }

    rfcommon::ListenerDispatcher<OverextensionListener> dispatcher;

private:
    struct Moves
    {
        Moves(rfcommon::FighterMotion before, rfcommon::FighterMotion after, rfcommon::FrameIndex start, rfcommon::FrameIndex end, int gap)
            : before(before), after(after), start(start), end(end), gap(gap)
        {}

        const rfcommon::FighterMotion before, after;
        const rfcommon::FrameIndex start, end;
        const int gap;

        struct Hasher {
            typedef uint32_t HashType;
            HashType operator()(const Moves& moves) const {
                rfcommon::FighterMotion::Hasher h;
                return rfcommon::hash32_combine(h(moves.before), h(moves.after));
            }
        };
    };

    struct Player
    {
        Player(const rfcommon::String& name, const rfcommon::String& tag, rfcommon::FighterID fighterID, const rfcommon::String& fighter)
            : name(name), tag(tag), fighterID(fighterID), fighter(fighter)
        {}

        const rfcommon::String name;
        const rfcommon::String tag;
        const rfcommon::String fighter;
        const rfcommon::FighterID fighterID;

        rfcommon::Vector<Moves> moves;
        rfcommon::Vector<int> category[5];
    };

    rfcommon::Vector<Player> players_;

    // We save these separately so they persist
    rfcommon::SmallLinearMap<rfcommon::String, int, 4> opponentEarliestEscapeOptions_;

    int currentFighter_ = 0;
    int playerMap_[2];
    int upperGapThreshold_ = 40;
};
