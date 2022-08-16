#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/MappingInfoStatus.hpp"
#include "rfcommon/MappingInfoHitStatus.hpp"
#include "rfcommon/MappingInfoStage.hpp"
#include "rfcommon/MappingInfoFighter.hpp"
#include <cstdio>

namespace rfcommon {

class FrameData;
class MetaData;

class RFCOMMON_PUBLIC_API MappingInfo : public RefCounted
{
public:
    MappingInfo(uint32_t checksum);
    ~MappingInfo();

    static MappingInfo* load(const void* data, uint32_t size);
    uint32_t save(FILE* fp) const;
    uint32_t saveNecessary(FILE* fp, const MetaData* metaData, const FrameData* frameData) const;

    /*!
     * \brief This is the checksum value we received from the server when
     * requesting the mapping info. This is used to determine if our local
     * copy is outdated or not. Gets saved along with the rest of the data.
     */
    uint32_t checksum() const;

    MappingInfoStatus status;
    MappingInfoHitStatus hitStatus;
    MappingInfoStage stage;
    MappingInfoFighter fighter;

private:
    const uint32_t checksum_;
};

}
