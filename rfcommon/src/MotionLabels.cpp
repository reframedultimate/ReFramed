#include "rfcommon/Deserializer.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/LastError.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/MotionLabelsListener.hpp"
#include "rfcommon/Serializer.hpp"
#include "rfcommon/Utf8.hpp"

#include "nlohmann/json.hpp"

#include <cstdio>

namespace rfcommon {

using nlohmann::json;

// ----------------------------------------------------------------------------
MotionLabels::MotionLabels()
{}

// ----------------------------------------------------------------------------
MotionLabels::MotionLabels(const char* filePathUtf8)
    : filePath_(filePathUtf8)
{
    load();
}

// ----------------------------------------------------------------------------
MotionLabels::~MotionLabels()
{
}

// ----------------------------------------------------------------------------
bool MotionLabels::load()
{
    if (filePath_.isEmpty())
        return false;
    return load(filePath_.cStr());
}
bool MotionLabels::load(const char* filePathUtf8)
{
    Log* log = Log::root();

    MappedFile f;
    log->info("Loading motion labels binary file \"%s\"", filePathUtf8);
    if (f.open(filePathUtf8) == false)
    {
        log->error("Failed to open file \"%s\": %s", filePathUtf8, LastError().cStr());
        return false;
    }

    Deserializer d(f.address(), f.size());
    const uint8_t major = d.readU8();
    const uint8_t minor = d.readU8();
    if (major != 1 || minor != 1)
    {
        log->error("Failed to load file \"%s\": Unsupported version %d.%d", filePathUtf8, (int)major, (int)minor);
        return false;
    }

    filePath_ = filePathUtf8;

    hash40s_.clear();
    fighters_.clear();
    layers_.clear();

    // Preferred layers
    for (int i = 0; i != 3; ++i)
    {
        preferredLayers_[i] = d.readLI16();
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

    // Load all layer names, groups and usages
    const int layerCount = d.readLU16();
    for (int i = 0; i != layerCount; ++i)
    {
        int nameLen = d.readU8();
        int groupLen = d.readU8();
        Usage usage = static_cast<Usage>(d.readU8());
        const char* layerName = static_cast<const char*>(d.readFromPtr(nameLen));
        const char* groupName = static_cast<const char*>(d.readFromPtr(groupLen));
        layers_.push({
             String(layerName, nameLen),
             String(groupName, groupLen),
             usage
        });
    }

    // Load layers
    const int fighterCount = d.readU8();
    for (int fighterIdx = 0; fighterIdx != fighterCount; ++fighterIdx)
    {
        const int rowCount = d.readLU16();
        Fighter& fighter = fighters_.emplace();
        fighter.colLayer.resize(layerCount);
        fighter.layerMaps.resize(layerCount);

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
                auto& label = fighter.colLayer[layerIdx].labels.emplace(static_cast<const char*>(d.readFromPtr(len)), len);
                if (label.notEmpty())
                {
                    auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;
                    labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);
                }
            }

            if (motion.isValid() == false)
                log->warning("Found invalid motion value in file");
            if (fighter.motionToRow.insertIfNew(motion, row) == fighter.motionToRow.end())
                log->warning("Found duplicate motion value 0x%lx in file", motion.value());
        }
    }

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLoaded);

    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabels::save() const
{
    if (filePath_.isEmpty())
        return false;
    return save(filePath_.cStr());
}
bool MotionLabels::save(const char* filePathUtf8) const
{
    Log* log = Log::root();

    log->info("Saving motion labels binary file \"%s\"", filePathUtf8);
    FILE* fp = utf8_fopen_wb(filePathUtf8, (int)strlen(filePathUtf8));
    if (fp == nullptr)
    {
        log->error("Failed to open file \"%s\": %s", filePathUtf8, LastError().cStr());
        return false;
    }

    char scratch[8];

    // Write file version
    {
        Serializer s(scratch, sizeof scratch);
        s.writeU8(1);  // major
        s.writeU8(1);  // minor
        fwrite(s.data(), s.bytesWritten(), 1, fp);
    }

    // Preferred layers
    for (int i = 0; i != 3; ++i)
    {
        Serializer s(scratch, sizeof scratch);
        s.writeLI16(preferredLayers_[i]);
        fwrite(s.data(), s.bytesWritten(), 1, fp);
    }

    // Save hash40 <-> string map
    {
        Serializer s(scratch, sizeof scratch);
        s.writeLU32(hash40s_.count());
        fwrite(s.data(), s.bytesWritten(), 1, fp);
    }
    for (auto it : hash40s_)
    {
        Serializer s(scratch, sizeof scratch);
        s.writeLU32(it.key().lower());
        s.writeU8(it.key().upper());
        s.writeU8(it.value().length());
        fwrite(s.data(), s.bytesWritten(), 1, fp);
        fwrite(it.value().cStr(), it.value().length(), 1, fp);
    }

    // Save layer names and usages
    {
        Serializer s1(scratch, sizeof scratch);
        s1.writeLU16(layers_.count());
        fwrite(s1.data(), s1.bytesWritten(), 1, fp);
        for (const Layer& layer : layers_)
        {
            Serializer s2(scratch, sizeof scratch);
            s2.writeU8(layer.name.length());
            s2.writeU8(layer.group.length());
            s2.writeU8(static_cast<uint8_t>(layer.usage));
            fwrite(s2.data(), s2.bytesWritten(), 1, fp);
            fwrite(layer.name.cStr(), layer.name.length(), 1, fp);
            fwrite(layer.group.cStr(), layer.group.length(), 1, fp);
        }
    }

    // Save fighter layers
    {
        // Write number of fighters and number of layers
        Serializer s(scratch, sizeof scratch);
        s.writeU8((uint8_t)fighters_.count());
        fwrite(s.data(), s.bytesWritten(), 1, fp);
    }
    for (const Fighter& fighter : fighters_)
    {
        // Write number of rows in table for this fighter. The number of
        // columns remains constant for all fighters
        Serializer s1(scratch, sizeof scratch);
        s1.writeLU16(fighter.colMotionValue.count());
        fwrite(s1.data(), s1.bytesWritten(), 1, fp);

        for (int row = 0; row != fighter.colMotionValue.count(); ++row)
        {
            Serializer s2(scratch, sizeof scratch);
            s2.writeLU32(fighter.colMotionValue[row].lower());
            s2.writeU8(fighter.colMotionValue[row].upper());
            s2.writeU8(fighter.colCategory[row]);
            fwrite(s2.data(), s2.bytesWritten(), 1, fp);

            for (const LayerColumn& layer : fighter.colLayer)
            {
                Serializer s3(scratch, sizeof scratch);
                s3.writeI8(layer.labels[row].length());
                fwrite(s3.data(), s3.bytesWritten(), 1, fp);
                fwrite(layer.labels[row].cStr(), layer.labels[row].length(), 1, fp);
            }
        }
    }

    fclose(fp);

    return true;
}

