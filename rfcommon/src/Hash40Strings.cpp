#include "rfcommon/Deserializer.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Profiler.hpp"

#include <memory>

namespace rfcommon {

// ----------------------------------------------------------------------------
Hash40Strings::Hash40Strings()
{}

// ----------------------------------------------------------------------------
Hash40Strings::~Hash40Strings()
{}

// ----------------------------------------------------------------------------
Hash40Strings* Hash40Strings::loadCSV(const char* fileName)
{
    PROFILE(Hash40Strings, loadCSV);

#define fopen_nolint fopen
    FILE* fp = fopen_nolint(fileName, "rb");
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
            Log::root()->warning("Label string empty for \"%s\"", line);
            continue;
        }

        const auto motion = FighterMotion::fromHexString(line);
        if (motion.isValid() == false)
        {
            Log::root()->warning("Invalid hex value \"%s\"\n", line);
            continue;
        }

        auto motionMapResult = hash40Strings->entries_.insertIfNew(motion, labelStr);
        if (motionMapResult == hash40Strings->entries_.end())
        {
            Log::root()->warning("Duplicate motion value: %s\n", line);
            continue;
        }
    }
    fclose(fp);

    Log::root()->info("Loaded %d motion labels\n", hash40Strings->entries_.count());

    return hash40Strings;
}

// ----------------------------------------------------------------------------
Hash40Strings* Hash40Strings::loadBinary(const char* fileName)
{
    PROFILE(Hash40Strings, loadBinary);

    MappedFile f;
    if (f.open(fileName) == false)
        return nullptr;

    Hash40Strings* hash40Strings = new Hash40Strings();
    Deserializer d(f.address(), f.size());

    while (d.bytesLeft())
    {
        const uint32_t motion_l = d.readLU32();
        const uint8_t motion_h = d.readU8();
        const auto motion = FighterMotion::fromParts(motion_h, motion_l);

        const uint8_t len = d.readU8();
        const char* labelStr = static_cast<const char*>(d.readFromPtr(len));
        hash40Strings->entries_.insertAlways(motion, labelStr);
    }

    Log::root()->info("Loaded %d motion labels\n", hash40Strings->entries_.count());

    return hash40Strings;
}

// ----------------------------------------------------------------------------
Hash40Strings* Hash40Strings::makeEmpty()
{
    NOPROFILE();

    return new Hash40Strings();
}

// ----------------------------------------------------------------------------
const char* Hash40Strings::toString(FighterMotion motion) const
{
    NOPROFILE();

    return toString(motion, "(unknown)");
}

// ----------------------------------------------------------------------------
const char* Hash40Strings::toString(FighterMotion motion, const char* fallback) const
{
    PROFILE(Hash40Strings, toString);

    auto it = entries_.find(motion);
    if (it != entries_.end())
        return it->value().cStr();
    return fallback;
}

// ----------------------------------------------------------------------------
FighterMotion Hash40Strings::toMotion(const char* str) const
{
    PROFILE(Hash40Strings, toMotion);

    // Have to do 1 lookup to avoid hash collisions
    const FighterMotion motion = hash40(str);
    if (entries_.find(motion) == entries_.end())
        return FighterMotion::makeInvalid();
    return motion;
}

}
