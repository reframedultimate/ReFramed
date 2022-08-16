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
Hash40Strings::Hash40Strings()
{}

// ----------------------------------------------------------------------------
Hash40Strings::~Hash40Strings()
{}

// ----------------------------------------------------------------------------
Hash40Strings* Hash40Strings::loadCSV(const char* fileName)
{
    FILE* fp = fopen(fileName, "rb");
    if (fp == nullptr)
        return nullptr;

    Hash40Strings* hash40Strings = new Hash40Strings();
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

        if (labelStr[0] == '\0')
        {
            fprintf(stderr, "Label string empty for \"%s\"", line);
            continue;
        }

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

        auto motionMapResult = hash40Strings->entries_.insertIfNew(motion, labelStr);
        if (motionMapResult == hash40Strings->entries_.end())
        {
            fprintf(stderr, "Duplicate motion value: %s\n", line);
            continue;
        }
    }
    fclose(fp);

    fprintf(stderr, "Loaded %d motion labels\n", hash40Strings->entries_.count());

    return hash40Strings;
}

// ----------------------------------------------------------------------------
Hash40Strings* Hash40Strings::makeEmpty()
{
    return new Hash40Strings();
}

// ----------------------------------------------------------------------------
const char* Hash40Strings::toString(FighterMotion motion) const
{
    auto it = entries_.find(motion);
    if (it != entries_.end())
        return it->value().cStr();
    return "(unknown)";
}

// ----------------------------------------------------------------------------
FighterMotion Hash40Strings::toMotion(const char* str) const
{
    // Have to do 1 lookup to avoid hash collisions
    const FighterMotion motion = hash40(str);
    if (entries_.find(motion) == entries_.end())
        return FighterMotion::makeInvalid();
    return motion;
}

}
