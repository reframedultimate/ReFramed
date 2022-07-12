#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/LinearMap.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API EnumStrings
{
    EnumStrings(uint32_t checksum);

    /*!
     * \brief This is the checksum value we received from the server when
     * requesting the mapping info. This is used to determine if our local
     * copy is outdated or not. Gets saved along with the rest of the data.
     */
    uint32_t checksum() const
        { return checksum_; }

    bool saveJSON(const char* fileName) const;
    bool loadJSON(const char* fileName);

    //const char* statusString(FighterStatus status) const;
    const char* statusString(FighterStatus status, FighterID fighterID) const;
    const FighterStatus* status(const char* name) const;

    void addBaseStatus(FighterStatus status, const char* name);
    void addSpecificStatus(FighterStatus status, FighterID fighterID, const char* name);

    const char* hitStatusString(FighterHitStatus status) const;
    const FighterHitStatus* hitStatus(const char* name) const;
    void addHitStatus(FighterHitStatus status, const char* name);

    const char* fighterName(FighterID fighterID) const;
    const FighterID* fighterID(const char* name) const;
    void addFighter(FighterID fighterID, const char* name);

    const char* stageName(StageID stageID) const;
    const StageID* stage(const char* name) const;
    void addStage(StageID stageID, const char* name);

private:
    HashMap<FighterStatus, String, FighterStatus::Hasher> statusStringMap_;
    HashMap<FighterID, HashMap<FighterStatus, String, FighterStatus::Hasher>, FighterID::Hasher> statusStringSpecificMap_;
    HashMap<String, FighterStatus> statusMap_;
    HashMap<FighterID, String, FighterID::Hasher> fighterNameMap_;
    HashMap<String, FighterID> fighterMap_;
    SmallLinearMap<FighterHitStatus, String, 6> hitStatusMap_;
    SmallLinearMap<StageID, String, 10> stageMap_;

    const uint32_t checksum_;
};

}
