#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MotionLabels.hpp"

#include "nlohmann/json.hpp"

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
    return false;
}

// ----------------------------------------------------------------------------
bool MotionLabels::save(const char* fileNameUtf8)
{
    return false;
}

// ----------------------------------------------------------------------------
bool MotionLabels::updateHash40FromCSV(const char* fileNameUtf8)
{
    return false;
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toHash40String(FighterMotion motion, const char* fallback) const
{
    return nullptr;
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

}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> MotionLabels::toMotion(FighterID fighterID, const char* label) const
{

}

// ----------------------------------------------------------------------------
int MotionLabels::importLayer(const char* fileNameUtf8)
{
    Log* log = Log::root();

    MappedFile file;
    log->info("Importing layer from file \"%s\"", fileNameUtf8);
    if (file.open(fileNameUtf8) == false)
    {
        log->error("Failed to map file: \"%s\"", fileNameUtf8);
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

        for (int i = 0; i != jMotions.size(); ++i)
        {
            if (jMotions[i].is_number_integer() == false)
                continue;
            if (jLabels[i].is_string() == false)
                continue;
            if (jCategories[i].is_number_unsigned() == false)
                continue;

            const auto category = static_cast<UserMotionLabelsCategory>(jCategories[i].get<int>());
            const auto motion = FighterMotion::fromValue(jMotions[i].get<FighterMotion::Type>());
            const auto label = jLabels[i].get<std::string>();

            addEntry(fighterID, layerIdx, motion, label.c_str(), category);
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabels::exportLayer(int layerIdx, const char* fileNameUtf8)
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::exportLayer(int layerIdx, FighterID fighterID, const char* fileNameUtf8)
{

}

// ----------------------------------------------------------------------------
int MotionLabels::newLayer(const char* nameUtf8, Usage usage)
{

}

// ----------------------------------------------------------------------------
void MotionLabels::removeLayer(int layerIdx)
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::renameLayer(int layerIdx, const char* newNameUtf8)
{

}

// ----------------------------------------------------------------------------
int MotionLabels::mergeLayers(int targetLayerIdx, int sourceLayerIdx)
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::addUnknownMotion(FighterID fighterID, FighterMotion motion)
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::addEntry(FighterID fighterID, int layerIdx, FighterMotion motion, const char* label, Category category)
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::changeLabel(FighterID fighterID, int layerIdx, FighterMotion motion, const char* newUserLabel)
{

}

// ----------------------------------------------------------------------------
bool MotionLabels::changeCategory(FighterID fighterID, FighterMotion motion, Category newCategory)
{

}

}
