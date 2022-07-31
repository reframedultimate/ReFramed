#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/hash40.hpp"
#include <cstdio>
#include <memory>

namespace rfcommon {

// ----------------------------------------------------------------------------
static uint64_t hexStringToValue(const char* hex, int* error)
{
    uint64_t value = 0;

    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
        hex += 2;

    for (; *hex; ++hex)
    {
        value <<= 4;
        if (*hex >= '0' && *hex <= '9')
            value |= *hex - '0';
        else if (*hex >= 'A' && *hex <= 'F')
            value |= *hex - 'A' + 10;
        else if (*hex >= 'a' && *hex <= 'f')
            value |= *hex - 'a' + 10;
        else
        {
            *error = 1;
            return 0;
        }
    }

    return value;
}

// ----------------------------------------------------------------------------
bool Hash40Strings::loadCSV(const char* fileName)
{
    FILE* fp = fopen(fileName, "rb");
    if (fp == nullptr)
        return false;

    char line[128];
    while (fgets(line, sizeof(line), fp))
    {
        // Split string at comma
        char* delim = line;
        for (; *delim != ',' && *delim; ++delim) {}
        if (!*delim)
            continue;
        *delim = '\0';
        char* labelStr = delim + 1;

        // Remove newline and/or carriage return
        for (delim++; *delim != '\r' && *delim != '\n' && *delim; ++delim) {}
        *delim = '\0';

        int error = 0;
        const auto motion = FighterMotion::fromValue(hexStringToValue(line, &error));
        if (motion.value() == 0)
        {
            if (error)
                fprintf(stderr, "Failed to parse \"%s\" into hex value\n", line);
            else
                fprintf(stderr, "Invalid hex value \"%s\"\n", line);
            continue;
        }

        auto motionMapResult = entries_.insertIfNew(motion, labelStr);
        if (motionMapResult == entries_.end())
        {
            fprintf(stderr, "Duplicate motion value: %s\n", line);
            continue;
        }
    }
    fclose(fp);

    fprintf(stderr, "Loaded %d motion labels\n", entries_.count());

    return true;
}

// ----------------------------------------------------------------------------
const char* Hash40Strings::stringOf(FighterMotion motion) const
{
    auto it = entries_.find(motion);
    if (it != entries_.end())
        return it->value().count() ? it->value().cStr() : nullptr;
    return nullptr;
}

// ----------------------------------------------------------------------------
FighterMotion Hash40Strings::motionOf(const char* str) const
{
    const FighterMotion motion = hash40(str);
#ifndef NDEBUG
    if (entries_.find(motion) == entries_.end())
        fprintf(stderr, "Motion string \"%s\" was not found in the table. Returning the hash40 value anyway...\n", str);
#endif
    return motion;
}

}
