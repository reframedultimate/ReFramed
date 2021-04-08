#pragma once

#include "uh/models/Recording.hpp"

namespace uh {

class SavedRecording : public Recording
{
public:
    static SavedRecording* load(const QString& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo& mappingInfo() { return mappingInfo_; }
};

}
