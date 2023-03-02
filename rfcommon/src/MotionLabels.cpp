#include "rfcommon/Deserializer.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/LastError.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Serializer.hpp"
#include "rfcommon/Utf8.hpp"

#include "nlohmann/json.hpp"

#include <cstdio>

namespace rfcommon {

using nlohmann::json;

// ----------------------------------------------------------------------------
MotionLabels::MotionLabels()
{

}

// ----------------------------------------------------------------------------
MotionLabels::~MotionLabels()
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::load(const char* fileNameUtf8)
{
    Log* log = Log::root();

    MappedFile f;
    log->info("Loading motion labels binary file \"%s\"", fileNameUtf8);
    if (f.open(fileNameUtf8) == false)
    {
        log->error("Failed to open file \"%s\": %s", fileNameUtf8, LastError().cStr());
        return false;
    }

    Deserializer d(f.address(), f.size());
    const uint8_t major = d.readU8();
    const uint8_t minor = d.readU8();
    if (major != 1 || minor != 0)
    {
        log->error("Failed to load file \"%s\": Unsupported version %d.%d", fileNameUtf8, (int)major, (int)minor);
        return false;
    }

    // Load hash40 table
    const int hash40Count = d.readLU32();
    for (int i = 0; i != hash40Count; ++i)
    {
        const uint32_t lower = d.readLU32();
        const uint8_t upper = d.readU8();
        const uint8_t labelLen = d.readU8();
        hash40s_.insertIfNew(
                FighterMotion::fromParts(upper, lower),
                SmallString<31>(static_cast<const char*>(d.readFromPtr(labelLen)), labelLen));
    }

    // Load all layer names
    const int layerCount = d.readLU16();
    for (int i = 0; i != layerCount; ++i)
    {
        int len = d.readU8();
        layerNames_.emplace(static_cast<const char*>(d.readFromPtr(len)), len);
    }

    // Load layers
    const int fighterCount = d.readU8();
    for (int fighterIdx = 0; fighterIdx != fighterCount; ++fighterIdx)
    {
        const int rowCount = d.readLU16();
        Fighter& fighter = fighters_.emplace();
        fighter.colLayer.resize(layerCount);

        for (int row = 0; row != rowCount; ++row)
        {
            const uint32_t lower = d.readLU32();
            const uint8_t upper = d.readU8();
            const Category category = static_cast<Category>(d.readU8());
            const auto motion = FighterMotion::fromParts(upper, lower);

            fighter.colMotionValue.emplace(motion);
            fighter.colCategory.emplace(category);
            for (int layerIdx = 0; layerIdx != layerCount; ++layerIdx)
            {
                fighter.colLayer[layerIdx].usages.emplace(static_cast<Usage>(d.readU8()));
                const int8_t len = d.readI8();
                fighter.colLayer[layerIdx].labels.emplace(static_cast<const char*>(d.readFromPtr(len)), len);
            }

            if (motion.isValid() == false)
                log->warning("Found invalid motion value in file");
            if (fighter.motionToRow.insertIfNew(motion, row) != fighter.motionToRow.end())
                log->warning("Found duplicate motion value 0x%lx in file", motion.value());
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabels::save(const char* fileNameUtf8)
{
    Log* log = Log::root();

    log->info("Saving motion labels binary file \"%s\"", fileNameUtf8);
    FILE* fp = utf8_fopen_wb(fileNameUtf8, strlen(fileNameUtf8));
    if (fp == nullptr)
    {
        log->error("Failed to open file \"%s\": %s", fileNameUtf8, LastError().cStr());
        return false;
    }

    char scratch[4096];

    // Write file version
    {
        Serializer s(scratch, sizeof *scratch);
        s.writeU8(1);  // major
        s.writeU8(0);  // minor
        fwrite(s.data(), s.bytesWritten(), 1, fp);
    }

    // Save hash40 <-> string map
    {
        Serializer s(scratch, sizeof *scratch);
        s.writeLU32(hash40s_.count());
        fwrite(s.data(), s.bytesWritten(), 1, fp);
    }
    for (auto it : hash40s_)
    {
        Serializer s(scratch, sizeof *scratch);
        s.writeLU32(it.key().lower());
        s.writeU8(it.key().upper());
        s.writeU8(it.value().length());
        fwrite(s.data(), s.bytesWritten(), 1, fp);
        fwrite(it.value().cStr(), it.value().length(), 1, fp);
    }

    // Save layer names
    {
        Serializer s1(scratch, sizeof *scratch);
        s1.writeLU16(layerNames_.count());
        fwrite(s1.data(), s1.bytesWritten(), 1, fp);
        for (const String& name : layerNames_)
        {
            Serializer s2(scratch, sizeof *scratch);
            s2.writeU8(name.length());
            fwrite(s2.data(), s2.bytesWritten(), 1, fp);
            fwrite(name.cStr(), name.length(), 1, fp);
        }
    }

    // Save fighter layers
    {
        // Write number of fighters and number of layers
        Serializer s(scratch, sizeof *scratch);
        s.writeU8(fighters_.count());
        fwrite(s.data(), s.bytesWritten(), 1, fp);
    }
    for (const Fighter& fighter : fighters_)
    {
        // Write number of rows in table for this fighter. The number of
        // columns remains constant for all fighters
        Serializer s1(scratch, sizeof *scratch);
        s1.writeLU16(fighter.colMotionValue.count());
        fwrite(s1.data(), s1.bytesWritten(), 1, fp);

        for (int row = 0; row != fighter.colMotionValue.count(); ++row)
        {
            Serializer s2(scratch, sizeof *scratch);
            s2.writeLU32(fighter.colMotionValue[row].lower());
            s2.writeU8(fighter.colMotionValue[row].upper());
            s2.writeU8(fighter.colCategory[row]);
            fwrite(s2.data(), s2.bytesWritten(), 1, fp);

            for (const Layer& layer : fighter.colLayer)
            {
                Serializer s3(scratch, sizeof *scratch);
                s3.writeU8(layer.usages[row]);
                s3.writeI8(layer.labels[row].length());
                fwrite(s3.data(), s3.bytesWritten(), 1, fp);
                fwrite(layer.labels[row].cStr(), layer.labels[row].length(), 1, fp);
            }
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabels::updateHash40FromCSV(const char* fileNameUtf8)
{
    Log* log = Log::root();

    MappedFile f;
    log->info("Updating hash40 strings from CSV file \"%s\"", fileNameUtf8);
    if (f.open(fileNameUtf8) == false)
    {
        log->error("Failed to open file \"%s\": %s", fileNameUtf8, LastError().cStr());
        return false;
    }

    char hex[32];
    char label[128];
    const char* p = static_cast<const char*>(f.address());
    const char* pe = static_cast<const char*>(f.address()) + f.size();
    int numInserted = 0;
    while (1)
    {
        // Copy next line into hex array until we see a comma
        int i = 0;
        for (; *p != ',' && p != pe && i < (int)sizeof(hex) - 1; ++p, ++i)
            hex[i] = *p;
        if (p == pe || ++p == pe)  // Skip ','
        {
            log->warning("File ended with unexpected data");
            return false;
        }
        hex[i] = '\0';
        if (i >= (int)sizeof(hex) - 1)
        {
            log->warning("Expected delimiter ',' was not encountered in string \"%s\", skipping to next line...", hex);
            goto skip_to_next_line;
        }

        // Continue copying line into label array until we see a newline
        for (i = 0; *p != '\r' && *p != '\n' && p != pe && i < (int)sizeof(label) - 1; ++p, ++i)
            label[i] = *p;
        if (i >= (int)sizeof(label) - 1)
        {
            log->warning("Expected delimiter '[\\r\\n]' was not encountered in string \"%s\", skipping to next line...", hex);
            goto skip_to_next_line;
        }
        label[i] = '\0';

        // label can be empty sometimes
        if (label[0] == '\0')
        {
            log->warning("Label string was empty for hash40 value \"%s\"", hex);
            goto skip_to_next_line;
        }

        {  // "goto skips initialization..."
            const auto motion = FighterMotion::fromHexString(hex);
            if (motion.isValid() == false)
            {
                Log::root()->warning("Invalid hex value \"%s\"\n", hex);
                goto skip_to_next_line;
            }

            auto motionMapResult = hash40s_.insertIfNew(motion, label);
            if (motionMapResult != hash40s_.end())
                numInserted++;
        }

        // Find beginning of next line
skip_to_next_line:
        while (*p != '\n')
            if (++p == pe)
            {
                log->notice("File ended with unexpected data");
                return false;
            }
    }

    log->info("Updated %d hash40 labels\n", numInserted);

    return true;
}

// ----------------------------------------------------------------------------
const char* MotionLabels::lookupHash40(FighterMotion motion, const char* fallback) const
{
    if (auto it = hash40s_.find(motion); it != hash40s_.end())
        return it->value().cStr();
    return fallback;
}

// ----------------------------------------------------------------------------
FighterMotion MotionLabels::toMotion(const char* hash40Str) const
{
    FighterMotion motion = hash40(hash40Str);
    if (hash40s_.find(motion) != hash40s_.end())
        return motion;
    return FighterMotion::makeInvalid();
}

// ----------------------------------------------------------------------------
const char* MotionLabels::lookupLayer(FighterID fighterID, FighterMotion motion, int layerIdx, const char* fallback) const
{
    if (layerIdx < 0)
        return fallback;

    const Fighter& fighter = fighters_[fighterID.value()];
    const auto it = fighter.motionToRow.find(motion);
    if (it == fighter.motionToRow.end())
        return fallback;
    const int row = it->value();

    return fighter.colLayer[layerIdx].labels[row].cStr();
}

// ----------------------------------------------------------------------------
const char* MotionLabels::lookupGroup(FighterID fighterID, FighterMotion motion, Usage usage, int preferredLayerIdx, const char* fallback) const
{
    const Fighter& fighter = fighters_[fighterID.value()];
    const auto it = fighter.motionToRow.find(motion);
    if (it == fighter.motionToRow.end())
        return fallback;
    const int row = it->value();

    if (preferredLayerIdx >= 0)
        if (fighter.colLayer[preferredLayerIdx].labels[row].length() > 0)
            return fighter.colLayer[preferredLayerIdx].labels[row].cStr();

    for (int i = 0; i != layerCount(); ++i)
        if (fighter.colLayer[i].labels[row].length() > 0)
            return fighter.colLayer[i].labels[row].cStr();

    return fallback;
}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> MotionLabels::toMotions(FighterID fighterID, const char* label) const
{
    const Fighter& fighter = fighters_[fighterID.value()];

    SmallVector<FighterMotion, 4> result;
    for (int i = 0; i != layerCount(); ++i)
        fighter.layerMaps[i].labelToRows.find(label)
}

// ----------------------------------------------------------------------------
int MotionLabels::importLayer(const char* fileNameUtf8)
{
    Log* log = Log::root();

    MappedFile file;
    log->info("Importing layer from file \"%s\"", fileNameUtf8);
    if (file.open(fileNameUtf8) == false)
    {
        log->error("Failed to map file \"%s\": %s", fileNameUtf8, LastError().cStr());
        return false;
    }

    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(file.address());
    const unsigned char* const end = static_cast<const unsigned char*>(file.address()) + file.size();
    json j = json::parse(begin, end, nullptr, false);
    if (j.is_discarded())
    {
        log->error("Failed to parse json file: \"%s\"", fileNameUtf8);
        return false;
    }

    if (j["version"] != "1.0")
    {
        log->error("Unsupported version \"%s\" while parsing file \"%s\"", j["version"].get<std::string>().c_str(), fileNameUtf8);
        return false;
    }

    json jName = j["name"];
    if (jName.is_string() == false)
    {
        log->notice("Missing property \"name\", setting default name");
        jName = "Layer";
    }

    json jUsage = j["usage"];
    if (jUsage.is_string() == false)
    {
        log->notice("Missing property \"usage\", setting to default");
        jUsage = "notation";
    }

    Usage usage = [log, &jUsage] {
        if (jUsage == "readable")
            return Usage::READABLE;
        if (jUsage == "notation")
            return Usage::NOTATION;
        if (jUsage == "group")
            return Usage::GROUP;

        log->notice("Usage \"%s\" is invalid. Using default.", jUsage.get<std::string>().c_str());
        return Usage::NOTATION;
    }();

    const int layerIdx = newLayer(jName.get<std::string>().c_str(), usage);

    json jFighters = j["fighters"];
    for (const auto& [fighterIDStr, jFighter] : jFighters.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(fighterIDStr, &pos));
        if (pos != fighterIDStr.length())
            continue;

        json jMotions = jFighter["motions"];
        json jLabels = jFighter["labels"];
        json jCategories = jFighter["categories"];

        if (jMotions.is_array() == false || jLabels.is_array() == false || jCategories.is_array() == false)
            continue;
        if (jMotions.size() != jLabels.size() || jMotions.size() != jCategories.size())
            continue;

        for (int i = 0; i != (int)jMotions.size(); ++i)
        {
            if (jMotions[i].is_number_integer() == false)
                continue;
            if (jLabels[i].is_string() == false)
                continue;
            if (jCategories[i].is_number_unsigned() == false)
                continue;

            const auto category = static_cast<Category>(jCategories[i].get<int>());
            const auto motion = FighterMotion::fromValue(jMotions[i].get<FighterMotion::Type>());
            const auto label = jLabels[i].get<std::string>();

            addNewLabel(fighterID, motion, category, layerIdx, label.c_str());
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabels::exportLayer(int layerIdx, const char* fileNameUtf8)
{
    return false;
}

// ----------------------------------------------------------------------------
bool MotionLabels::exportLayer(int layerIdx, FighterID fighterID, const char* fileNameUtf8)
{
    return false;
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toString(FighterMotion motion, int layerIdx, const char* fallback) const
{
    return nullptr;
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toString(FighterMotion motion, Usage usage, int preferredLayerIdx, const char* fallback) const
{
    return nullptr;
}

// ----------------------------------------------------------------------------
FighterMotion MotionLabels::toMotion(const char* hash40Str) const
{
    return FighterMotion::makeInvalid();
}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> MotionLabels::toMotion(FighterID fighterID, const char* label) const
{
    return SmallVector<FighterMotion, 4>();
}

// ----------------------------------------------------------------------------
int MotionLabels::newLayer(const char* nameUtf8, Usage usage)
{
    return -1;
}

// ----------------------------------------------------------------------------
void MotionLabels::deleteLayer(int layerIdx)
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::renameLayer(int layerIdx, const char* newNameUtf8)
{
    return false;
}

// ----------------------------------------------------------------------------
int MotionLabels::mergeLayers(int targetLayerIdx, int sourceLayerIdx)
{
    return -1;
}

// ----------------------------------------------------------------------------
bool MotionLabels::addUnknownMotion(FighterID fighterID, FighterMotion motion)
{
    return false;
}

// ----------------------------------------------------------------------------
bool MotionLabels::addNewLabel(FighterID fighterID, FighterMotion motion, Category category, int layerIdx, const char* label)
{
    return false;
}

// ----------------------------------------------------------------------------
bool MotionLabels::changeLabel(FighterID fighterID, int entryIdx, int layerIdx, const char* newLabel)
{
    return false;
}

// ----------------------------------------------------------------------------
bool MotionLabels::changeCategory(FighterID fighterID, int entryIdx, Category newCategory)
{
    return false;
}

// ----------------------------------------------------------------------------
void MotionLabels::populateMissingFighters(FighterID fighterID)
{
}

}
