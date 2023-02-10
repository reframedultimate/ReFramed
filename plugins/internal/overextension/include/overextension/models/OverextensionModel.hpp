#pragma once

#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class GameMetadata;
}

class OverextensionListener;

class OverextensionModel
{
public:
    void startNewSession(const rfcommon::MappingInfo* map, const rfcommon::GameMetadata* mdata);
    void addFrameNoNotify(int frameIdx, const rfcommon::FrameData* fdata);
    void addFrame(int frameIdx, const rfcommon::FrameData* fdata);
    void addAllFrames(const rfcommon::FrameData* fdata);
    void clearAll();

    int fighterCount() const;
    const rfcommon::String& playerName(int fighterIdx) const;
    const rfcommon::String& fighterName(int fighterIdx) const;

    void setOpponentEarliestEscape(int fighterIdx, int frame);
    int opponentEarliestEscape(int fighterIdx) const;

    int numTotal(int fighterIdx) const;
    int numTrueCombos(int fighterIdx) const;
    int numCombos(int fighterIdx) const;
    int numWinningOverextensions(int fighterIdx) const;
    int numLosingOverextensions(int fighterIdx) const;
    int numTradingOverextensions(int fighterIdx) const;

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
        Player(const rfcommon::String& name, const rfcommon::String& tag, const rfcommon::String& fighter)
            : name(name), tag(tag), fighter(fighter)
        {}

        const rfcommon::String name;
        const rfcommon::String tag;
        const rfcommon::String fighter;

        rfcommon::Vector<Moves> moves;
        rfcommon::Vector<int> trueCombos;
        rfcommon::Vector<int> combos;
        rfcommon::Vector<int> winningOverextensions;
        rfcommon::Vector<int> losingOverextensions;
        rfcommon::Vector<int> tradingOverextensions;

        int opponentEarliestEscape = 4;
    };

    rfcommon::Vector<Player> players_;

    int playerMap_[2];
    int upperGapThreshold_ = 20;
};
