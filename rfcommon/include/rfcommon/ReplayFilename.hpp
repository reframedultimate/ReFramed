#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/BracketType.hpp"
#include "rfcommon/Round.hpp"
#include "rfcommon/ScoreCount.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/String.hpp"

namespace rfcommon {

class MappingInfo;
class Metadata;

class RFCOMMON_PUBLIC_API ReplayFilename
{
public:
    /*!
     * \brief Creates a suitable filename for a given session's metadata.
     * 
     * \param[in] map Mapping info from session
     * \param[in] mdata Metadata from session
     * \param[in] deduplicate If the returned filename exists, then you can
     * specify an additional index (value greater than 0) to be added to the
     * filename to avoid duplicates.
     */
    static String fromMetadata(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata);
};

}
