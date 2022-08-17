#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include <cstdio>

#define RFCOMMON_USER_LABEL_CATEGORIES_LIST \
    X(MOVEMENT, "Movement") \
    X(GROUND_ATTACKS, "Ground Attacks") \
    X(AERIAL_ATTACKS, "Aerial Attacks") \
    X(SPECIAL_ATTACKS, "Special Attacks") \
    X(GRABS, "Grabs") \
    X(LEDGE, "Ledge") \
    X(DEFENSIVE, "Defensive") \
    X(MISC, "Misc") \
    X(UNLABELED, "Unlabeled")

namespace rfcommon {

class Hash40Strings;
class UserMotionLabelsListener;

class RFCOMMON_PUBLIC_API FighterUserMotionLabels
{
public:
    enum Category
    {
#define X(name, desc) name,
        RFCOMMON_USER_LABEL_CATEGORIES_LIST
#undef X
        COUNT
    };

    FighterUserMotionLabels();
    ~FighterUserMotionLabels();

private:
    friend class UserMotionLabels;

    struct Layer
    {
        Vector<String> userLabels;
    };

    // These vectors hold all of the data in the table
    Vector<FighterMotion> motions;
    Vector<Category> categories;
    SmallVector<Layer, 4> layers;

    struct LayerMap
    {
        HashMap<String, SmallVector<int, 4>> userMap;
    };

    HashMap<FighterMotion, int, FighterMotion::Hasher> motionMap;
    SmallVector<LayerMap, 4> layerMaps;
};

class RFCOMMON_PUBLIC_API UserMotionLabels : public RefCounted
{
public:
    UserMotionLabels();
    ~UserMotionLabels();

    bool loadLayer(const void* address, uint32_t size);
    bool loadUnlabeled(const void* address, uint32_t size);
    uint32_t saveLayer(FILE* fp, const int layerIdx) const;
    uint32_t saveUnlabeled(FILE* fp) const;
    int newEmptyLayer(const char* name);
    void removeLayer(int layerIdx);

    const char* layerName(int layerIdx) const
        { return layerNames_[layerIdx].cStr(); }
    
    int layerCount() const 
        { return layerNames_.count(); }

    void addUnknownMotion(FighterID fighterID, FighterMotion motion);
    bool addEntry(FighterID fighterID, int layerIdx, FighterMotion motion, const char* userLabel, FighterUserMotionLabels::Category category);
    bool modifyEntry(FighterID fighterID, int layerIdx, FighterMotion motion, const char* oldUserLabel, const char* newUserLabel, FighterUserMotionLabels::Category newCategory);
    bool clearEntry(FighterID fighterID, int layerIdx, FighterMotion motion);

    SmallVector<FighterMotion, 4> toMotion(FighterID fighterID, const char* userLabel) const;
    const char* toUserLabel(FighterID fighterID, FighterMotion motion) const;
    const char* toUserLabel(FighterID fighterID, FighterMotion motion, const char* fallback) const;

    int entryCount(FighterID fighterID) const 
        { return fighterID.value() < fighters_.count() ? fighters_[fighterID.value()].motions.count() : 0; }
    FighterUserMotionLabels::Category categoryAt(FighterID fighterID, int entryIdx) const
        { return fighters_[fighterID.value()].categories[entryIdx]; }
    FighterMotion motionAt(FighterID fighterID, int entryIdx) const
        { return fighters_[fighterID.value()].motions[entryIdx]; }
    const char* userLabelAt(FighterID fighterID, int layerIdx, int entryIdx) const
        { return fighters_[fighterID.value()].layers[layerIdx].userLabels[entryIdx].cStr(); }

    ListenerDispatcher<UserMotionLabelsListener> dispatcher;

private:
    void expandTablesUpTo(FighterID fighterID);

private:
    Vector<FighterUserMotionLabels> fighters_;
    Vector<String> layerNames_;
};

}
