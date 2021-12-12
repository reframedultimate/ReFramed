#pragma once

#include "rfcommon/FighterStatusMapping.hpp"
#include "rfcommon/HitStatusMapping.hpp"
#include "rfcommon/FighterIDMapping.hpp"
#include "rfcommon/StageIDMapping.hpp"

namespace rfcommon {

class MappingInfo
{
public:
    MappingInfo(uint32_t checksum);

    /*!
     * \brief Loads the mapping info struct from a JSON file.
     */
    static MappingInfo* load(const String& fileName);

    /*!
     * \brief Saves the mapping info struct to a JSON file.
     */
    bool save(const String& fileName) const;

    /*!
     * \brief This is the checksum value we received from the server when
     * requesting the mapping info. This is used to determine if our local
     * copy is outdated or not.
     */
    uint32_t checksum() const
        { return checksum_; }

private:
    uint32_t checksum_;

public:
    FighterStatusMapping fighterStatus;
    HitStatusMapping hitStatus;
    FighterIDMapping fighterID;
    StageIDMapping stageID;
};

}
