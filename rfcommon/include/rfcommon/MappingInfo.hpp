#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

#define USER_LABEL_CATEGORIES_LIST \
    X(MOVEMENT, "Movement") \
    X(GROUND_ATTACKS, "Ground Attacks") \
    X(AERIAL_ATTACKS, "Aerial Attacks") \
    X(SPECIAL_ATTACKS, "Special Attacks") \
    X(GRABS, "Grabs") \
    X(LEDGE, "Ledge") \
    X(DEFENSIVE, "Defensive") \
    X(MISC, "Misc") \
    X(UNLABELED, "Unlabeled")


enum class UserLabelCategory {
#define X(name, desc) name,
    USER_LABEL_CATEGORIES_LIST
#undef X
    COUNT
};


class RFCOMMON_PUBLIC_API MappingInfoMotion
{
public:
    enum MatchFlags
    {
        MATCH_MOTION = 0x01,
        MATCH_STATUS = 0x02,
    };

    struct Entry
    {
        String statusName;
        SmallVector<String, 2> userLabels;
        FighterMotion motion;
        FighterStatus status;
        UserLabelCategory category;
        uint8_t matchFlags;
    };

private:
    Vector<Entry> entries_;
};

class RFCOMMON_PUBLIC_API MappingInfo
{
public:
};

}
