#pragma once

#define RFCOMMON_USER_LABEL_CATEGORIES_LIST \
    X(MOVEMENT, "Movement") \
    X(GROUND_ATTACKS, "Ground Attacks") \
    X(AERIAL_ATTACKS, "Aerial Attacks") \
    X(SPECIAL_ATTACKS, "Special Attacks") \
    X(GRABS, "Grabs") \
    X(LEDGE, "Ledge") \
    X(DEFENSIVE, "Defensive") \
    X(DISADVANTAGE, "Disadvantage") \
    X(ITEMS, "Items") \
    X(MISC, "Misc") \
    X(UNLABELED, "Unlabeled")

namespace rfcommon {

enum UserMotionLabelsCategory
{
#define X(name, desc) name,
    RFCOMMON_USER_LABEL_CATEGORIES_LIST
#undef X
    COUNT
};

}
