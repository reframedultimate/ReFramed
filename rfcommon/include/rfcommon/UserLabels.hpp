#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/Types.hpp"
#include <cassert>

#define RFCOMMON_USER_LABEL_CATEGORIES_LIST \
    X(MOVEMENT, "Movement") \
    X(GROUND_ATTACKS, "Ground Attacks") \
    X(AERIAL_ATTACKS, "Aerial Attacks") \
    X(SPECIAL_ATTACKS, "Special Attacks") \
    X(GRABS, "Grabs") \
    X(LEDGE, "Ledge") \
    X(DEFENSIVE, "Defensive") \
    X(MISC, "Misc") \
    X(UNLABELED, "Unlabeled")

namespace rfcommon {

class RFCOMMON_PUBLIC_API FighterUserLabels
{
public:
    enum MatchFlags
    {
        MATCH_MOTION = 0x01,
        MATCH_STATUS = 0x02,
    };

    enum Category
    {
#define X(name, desc) name,
        RFCOMMON_USER_LABEL_CATEGORIES_LIST
#undef X
        COUNT
    };

    struct Entry
    {
        String userLabel;
        FighterMotion motion;
        FighterStatus status;
        Category category;
        uint8_t matchFlags;
    };

private:
    Vector<Entry> entries_;
    HashMap<FighterMotion, int, FighterMotion::Hasher> motionMap_;
    HashMap<String, SmallVector<int, 4>> userMap_;
};

class RFCOMMON_PUBLIC_API UserLabels
{
public:
    bool loadJSON(const char* fileName);

    const FighterUserLabels& fighter(FighterID fighterID) const
        { assert(fighterID.value() < fighters_.count()); return fighters_[fighterID.value()]; }

private:
    Vector<FighterUserLabels> fighters_;
};

}