// ----------------------------------------------------------------------------
int MotionLabels::updateHash40FromCSV(const char* filePathUtf8)
{
    Log* log = Log::root();

    MappedFile f;
    log->info("Updating hash40 strings from CSV file \"%s\"", filePathUtf8);
    if (f.open(filePathUtf8) == false)
    {
        log->error("Failed to open file \"%s\": %s", filePathUtf8, LastError().cStr());
        return -1;
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
            return -1;
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
        for (; *p != '\n' && p != pe; ++p) {}
        if (p == pe || ++p == pe)
            break;
    }

    log->info("Updated %d hash40 labels\n", numInserted);
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsHash40sUpdated);

    return numInserted;
}

// ----------------------------------------------------------------------------
int MotionLabels::importLayers(const char* filePathUtf8)
{
    Log* log = Log::root();

    MappedFile file;
    log->info("Importing layer from file \"%s\"", filePathUtf8);
    if (file.open(filePathUtf8) == false)
    {
        log->error("Failed to map file \"%s\": %s", filePathUtf8, LastError().cStr());
        return -1;
    }

    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(file.address());
    const unsigned char* const end = static_cast<const unsigned char*>(file.address()) + file.size();
    json j = json::parse(begin, end, nullptr, false);
    if (j.is_discarded())
    {
        log->error("Failed to parse json file: \"%s\"", filePathUtf8);
        return -1;
    }

    const int layerCountBeforeImport = layerCount();

    if (j["version"] == "1.0" && j["name"] == "Unlabeled")
    {
        json jFighters = j["fighters"];
        for (const auto& [fighterIDStr, jFighter] : jFighters.items())
        {
            std::size_t pos;
            const auto fighterID = FighterID::fromValue((FighterID::Type)std::stoul(fighterIDStr, &pos));
            if (pos != fighterIDStr.length())
                continue;

            json jMotions = jFighter["motions"];
            if (jMotions.is_array() == false)
                continue;

            for (int i = 0; i != (int)jMotions.size(); ++i)
            {
                if (jMotions[i].is_number_integer() == false)
                    continue;

                const auto motion = FighterMotion::fromValue(jMotions[i].get<FighterMotion::Type>());
                addUnknownMotionNoNotify(fighterID, motion);
            }
        }

        dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLoaded);
    }
    else if (j["version"] == "1.0")
    {
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

        const int layerIdx = newLayerNoNotify(
            jName.get<std::string>().c_str(),
            jName.get<std::string>().c_str(),
            usage
        );

        json jFighters = j["fighters"];
        for (const auto& [fighterIDStr, jFighter] : jFighters.items())
        {
            std::size_t pos;
            const auto fighterID = FighterID::fromValue((FighterID::Type)std::stoul(fighterIDStr, &pos));
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

                addNewLabelNoNotify(fighterID, motion, category, layerIdx, label.c_str());
            }
        }

        dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerInserted, int(layerIdx));
    }
    else if (j["version"] == "1.1")
    {
        json jLayers = j["layers"];
        if (jLayers.is_array() == false)
        {
            log->error("Property \"layers\" is not an array");
            return -1;
        }

        for (const auto& jLayer : jLayers)
        {
            json jName = jLayer["name"];
            if (jName.is_string() == false)
            {
                log->notice("Missing property \"name\", setting default name");
                jName = "Layer";
            }

            json jGroup = jLayer["group"];
            if (jGroup.is_string() == false)
            {
                log->notice("Missing property \"group\", setting to same as name");
                jGroup = jName;
            }

            json jUsage = jLayer["usage"];
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
                if (jUsage == "categorization")
                    return Usage::CATEGORIZATION;

                log->notice("Usage \"%s\" is invalid. Using default.", jUsage.get<std::string>().c_str());
                return Usage::NOTATION;
            }();

            const int layerIdx = newLayerNoNotify(
                jName.get<std::string>().c_str(),
                jGroup.get<std::string>().c_str(),
                usage
            );

            json jFighters = jLayer["fighters"];
            for (const auto& [fighterIDStr, jFighter] : jFighters.items())
            {
                std::size_t pos;
                const auto fighterID = FighterID::fromValue((FighterID::Type)std::stoul(fighterIDStr, &pos));
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

                    addNewLabelNoNotify(fighterID, motion, category, layerIdx, label.c_str());
                }
            }

            dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerInserted, int(layerIdx));
        }
    }
    else
    {
        log->error("Unsupported version \"%s\" while parsing file \"%s\"", j["version"].get<std::string>().c_str(), filePathUtf8);
        return -1;
    }

    const int layerCountAfterImport = layerCount();
    return layerCountAfterImport - layerCountBeforeImport;
}

