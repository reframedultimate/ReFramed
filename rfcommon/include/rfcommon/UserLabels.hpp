#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include <cstdio>

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

private:
    friend class UserMotionLabels;
    Vector<Entry> table;
    HashMap<FighterMotion, int, FighterMotion::Hasher> motionMap;
    HashMap<String, SmallVector<int, 4>> userMap;
};

class RFCOMMON_PUBLIC_API UserMotionLabels
{
    UserMotionLabels(Hash40Strings* hash40Strings);

public:
    ~UserMotionLabels();

    static UserMotionLabels* makeEmpty(Hash40Strings* hash40Strings);
    static UserMotionLabels* load(Hash40Strings* hash40Strings, const void* address, uint32_t size);
    bool addLayer(const void* address, uint32_t size);
    uint32_t save(FILE* fp) const;

    void addUnknownMotion(FighterID fighterID, FighterMotion motion);

    SmallVector<FighterMotion, 4> toMotion(FighterID fighterID, const char* userLabel) const;
    const char* toUserLabel(FighterID fighterID, FighterMotion motion) const;
    const char* toUserLabel(FighterID fighterID, FighterMotion motion, const char* fallback) const;

    int entryCount(FighterID fighterID) const { return fighters_.count() ? fighters_[fighterID.value()].table.count() : 0; }
    FighterUserMotionLabels::Category categoryAt(FighterID fighterID, int entryIdx) const
            { return fighters_[fighterID.value()].table[entryIdx].category; }
    FighterMotion motionAt(FighterID fighterID, int entryIdx) const
        { return fighters_[fighterID.value()].table[entryIdx].motion; }
    const char* userLabelAt(FighterID fighterID, int entryIdx) const
        { return fighters_[fighterID.value()].table[entryIdx].userLabel.cStr(); }

private:
    Hash40Strings* hash40Strings_;
    Vector<FighterUserMotionLabels> fighters_;
};

}
