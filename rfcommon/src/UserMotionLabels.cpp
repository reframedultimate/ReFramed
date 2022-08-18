#include "rfcommon/UserMotionLabels.hpp"
#include "rfcommon/UserMotionLabelsListener.hpp"

#include "nlohmann/json.hpp"

namespace rfcommon {

using nlohmann::json;

// ----------------------------------------------------------------------------
FighterUserMotionLabels::FighterUserMotionLabels()
{}

// ----------------------------------------------------------------------------
FighterUserMotionLabels::~FighterUserMotionLabels()
{}

// ----------------------------------------------------------------------------
UserMotionLabels::UserMotionLabels()
{}

// ----------------------------------------------------------------------------
UserMotionLabels::~UserMotionLabels()
{}

// ----------------------------------------------------------------------------
bool UserMotionLabels::loadLayer(const void* address, uint32_t size)
{
    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(address);
    const unsigned char* const end = static_cast<const unsigned char*>(address) + size;
    json j = json::parse(begin, end, nullptr, false);
    if (j == json::value_t::discarded)
        return false;

    if (j["version"] != "1.0")
        return false;

    json jName = j["name"];
    if (jName.is_string() == false)
        return false;

    const int layerIdx = newEmptyLayer(jName.get<std::string>().c_str());

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
bool UserMotionLabels::loadUnlabeled(const void* address, uint32_t size)
{
    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(address);
    const unsigned char* const end = static_cast<const unsigned char*>(address) + size;
    json j = json::parse(begin, end, nullptr, false);
    if (j == json::value_t::discarded)
        return false;

    if (j["version"] != "1.0")
        return false;

    json jName = j["name"];
    if (jName.is_string() == false)
        return false;

    if (jName.get<std::string>() != "Unlabeled")
        return false;

    json jFighters = j["fighters"];
    for (const auto& [fighterIDStr, jFighter] : jFighters.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(fighterIDStr, &pos));
        if (pos != fighterIDStr.length())
            continue;

        json jMotions = jFighter["motions"];
        if (jMotions.is_array() == false)
            continue;

        for (int i = 0; i != jMotions.size(); ++i)
        {
            if (jMotions[i].is_number_integer() == false)
                continue;

            const auto motion = FighterMotion::fromValue(jMotions[i].get<FighterMotion::Type>());
            addUnknownMotion(fighterID, motion);
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
uint32_t UserMotionLabels::saveLayer(FILE* fp, const int layerIdx) const
{
    json jFighters = json::object();
    for (int fighterIdx = 0; fighterIdx != fighters_.count(); ++fighterIdx)
    {
        const auto fighterID = FighterID::fromValue(fighterIdx);

        json jMotions = json::array();
        json jLabels = json::array();
        json jCategories = json::array();
        for (int entryIdx = 0; entryIdx != entryCount(fighterID); ++entryIdx)
        {
            const char* label = userLabelAt(fighterID, layerIdx, entryIdx);
            if (strlen(label) == 0)  // Skip any unlabelled entries
                continue;
            const auto motion = motionAt(fighterID, entryIdx);
            const auto category = categoryAt(fighterID, entryIdx);

            jMotions += motion.value();
            jLabels += label;
            jCategories += static_cast<int>(category);
        }

        if (jMotions.size() > 0)
        {
            jFighters[std::to_string(fighterID.value())] = {
                {"motions", jMotions},
                {"labels", jLabels},
                {"categories", jCategories}
            };
        }
    }

    json j = {
        {"version", "1.0"},
        {"name", layerName(layerIdx)},
        {"fighters", jFighters}
    };

    const std::string jsonAsString = j.dump();
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
        return 0;

    return jsonAsString.length();
}

// ----------------------------------------------------------------------------
uint32_t UserMotionLabels::saveUnlabeled(FILE* fp) const
{
    json jFighters = json::object();
    for (int fighterIdx = 0; fighterIdx != fighters_.count(); ++fighterIdx)
    {
        const auto fighterID = FighterID::fromValue(fighterIdx);
        if (entryCount(fighterID) == 0)  // Empty table
            continue;

        json jMotions = json::array();
        for (int entryIdx = 0; entryIdx != entryCount(fighterID); ++entryIdx)
        {
            for (int layerIdx = 0; layerIdx != layerCount(); ++layerIdx)
            {
                const char* label = userLabelAt(fighterID, layerIdx, entryIdx);
                if (strlen(label) > 0)  // This entry has a label, so skip
                    continue;
            }

            const auto motion = motionAt(fighterID, entryIdx);
            jMotions += motion.value();
        }

        jFighters[std::to_string(fighterID.value())] = {
            {"motions", jMotions}
        };
    }

    json j = {
        {"version", "1.0"},
        {"name", "Unlabeled"},
        {"fighters", jFighters}
    };

    const std::string jsonAsString = j.dump();
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
        return 0;

    return jsonAsString.length();
}

// ----------------------------------------------------------------------------
int UserMotionLabels::newEmptyLayer(const char* name)
{
    const int layerIdx = layerNames_.count();
    layerNames_.push(name);

    for (int fighterIdx = 0; fighterIdx != fighters_.count(); ++fighterIdx)
    {
        fighters_[fighterIdx].layers.emplace();
        fighters_[fighterIdx].layerMaps.emplace();

        const int fighterEntryCount = entryCount(rfcommon::FighterID::fromValue(fighterIdx));
        fighters_[fighterIdx].layers.back().userLabels.resize(fighterEntryCount);
    }

    dispatcher.dispatch(&UserMotionLabelsListener::onUserMotionLabelsLayerAdded, int(layerIdx), name);
    return layerIdx;
}

// ----------------------------------------------------------------------------
void UserMotionLabels::removeLayer(int layerIdx)
{
    // Have to remove all entries in the hashmaps before removing the layer
    for (auto& fighter : fighters_)
        for (const auto motion : fighter.motions)
        {
            // This must exist or something is wrong
            auto motionIt = fighter.motionMap.find(motion);
            assert(motionIt != fighter.motionMap.end());
            fighter.motionMap.erase(motionIt);
        }

    for (auto& fighter : fighters_)
    {
        fighter.layerMaps.erase(layerIdx);
        fighter.layers.erase(layerIdx);
    }

    String name = layerNames_.take(layerIdx);
    dispatcher.dispatch(&UserMotionLabelsListener::onUserMotionLabelsLayerRemoved, layerIdx, name.cStr());
}

// ----------------------------------------------------------------------------
bool UserMotionLabels::addUnknownMotion(FighterID fighterID, FighterMotion motion)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    expandTablesUpTo(fighterID);

    auto& motions = fighters_[fighterID.value()].motions;
    auto& categories = fighters_[fighterID.value()].categories;
    auto& layers = fighters_[fighterID.value()].layers;

    auto& map = fighters_[fighterID.value()].motionMap;
    if (map.insertIfNew(motion, entryCount(fighterID)) == map.end())
        return false;

    motions.push(motion);
    categories.push(UserMotionLabelsCategory::UNLABELED);
    for (auto& layer : layers)
        layer.userLabels.emplace();

    dispatcher.dispatch(&UserMotionLabelsListener::onUserMotionLabelsNewEntry, fighterID, motions.count() - 1);
    return true;
}

// ----------------------------------------------------------------------------
bool UserMotionLabels::addEntry(
        FighterID fighterID, 
        int layerIdx,
        FighterMotion motion, 
        const char* userLabel, 
        UserMotionLabelsCategory category)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    expandTablesUpTo(fighterID);

    auto& motions = fighters_[fighterID.value()].motions;
    auto& categories = fighters_[fighterID.value()].categories;
    auto& layers = fighters_[fighterID.value()].layers;

    // If the key doesn't exist, we create a new entry at the end of the table
    auto& motionMap = fighters_[fighterID.value()].motionMap;
    const int entryIdx = motionMap.insertOrGet(motion, entryCount(fighterID))->value();
    if (entryIdx == entryCount(fighterID))
    {
        // Add entry to end of table. Map above stored entryCount() as the value
        motions.push(motion);
        categories.push(category);
        for (auto& layer : layers)
            layer.userLabels.emplace();
        layers[layerIdx].userLabels[entryIdx] = userLabel;

        // Create entry in user map so user label -> motion lookup works
        if (strlen(userLabel) > 0)
        {
            auto& userMap = fighters_[fighterID.value()].layerMaps[layerIdx].userMap;
            auto& entryIndices = userMap.insertOrGet(userLabel, SmallVector<int, 4>())->value();
            entryIndices.push(entryIdx);
        }

        dispatcher.dispatch(&UserMotionLabelsListener::onUserMotionLabelsNewEntry, fighterID, int(entryIdx));
    }
    else
    {
        // The table entry exists, but maybe the layer entry is still empty. If
        // not then we fail, because the label already exists.
        if (layers[layerIdx].userLabels[entryIdx].length() > 0)
            return false;

        // Set user label for this layer
        const String oldLabel = layers[layerIdx].userLabels[entryIdx];
        layers[layerIdx].userLabels[entryIdx] = userLabel;

        // Create entry in user map so user label -> motion lookup works
        if (strlen(userLabel) > 0)
        {
            auto& userMap = fighters_[fighterID.value()].layerMaps[layerIdx].userMap;
            auto& entryIndices = userMap.insertOrGet(userLabel, SmallVector<int, 4>())->value();
            entryIndices.push(entryIdx);

            dispatcher.dispatch(&UserMotionLabelsListener::onUserMotionLabelsUserLabelChanged, fighterID, int(entryIdx), "", userLabel);
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
bool UserMotionLabels::changeUserLabel(
        FighterID fighterID, 
        int layerIdx, 
        FighterMotion motion, 
        const char* newUserLabel)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    // Look up entry index using motion map
    auto& motionMap = fighters_[fighterID.value()].motionMap;
    const int entryIdx = motionMap.find(motion, entryCount(fighterID));
    if (entryIdx == entryCount(fighterID))
        return false;

    // Modify user map with new key so label -> motion lookup works
    // NOTE: We allow labels to be empty strings, in which case there won't
    // be an entry in the user map. If this is the case, then there's nothing
    // to remove
    auto& layers = fighters_[fighterID.value()].layers;
    const String oldUserLabel = layers[layerIdx].userLabels[entryIdx];
    auto& userMap = fighters_[fighterID.value()].layerMaps[layerIdx].userMap;
    auto oldEntryIndicesIt = userMap.find(oldUserLabel);
    if (oldEntryIndicesIt != userMap.end())
    {
        auto& oldEntryIndices = oldEntryIndicesIt->value();

        // User labels can map to multiple indices in the table. This list
        // contains all of these indices. Since the label is being changed,
        // we have to remove just that one entry that maps to the currenty
        // entry index
        oldEntryIndices.erase(oldEntryIndices.findFirst(entryIdx));
        if (oldEntryIndices.count() == 0)
            userMap.erase(oldEntryIndicesIt);
    }

    // Now, create a new entry in the user map
    // NOTE: We allow labels to be empty strings. In this case, we don't create
    // an entry in the user map
    if (strlen(newUserLabel) > 0)
    {
        auto& newEntryIndices = userMap.insertOrGet(newUserLabel, SmallVector<int, 4>())->value();
        newEntryIndices.push(entryIdx);
    }

    // Modify label
    layers[layerIdx].userLabels[entryIdx] = newUserLabel;

    if (oldUserLabel != newUserLabel)
        dispatcher.dispatch(&UserMotionLabelsListener::onUserMotionLabelsUserLabelChanged, fighterID, int(entryIdx), oldUserLabel.cStr(), newUserLabel);

    return true;
}

// ----------------------------------------------------------------------------
bool UserMotionLabels::changeCategory(
    FighterID fighterID,
    FighterMotion motion,
    UserMotionLabelsCategory newCategory)
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    // Look up entry index using motion map
    auto& motionMap = fighters_[fighterID.value()].motionMap;
    const int entryIdx = motionMap.find(motion, entryCount(fighterID));
    if (entryIdx == entryCount(fighterID))
        return false;

    // Modify category in table
    auto& categories = fighters_[fighterID.value()].categories;
    UserMotionLabelsCategory oldCategory = categories[entryIdx];
    categories[entryIdx] = newCategory;

    if (oldCategory != newCategory)
        dispatcher.dispatch(&UserMotionLabelsListener::onUserMotionLabelsCategoryChanged, fighterID, int(entryIdx), oldCategory, newCategory);

    return true;
}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> UserMotionLabels::toMotion(FighterID fighterID, const char* userLabel) const
{
    assert(fighterID.isValid());

    // A user label can map to multiple motion values. Additionally, there
    // can be multiple layers of user labels. What makes the most sense is
    // to prioritize layers with higher indices (layers that were added later), 
    // and only if we don't find anything in those layers, we search in the next 
    // layer down for a match.
    SmallVector<FighterMotion, 4> motions;

    const auto& fighter = fighters_[fighterID.value()];
    for (int layerIdx = layerCount() - 1; layerIdx >= 0; --layerIdx)
    {
        const auto& layer = fighter.layerMaps[layerIdx];
        const auto it = layer.userMap.find(userLabel);
        if (it != layer.userMap.end())  // map user label to vetor of indices into the table
            for (int i : it->value())  // for each index in the table, retrieve motion
                motions.push(fighter.motions[i]);
        if (motions.count() > 0)  // Don't proceed to next layer if we found matches
            break;
    }
    return motions;
}

// ----------------------------------------------------------------------------
const char* UserMotionLabels::toUserLabel(FighterID fighterID, FighterMotion motion) const
{
    return toUserLabel(fighterID, motion, "(unlabeled)");
}

// ----------------------------------------------------------------------------
const char* UserMotionLabels::toUserLabel(FighterID fighterID, FighterMotion motion, const char* fallback) const
{
    assert(fighterID.isValid());
    assert(motion.isValid());

    const auto& map = fighters_[fighterID.value()].motionMap;
    const auto& layers = fighters_[fighterID.value()].layers;

    const auto it = map.find(motion);
    if (it != map.end())
        for (int layerIdx = layerCount() - 1; layerIdx >= 0; --layerIdx)  // Prioritize higher layers first
            if (layers[layerIdx].userLabels[it->value()].length() > 0)    // Found a non-empty string
                return layers[layerIdx].userLabels[it->value()].cStr();   // Map value is an index into the user label table

    return fallback;
}

// ----------------------------------------------------------------------------
void UserMotionLabels::expandTablesUpTo(FighterID fighterID)
{
    assert(fighterID.isValid());

    while (fighters_.count() < fighterID.value() + 1)
    {
        auto& fighter = fighters_.emplace();
        while (fighter.layers.count() < layerCount())
            fighter.layers.emplace();
        while (fighter.layerMaps.count() < layerCount())
            fighter.layerMaps.emplace();
    }
}

}