// ----------------------------------------------------------------------------
bool MotionLabels::exportLayers(SmallVector<int, 4> layers, const char* filePathUtf8) const
{
    Log* log = Log::root();

    json jLayers = json::array();
    for (int layerIdx : layers)
    {
        json jFighters;
        for (int fighterIdx = 0; fighterIdx != fighters_.count(); ++fighterIdx)
        {
            const Fighter& fighter = fighters_[fighterIdx];
            const LayerColumn& layer = fighter.colLayer[layerIdx];

            json jMotions = json::array();
            json jLabels = json::array();
            json jCategories = json::array();
            for (int row = 0; row != fighter.colMotionValue.count(); ++row)
            {
                if (layer.labels[row].isEmpty())
                    continue;
                jMotions.push_back(fighter.colMotionValue[row].value());
                jLabels.push_back(layer.labels[row].cStr());
                jCategories.push_back(fighter.colCategory[row]);
            }

            if (jLabels.size() == 0)
                continue;

            jFighters[std::to_string(fighterIdx)] = {
                {"motions", jMotions},
                {"labels", jLabels},
                {"categories", jCategories}
            };
        }

        if (jFighters.size() == 0)
            continue;

        auto usageToStr = [](Usage usage) {
            switch (usage)
            {
                case Usage::READABLE: return "readable";
                case Usage::NOTATION: return "notation";
                case Usage::CATEGORIZATION: return "categorization";
            }
            return "notation";
        };

        jLayers.push_back({
            {"name", layers_[layerIdx].name.cStr()},
            {"group", layers_[layerIdx].group.cStr()},
            {"usage", usageToStr(layers_[layerIdx].usage)},
            {"fighters", jFighters}
        });
    }

    json j = {
        {"version", "1.1"},
        {"layers", jLayers}
    };

    const std::string jsonAsString = j.dump();
    FILE* fp = utf8_fopen_wb(filePathUtf8, (int)strlen(filePathUtf8));
    if (fp == nullptr)
    {
        log->error("Failed to open file \"%s\": %s", filePathUtf8, LastError().cStr());
        return false;
    }

    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
    {
        log->error("Failed to write data to file \"%s\": %s", filePathUtf8, LastError().cStr());
        return false;
    }

    fclose(fp);
    log->info("Exported layers to file \"%s\"", filePathUtf8);
    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabels::exportLayers(SmallVector<int, 4> layers, FighterID fighterID, const char* fileNameUtf8) const
{
    Log* log = Log::root();

    json jLayers = json::array();
    for (int layerIdx : layers)
    {
        json jFighters;

        const Fighter& fighter = fighters_[fighterID.value()];
        const LayerColumn& layer = fighter.colLayer[layerIdx];

        json jMotions = json::array();
        json jLabels = json::array();
        json jCategories = json::array();
        for (int row = 0; row != fighter.colMotionValue.count(); ++row)
        {
            if (layer.labels[row].isEmpty())
                continue;
            jMotions.push_back(fighter.colMotionValue[row].value());
            jLabels.push_back(layer.labels[row].cStr());
            jCategories.push_back(fighter.colCategory[row]);
        }

        if (jLabels.size() == 0)
            continue;

        jFighters[std::to_string(fighterID.value())] = {
            {"motions", jMotions},
            {"labels", jLabels},
            {"categories", jCategories}
        };

        auto usageToStr = [](Usage usage) {
            switch (usage)
            {
                case Usage::READABLE: return "readable";
                case Usage::NOTATION: return "notation";
                case Usage::CATEGORIZATION: return "categorization";
            }
            return "notation";
        };

        jLayers.push_back({
            {"name", layers_[layerIdx].name.cStr()},
            {"usage", usageToStr(layers_[layerIdx].usage)},
            {"fighters", {
                 {std::to_string(fighterID.value()), {
                      {"motions", jMotions},
                      {"labels", jLabels},
                      {"categories", jCategories}
                 }}
            }}
        });
    }

    json j = {
        {"version", "1.1"},
        {"layers", jLayers}
    };

    const std::string jsonAsString = j.dump();
    FILE* fp = utf8_fopen_wb(fileNameUtf8, (int)strlen(fileNameUtf8));
    if (fp == nullptr)
    {
        log->error("Failed to open file \"%s\": %s", fileNameUtf8, LastError().cStr());
        return false;
    }

    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
    {
        log->error("Failed to write data to file \"%s\": %s", fileNameUtf8, LastError().cStr());
        return false;
    }

    fclose(fp);
    log->info("Exported layers to file \"%s\"", fileNameUtf8);
    return true;
}

// ----------------------------------------------------------------------------
void MotionLabels::setPreferredLayer(Usage usage, int layerIdx)
{
    preferredLayers_[usage] = layerIdx;
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsPreferredLayerChanged, usage);
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toPreferredNotation(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion, const char* fallback) const
{
    return toGroupLabel(fighterID, motion, preferredLayers_[NOTATION], fallback);
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toPreferredReadable(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion, const char* fallback) const
{
    return toGroupLabel(fighterID, motion, preferredLayers_[READABLE], fallback);
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toPreferredCategory(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion, const char* fallback) const
{
    return toGroupLabel(fighterID, motion, preferredLayers_[CATEGORIZATION], fallback);
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toHash40(FighterMotion motion, const char* fallback) const
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
int MotionLabels::toRow(FighterID fighterID, FighterMotion motion) const
{
    if (fighterID.value() >= fighters_.count())
        return -1;

    const Fighter& fighter = fighters_[fighterID.value()];
    const auto it = fighter.motionToRow.find(motion);
    if (it == fighter.motionToRow.end())
        return -1;
    return it->value();
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toLabel(FighterID fighterID, FighterMotion motion, int layerIdx, const char* fallback) const
{
    if (layerIdx < 0 || fighterID.value() >= fighters_.count())
        return fallback;

    // Look up row for specified fighter
    const Fighter& fighter = fighters_[fighterID.value()];
    const auto it = fighter.motionToRow.find(motion);
    if (it == fighter.motionToRow.end())
        return fallback;
    const int row = it->value();

    // Cell could contain an empty string
    const LayerColumn& layer = fighter.colLayer[layerIdx];
    if (layer.labels[row].isEmpty())
        return fallback;

    return layer.labels[row].cStr();
}

// ----------------------------------------------------------------------------
const char* MotionLabels::toGroupLabel(FighterID fighterID, FighterMotion motion, int startLayerIdx, const char* fallback) const
{
    if (startLayerIdx < 0 || fighterID.value() >= fighters_.count())
        return fallback;

    // Look up row for specified fighter
    const Fighter& fighter = fighters_[fighterID.value()];
    const auto it = fighter.motionToRow.find(motion);
    if (it == fighter.motionToRow.end())
        return fallback;
    const int row = it->value();

    if (fighter.colLayer[startLayerIdx].labels[row].notEmpty())
        return fighter.colLayer[startLayerIdx].labels[row].cStr();

    const rfcommon::String& groupName = layers_[startLayerIdx].group;
    for (int layerIdx = 0; layerIdx != layers_.count(); ++layerIdx)
    {
        if (layers_[layerIdx].group != groupName)
            continue;

        const LayerColumn& layer = fighter.colLayer[layerIdx];
        if (layer.labels[row].notEmpty())
            return layer.labels[row].cStr();
    }

    return fallback;
}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> MotionLabels::toMotions(FighterID fighterID, const char* label) const
{
    SmallVector<FighterMotion, 4> result;

    if (fighterID.value() >= fighters_.count())
        return result;

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
int MotionLabels::findLayer(const char* groupName, const char* layerName) const
{
    for (int i = 0; i != layers_.count(); ++i)
        if (layers_[i].name == layerName && layers_[i].group == groupName)
            return i;
    return -1;
}

// ----------------------------------------------------------------------------
int MotionLabels::findGroup(const char* groupName) const
{
    for (int i = 0; i != layers_.count(); ++i)
        if (layers_[i].group == groupName)
            return i;
    return -1;
}

// ----------------------------------------------------------------------------
int MotionLabels::newLayer(const char* layerName, const char* groupName, Usage usage)
{
    int layerIdx = newLayerNoNotify(layerName, layerName, usage);
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerInserted, layerIdx);
    return layerIdx;
}
int MotionLabels::newLayerNoNotify(const char* layerName, const char* groupName, Usage usage)
{
    const int layerIdx = layers_.count();

    layers_.push({
         layerName,
         groupName,
         usage
     });

    for (Fighter& fighter : fighters_)
    {
        const int rowCount = fighter.colMotionValue.count();
        LayerColumn& layer = fighter.colLayer.emplace();
        layer.labels.resize(rowCount);

        fighter.layerMaps.emplace();
    }

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

    layers_.erase(layerIdx);

    // Make sure preferred layer isn't pointing to an invalid index
    for (int i = 0; i != 3; ++i)
        if (preferredLayers_[i] == layerIdx)
            preferredLayers_[i] = -1;

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerRemoved, layerIdx);
}

// ----------------------------------------------------------------------------
void MotionLabels::deleteLayers(rfcommon::SmallVector<int, 4> layerIdxs)
{
    // Delete in descending order, otherwise the next index will be invalidated
    std::sort(layerIdxs.begin(), layerIdxs.end(), std::greater<int>());
    for (int layerIdx : layerIdxs)
        deleteLayer(layerIdx);
}

// ----------------------------------------------------------------------------
void MotionLabels::renameLayer(int layerIdx, const char* newNameUtf8)
{
    layers_[layerIdx].name = newNameUtf8;
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerNameChanged, layerIdx);
}

// ----------------------------------------------------------------------------
void MotionLabels::changeGroup(int layerIdx, const char* newGroupUtf8)
{
    layers_[layerIdx].group = newGroupUtf8;
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerNameChanged, layerIdx);
}

// ----------------------------------------------------------------------------
void MotionLabels::changeUsage(int layerIdx, Usage newUsage)
{
    Usage oldUsage = layers_[layerIdx].usage;
    layers_[layerIdx].usage = newUsage;
    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerUsageChanged, layerIdx, oldUsage);
}

// ----------------------------------------------------------------------------
int MotionLabels::moveLayer(int layerIdx, int insertIdx)
{
    return -1;
}

// ----------------------------------------------------------------------------
int MotionLabels::mergeLayers(int targetLayerIdx, int sourceLayerIdx)
{
    if (layers_[targetLayerIdx].usage != layers_[sourceLayerIdx].usage)
        return -1;

    // Create new temporary "merge" layer
    const int mergeLayerIdx = layers_.count();
    for (Fighter& fighter : fighters_)
    {
        const int rowCount = fighter.colMotionValue.count();
        LayerColumn& layer = fighter.colLayer.emplace();
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
        layers_.push({
            layers_[targetLayerIdx].name + "|" + layers_[sourceLayerIdx].name,
            layers_[targetLayerIdx].name + "|" + layers_[sourceLayerIdx].name,
            layers_[targetLayerIdx].usage
        });

        dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerInserted, int(mergeLayerIdx));

        return mergeLayerIdx;
    }

    // Replace the target layer with the merge layer
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

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLayerMerged, targetLayerIdx);

    // Erase the source layer
    deleteLayer(sourceLayerIdx);

    return 0;
}

