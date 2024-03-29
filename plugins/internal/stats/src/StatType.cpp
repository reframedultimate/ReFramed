#include "rfcommon/Profiler.hpp"
#include "stats/StatType.hpp"
#include <cstring>

static const char* table[] = {
#define X(type, str, colorcode) str,
        STAT_TYPES_LIST
#undef X
};

const char* statTypeToString(StatType type)
{
    PROFILE(StatTypeGlobal, statTypeToString);

    return table[type];
}

StatType stringToStatType(const char* s)
{
    PROFILE(StatTypeGlobal, stringToStatType);

    for (int i = 0; i != STAT_COUNT; ++i)
        if (strcmp(s, table[i]) == 0)
            return static_cast<StatType>(i);
    return STAT_COUNT;
}
