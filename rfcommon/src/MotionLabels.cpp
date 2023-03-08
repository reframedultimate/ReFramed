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

    // Load all layer names and usages
    const int layerCount = d.readLU16();
    for (int i = 0; i != layerCount; ++i)
    {
        int len = d.readU8();
        layerNames_.emplace(static_cast<const char*>(d.readFromPtr(len)), len);
    }
    for (int i = 0; i != layerCount; ++i)
        layerUsages_.emplace(static_cast<Usage>(d.readU8()));

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

    // Save layer names and usages
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
        for (Usage usage : layerUsages_)
        {
            Serializer s2(scratch, sizeof *scratch);
            s2.writeU8(usage);
            fwrite(s2.data(), s2.bytesWritten(), 1, fp);
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
            return Usage::CATEGORIZATION;

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
const char* MotionLabels::lookupHash40(FighterMotion motion, const char* fallback) const
{
    if (const auto it = hash40s_.find(motion); it != hash40s_.end())
        return it->value().cStr();
    return fallback;
}

// ----------------------------------------------------------------------------
FighterMotion MotionLabels::toMotion(const char* hash40Str) const
{
    FighterMotion motion = hash40(hash40Str);
    if (hash40s_.find(motion) == hash40s_.end())
        return FighterMotion::makeInvalid();
    return motion;
}

// ----------------------------------------------------------------------------
const char* MotionLabels::lookupLayer(FighterID fighterID, FighterMotion motion, int layerIdx, const char* fallback) const
{
    if (layerIdx < 0)
        return fallback;

    // Look up row for specified fighter
    const Fighter& fighter = fighters_[fighterID.value()];
    const auto it = fighter.motionToRow.find(motion);
    if (it == fighter.motionToRow.end())
        return fallback;
    const int row = it->value();

    // Cell could contain an empty string
    const Layer& layer = fighter.colLayer[layerIdx];
    if (layer.labels[row].isEmpty())
        return fallback;

    return layer.labels[row].cStr();
}

// ----------------------------------------------------------------------------
const char* MotionLabels::lookupGroup(FighterID fighterID, FighterMotion motion, Usage usage, int preferredLayerIdx, const char* fallback) const
{
    // Look up row for specified fighter
    const Fighter& fighter = fighters_[fighterID.value()];
    const auto it = fighter.motionToRow.find(motion);
    if (it == fighter.motionToRow.end())
        return fallback;
    const int row = it->value();

    if (preferredLayerIdx >= 0)
    {
        const Layer& layer = fighter.colLayer[preferredLayerIdx];
        if (layer.labels[row].notEmpty())
            return layer.labels[row].cStr();
    }

    for (int layerIdx = 0; layerIdx != layerUsages_.count(); ++layerIdx)
    {
        if (layerUsages_[layerIdx] != usage)
            continue;

        const Layer& layer = fighter.colLayer[layerIdx];
        if (layer.labels[row].notEmpty())
            return layer.labels[row].cStr();
    }

    return fallback;

}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> MotionLabels::toMotions(FighterID fighterID, const char* label) const
{
    SmallVector<FighterMotion, 4> result;

    const Fighter& fighter = fighters_[fighterID.value()];
    for (int layerIdx = 0; layerIdx != fighter.layerMaps.count(); ++layerIdx)
    {
        const auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;
        const auto it = labelToRows.find(label);
        if (it == labelToRows.end())
            continue;

        for (int row : it->value())
            result.push(fighter.colMotionValue[row]);
        break;
    }

    return result;
}

// ----------------------------------------------------------------------------
int MotionLabels::layerIndex(const char* layerName) const
{
    for (int i = 0; i != layerNames_.count(); ++i)
        if (layerNames_[i] == layerName)
            return i;
    return -1;
}

// ----------------------------------------------------------------------------
int MotionLabels::newLayer(const char* nameUtf8, Usage usage)
{
    const int layerIdx = layerNames_.count();

    layerNames_.push(nameUtf8);
    layerUsages_.push(usage);

    for (Fighter& fighter : fighters_)
    {
        const int rowCount = fighter.colMotionValue.count();
        Layer& layer = fighter.colLayer.emplace();
        layer.labels.resize(rowCount);

        fighter.layerMaps.emplace();
    }

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerCountChanged);

    return layerIdx;
}