// ----------------------------------------------------------------------------
bool MotionLabels::isLayerEmpty(int layerIdx) const
{
    for (const Fighter& fighter : fighters_)
        if (fighter.layerMaps[layerIdx].labelToRows.count() > 0)
            return false;
    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabels::addUnknownMotion(FighterID fighterID, FighterMotion motion)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    if (addUnknownMotionNoNotify(fighterID, motion) == false)
        return false;

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsRowInserted, fighterID, rowCount(fighterID) - 1);
    return true;
}
bool MotionLabels::addUnknownMotionNoNotify(FighterID fighterID, FighterMotion motion)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    populateMissingFighters(fighterID);

    Fighter& fighter = fighters_[fighterID.value()];
    if (fighter.motionToRow.insertIfNew(motion, rowCount(fighterID)) == fighter.motionToRow.end())
        return false;

    fighter.colCategory.push(Category::UNLABELED);
    fighter.colMotionValue.push(motion);
    for (LayerColumn& layer : fighter.colLayer)
        layer.labels.emplace();

    return true;
}

// ----------------------------------------------------------------------------
int MotionLabels::addNewLabel(FighterID fighterID, FighterMotion motion, Category category, int layerIdx, const char* label)
{
    const int rows = rowCount(fighterID);
    int row = addNewLabelNoNotify(fighterID, motion, category, layerIdx, label);
    if (row < 0)
        return row;
    if (row == rows)
        dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsRowInserted, fighterID, row);
    else
        dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsLabelChanged, fighterID, row, layerIdx);
    return row;
}
int MotionLabels::addNewLabelNoNotify(FighterID fighterID, FighterMotion motion, Category category, int layerIdx, const char* label)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    populateMissingFighters(fighterID);

    // If this motion doesn't exist yet, we create a new row at the end of the table
    Fighter& fighter = fighters_[fighterID.value()];
    const int row = fighter.motionToRow.insertOrGet(motion, rowCount(fighterID))->value();
    if (row == rowCount(fighterID))
    {
        // Create row at end of table
        fighter.colMotionValue.push(motion);
        fighter.colCategory.push(category);
        for (LayerColumn& layer : fighter.colLayer)
            layer.labels.emplace();
    }
    else
    {
        // The motion exists in the table, but maybe the label is still empty.
        // If not then we fail, because the label already exists.
        if (fighter.colLayer[layerIdx].labels[row].notEmpty())
            return -1;
    }

    // Set user label for this layer
    fighter.colLayer[layerIdx].labels[row] = label;
    fighter.colCategory[row] = category;

    // Create entry in layer map so label -> motion lookup works
    if (fighter.colLayer[layerIdx].labels[row].notEmpty())
    {
        auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;
        labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);
    }

    return row;
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
    Category oldCategory = fighter.colCategory[row];
    fighter.colCategory[row] = newCategory;

    dispatcher.dispatch(&MotionLabelsListener::onMotionLabelsCategoryChanged, fighterID, row, oldCategory);
}

