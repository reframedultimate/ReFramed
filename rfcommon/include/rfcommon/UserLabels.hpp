#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
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

class Hash40Strings;

class RFCOMMON_PUBLIC_API FighterUserMotionLabels
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
        Category category;
    };

    FighterUserMotionLabels();
    ~FighterUserMotionLabels();

    SmallVector<Entry, 4> toEntry(const char* userLabel) const;
    SmallVector<FighterMotion, 4> toMotion(const char* userLabel) const;
    const char* toUserLabel(FighterMotion motion) const;

private:
    Vector<Entry> entries_;
    HashMap<String, SmallVector<int, 4>> userMap_;
};

class RFCOMMON_PUBLIC_API UserMotionLabels
{
public:
    UserMotionLabels(Hash40Strings* hash40Strings);
    ~UserMotionLabels();

    const FighterUserMotionLabels& fighter(FighterID fighterID) const;

private:
    Hash40Strings* hash40Strings_;
    Vector<FighterUserMotionLabels> fighters_;
};

}
