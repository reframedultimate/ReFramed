#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

#define USER_LABEL_CATEGORIES_LIST \
    X(MOVEMENT, "Movement") \
    X(GROUND_ATTACKS, "Ground Attacks") \
    X(AERIAL_ATTACKS, "Aerial Attacks") \
    X(SPECIAL_ATTACKS, "Special Attacks") \
    X(GRABS, "Grabs") \
    X(LEDGE, "Ledge") \
    X(DEFENSIVE, "Defensive") \
    X(MISC, "Misc") \
    X(UNLABELED, "Unlabeled")


enum class UserLabelCategory {
#define X(name, desc) name,
    USER_LABEL_CATEGORIES_LIST
#undef X
    COUNT
};

class RFCOMMON_PUBLIC_API MappingInfoHitStatus
{
public:
    const char* hitStatusString(FighterHitStatus status) const;
    const FighterHitStatus* hitStatus(const char* name) const;
    void addHitStatus(FighterHitStatus status, const char* name);

private:
    SmallLinearMap<FighterHitStatus, String, 6> hitStatusMap_;
};

class RFCOMMON_PUBLIC_API MappingInfoStageID
{
public:
    const char* stageName(StageID stageID) const;
    const StageID* stage(const char* name) const;
    void addStage(StageID stageID, const char* name);

private:
    SmallLinearMap<StageID, String, 10> stageMap_;
};

class RFCOMMON_PUBLIC_API MappingInfoFighterID
{
public:
    const char* fighterName(FighterID fighterID) const;
    const FighterID* fighterID(const char* name) const;
    void addFighter(FighterID fighterID, const char* name);

private:
    HashMap<FighterID, String, FighterID::Hasher> fighterNameMap_;
    HashMap<String, FighterID> fighterMap_;
};

class RFCOMMON_PRIVATE_API MappingInfoMotionLabels
{
public:
    MappingInfoMotionLabels();
    ~MappingInfoMotionLabels();

private:
    HashMap<FighterMotion, SmallString<31>, FighterMotion::Hasher> motionMap;
    HashMap<SmallString<31>, FighterMotion> labelMap;
};

class RFCOMMON_PUBLIC_API MappingInfoStatus
{
public:
    MappingInfoStatus();
    ~MappingInfoStatus();

    String toEnumName(FighterID fighterID, FighterStatus status);
    FighterID toFighterID(const char* enumName);

    //const char* statusString(FighterStatus status) const;
    const char* statusString(FighterStatus status, FighterID fighterID) const;
    const FighterStatus* status(const char* name) const;

    void addBaseStatus(FighterStatus status, const char* name);
    void addSpecificStatus(FighterStatus status, FighterID fighterID, const char* name);

private:
    HashMap<FighterStatus, SmallString<31>, FighterStatus::Hasher> statusMap;
    HashMap<SmallString<31>, FighterStatus> enumNameMap;
    HashMap<FighterStatus, String, FighterStatus::Hasher> statusStringMap_;
    HashMap<FighterID, HashMap<FighterStatus, String, FighterStatus::Hasher>, FighterID::Hasher> statusStringSpecificMap_;
    HashMap<String, FighterStatus> statusMap_;
};

class RFCOMMON_PUBLIC_API MappingInfoMotion
{
public:
    enum MatchFlags
    {
        MATCH_MOTION = 0x01,
        MATCH_STATUS = 0x02,
    };

    struct Entry
    {
        SmallVector<String, 2> userLabels;
        FighterMotion motion;
        FighterStatus status;
        UserLabelCategory category;
        uint8_t matchFlags;
    };

    MappingInfoMotion();
    ~MappingInfoMotion();

    String toLabel(FighterMotion);
    const char* toLabel(FighterMotion motion, const char* fallback);
    String toUserLabel(FighterMotion motion);
    const char* toUserLabel(FighterMotion motion, const char* fallback);
    FighterMotion fromLabel(const char* label);
    SmallVector<FighterMotion, 4> fromUserLabel(const char* userLabel, FighterID fighterID);

private:
    Vector<Entry> entries_;
};

class RFCOMMON_PUBLIC_API MappingInfo : public RefCounted
{
public:
    MappingInfo(uint32_t checksum);
    ~MappingInfo();

    static MappingInfo* load(FILE* fp, uint32_t size);
    uint32_t save(FILE* fp) const;

    /*!
     * \brief This is the checksum value we received from the server when
     * requesting the mapping info. This is used to determine if our local
     * copy is outdated or not. Gets saved along with the rest of the data.
     */
    uint32_t checksum() const
        { return checksum_; }

    MappingInfoMotion motion;
    MappingInfoStatus status;
    MappingInfoStageID stageID;
    MappingInfoFighterID fighterID;

private:
    const uint32_t checksum_;
};

}