// ----------------------------------------------------------------------------
int MotionLabels::propagateLabel(FighterID fighterID, int row, int layerIdx, bool replaceExisting, bool forceCreation)
{
    const FighterMotion motion = fighters_[fighterID.value()].colMotionValue[row];
    const auto& label = fighters_[fighterID.value()].colLayer[layerIdx].labels[row];
    const Category category = fighters_[fighterID.value()].colCategory[row];

    int updated = 0;

    for (int fighterIdx = 0; fighterIdx != fighters_.count(); ++fighterIdx)
    {
        Fighter& fighter = fighters_[fighterIdx];

        auto it = fighter.motionToRow.find(motion);
        if (it == fighter.motionToRow.end())
        {
            if (forceCreation == false)
                continue;

            // Create row at end of table and insert label
            row = rowCount(FighterID::fromValue(fighterIdx));
            fighter.colMotionValue.push(motion);
            fighter.colCategory.push(category);
            for (LayerColumn& layer : fighter.colLayer)
                layer.labels.emplace();
            fighter.colLayer[layerIdx].labels.back() = label;

            // Update motion -> row map
            fighter.motionToRow.insertAlways(motion, row);

            // Create entry in layer map so label -> motion lookup works
            if (label.notEmpty())
            {
                auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;
                labelToRows.insertOrGet(label, SmallVector<int, 4>())->value().push(row);
            }
        }
        else
        {
            row = it->value();
            auto& currentLabel = fighter.colLayer[layerIdx].labels[row];

            if (currentLabel.isEmpty() == false)
                if (replaceExisting == false)
                    continue;

            // Remove current entry in layer map
            auto& labelToRows = fighter.layerMaps[layerIdx].labelToRows;
            auto it2 = labelToRows.find(currentLabel);
            if (it2 != labelToRows.end())
            {
                auto& matchingRows = it2->value();
                matchingRows.erase(matchingRows.findFirst(row));
                if (matchingRows.count() == 0)
                    labelToRows.erase(it2);
            }

            currentLabel = label;

            // Create entry in layer map so label -> motion lookup works
            if (currentLabel.notEmpty())
                labelToRows.insertOrGet(currentLabel, SmallVector<int, 4>())->value().push(row);
        }

        updated++;
    }

    return updated;
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