// ----------------------------------------------------------------------------
void MotionLabels::deleteLayer(int layerIdx)
{
    for (Fighter& fighter : fighters_)
    {
        fighter.layerMaps.erase(layerIdx);
        fighter.colLayer.erase(layerIdx);
    }

    layerUsages_.erase(layerIdx);
    layerNames_.erase(layerIdx);

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerCountChanged);
}

// ----------------------------------------------------------------------------
void MotionLabels::renameLayer(int layerIdx, const char* newNameUtf8)
{
    layerNames_[layerIdx] = newNameUtf8;
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerNameChanged, layerIdx);
}

// ----------------------------------------------------------------------------
void MotionLabels::changeUsage(int layerIdx, Usage newUsage)
{
    layerUsages_[layerIdx] = newUsage;
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerUsageChanged, layerIdx);
}

// ----------------------------------------------------------------------------
int MotionLabels::moveLayer(int layerIdx, int insertIdx)
{
    return -1;
}

// ----------------------------------------------------------------------------
int MotionLabels::mergeLayers(int targetLayerIdx, int sourceLayerIdx)
{
    if (layerUsages_[targetLayerIdx] != layerUsages_[sourceLayerIdx])
        return -1;

    // Create new temporary "merge" layer
    const int mergeLayerIdx = layerNames_.count();
    for (Fighter& fighter : fighters_)
    {
        const int rowCount = fighter.colMotionValue.count();
        Layer& layer = fighter.colLayer.emplace();
        layer.labels.resize(rowCount);
        //fighter.layerMaps.emplace();  // no lookups are required in the merge layer
    }

    // Go through each row for each fighter and copy data over to the "merge"
    // layer. If two cells are found that contain different strings, we
    // annotate it as "x|y" in the merge layer and set the success flag to
    // false.
    bool successfulMerge = true;
    for (Fighter& fighter : fighters_)
    {
        for (int row = 0; row != fighter.colMotionValue.count(); ++row)
        {
            const auto& target = fighter.colLayer[targetLayerIdx].labels[row];
            const auto& source = fighter.colLayer[sourceLayerIdx].labels[row];
            auto& merge = fighter.colLayer[mergeLayerIdx].labels[row];
            if (source.isEmpty())
                merge = target;
            else if (target.isEmpty())
                merge = source;
            else if (source == target)
                merge = source;
            else
            {
                merge = target + "|" + source;
                successfulMerge = false;
            }
        }
    }

    if (successfulMerge == false)
    {
        // Have to build layer map for merge layer since now it becomes a "real"
        // layer.
        for (Fighter& fighter : fighters_)
        {
            auto& labelToRows = fighter.layerMaps.emplace().labelToRows;
            for (int row = 0; row != fighter.colMotionValue.count(); ++row)
            {
                const auto& label = fighter.colLayer[mergeLayerIdx].labels[row];
                if (label.notEmpty())
                    labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);
            }
        }

        // Create layer name and usage too
        layerNames_.push(layerNames_[targetLayerIdx] + "|" + layerNames_[sourceLayerIdx]);
        layerUsages_.push(layerUsages_[targetLayerIdx]);

        return mergeLayerIdx;
    }

    // Replace the target layer with the merge layer, and erase the merge layer
    for (Fighter& fighter : fighters_)
    {
        std::swap(fighter.colLayer[targetLayerIdx], fighter.colLayer[mergeLayerIdx]);
        fighter.colLayer.erase(mergeLayerIdx);

        // Rebuild layer map since it now contains new strings
        auto& labelToRows = fighter.layerMaps[targetLayerIdx].labelToRows;
        labelToRows.clear();
        for (int row = 0; row != fighter.colMotionValue.count(); ++row)
        {
            const auto& label = fighter.colLayer[targetLayerIdx].labels[row];
            if (label.notEmpty())
                labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);
        }
    }

    // Erase the source layer
    layerUsages_.erase(sourceLayerIdx);
    layerNames_.erase(sourceLayerIdx);
    for (Fighter& fighter : fighters_)
    {
        fighter.layerMaps.erase(sourceLayerIdx);
        fighter.colLayer.erase(sourceLayerIdx);
    }

    return 0;
}

