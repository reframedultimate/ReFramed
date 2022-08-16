#include "rfcommon/UserLabels.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterUserMotionLabels::FighterUserMotionLabels()
{}

// ----------------------------------------------------------------------------
FighterUserMotionLabels::~FighterUserMotionLabels()
{}

// ----------------------------------------------------------------------------
UserMotionLabels::UserMotionLabels(Hash40Strings* hash40Strings)
    : hash40Strings_(hash40Strings)
{}

// ----------------------------------------------------------------------------
UserMotionLabels::~UserMotionLabels()
{}

// ----------------------------------------------------------------------------
UserMotionLabels* UserMotionLabels::makeEmpty(Hash40Strings* hash40Strings)
{
    return new UserMotionLabels(hash40Strings);
}

// ----------------------------------------------------------------------------
UserMotionLabels* UserMotionLabels::load(Hash40Strings* hash40Strings, const void* address, uint32_t size)
{
    return nullptr;
}

// ----------------------------------------------------------------------------
bool UserMotionLabels::addLayer(const void* address, uint32_t size)
{
    return false;
}

// ----------------------------------------------------------------------------
uint32_t UserMotionLabels::save(FILE* fp) const
{
    return 0;
}

// ----------------------------------------------------------------------------
void UserMotionLabels::addUnknownMotion(FighterID fighterID, FighterMotion motion)
{
    if (fighters_.count() <= fighterID.value())
        fighters_.resize(fighterID.value() + 1);

    auto& map = fighters_[fighterID.value()].motionMap;
    auto& table = fighters_[fighterID.value()].table;
    if (map.insertIfNew(motion, table.count()) == map.end())
        return;

    table.push({"", motion, FighterUserMotionLabels::UNLABELED});
}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> UserMotionLabels::toMotion(FighterID fighterID, const char* userLabel) const
{
    const auto& map = fighters_[fighterID.value()].userMap;
    const auto& table = fighters_[fighterID.value()].table;

    SmallVector<FighterMotion, 4> motions;
    const auto it = map.find(userLabel);
    if (it != map.end())  // map user label to vetor of indices into the table
        for (int i : it->value())  // for each index in the table, retrieve motion
            motions.push(table[i].motion);
    return motions;
}

// ----------------------------------------------------------------------------
const char* UserMotionLabels::toUserLabel(FighterID fighterID, FighterMotion motion) const
{
    return toUserLabel(fighterID, motion, "(unlabeled)");
}

// ----------------------------------------------------------------------------
const char* UserMotionLabels::toUserLabel(FighterID fighterID, FighterMotion motion, const char* fallback) const
{
    const auto& map = fighters_[fighterID.value()].motionMap;
    const auto& table = fighters_[fighterID.value()].table;

    const auto it = map.find(motion);
    if (it != map.end())
        return table[it->value()].userLabel.cStr();
    return fallback;
}

}