// ----------------------------------------------------------------------------
bool MotionLabels::addUnknownMotion(FighterID fighterID, FighterMotion motion)
{
    return false;
}

// ----------------------------------------------------------------------------
bool MotionLabels::addNewLabel(FighterID fighterID, FighterMotion motion, Category category, int layerIdx, const char* label)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    populateMissingFighters(fighterID);

    // If this motion doesn't exist yet, we create a new entry at the end of the table
    Fighter& fighter = fighters_[fighterID.value()];
    const int rows = rowCount(fighterID);
    const int row = fighter.motionToRow.insertOrGet(motion, rows)->value();
    if (row == rows)
    {
        // Create row at end of table
        fighter.colMotionValue.push(motion);
        fighter.colCategory.push(category);
        for (Layer& layer : fighter.colLayer)
            layer.labels.emplace();

        // Store new label on the appropriate layer
        fighter.colLayer[layerIdx].labels[row] = label;

        // Create entry in layer map so label -> motion lookup works
        if (fighter.colLayer[layerIdx].labels[row].notEmpty())
        {
            auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;
            labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);
        }

        dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsNewLabel, fighterID, int(row), layerIdx);
    }
    else
    {
        // The motion exists in the table, but maybe the label is still empty.
        // If not then we fail, because the label already exists.
        if (fighter.colLayer[layerIdx].labels[row].notEmpty())
            return false;

        // Set user label for this layer
        fighter.colLayer[layerIdx].labels[row] = label;

        // Create entry in layer map so label -> motion lookup works
        if (fighter.colLayer[layerIdx].labels[row].notEmpty())
        {
            auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;
            labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);
        }

        dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLabelChanged, fighterID, int(row), layerIdx);
    }

    return true;
}

// ----------------------------------------------------------------------------
void MotionLabels::changeLabel(FighterID fighterID, int row, int layerIdx, const char* newLabel)
{
    assert(fighterID.isValid());

    Fighter& fighter = fighters_[fighterID.value()];
    auto& label = fighter.colLayer[layerIdx].labels[row];
    auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;

    auto it = labelToRows.find(label);
    if (it != labelToRows.end())
    {
        auto& matchingRows = it->value();
        matchingRows.erase(matchingRows.findFirst(row));
        if (matchingRows.count() == 0)
            labelToRows.erase(it);
    }

    label = newLabel;

    if (label.notEmpty())
        labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLabelChanged, fighterID, row, layerIdx);
}

// ----------------------------------------------------------------------------
void MotionLabels::changeCategory(FighterID fighterID, int row, Category newCategory)
{
    assert(fighterID.isValid());

    Fighter& fighter = fighters_[fighterID.value()];
    fighter.colCategory[row] = newCategory;

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsCategoryChanged, fighterID, row);
}

// ----------------------------------------------------------------------------
void MotionLabels::populateMissingFighters(FighterID fighterID)
{
    assert(fighterID.isValid());

    while (fighters_.count() < fighterID.value() + 1)
    {
        Fighter& fighter = fighters_.emplace();
        while (fighter.colLayer.count() < layerCount())
            fighter.colLayer.emplace();
        while (fighter.layerMaps.count() < layerCount())
            fighter.layerMaps.emplace();
    }
}

}
