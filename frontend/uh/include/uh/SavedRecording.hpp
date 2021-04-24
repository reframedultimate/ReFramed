#pragma once

#include "uh/config.hpp"
#include "uh/Recording.hpp"

namespace uh {

class UH_PUBLIC_API SavedRecording : public Recording
{
public:
    static SavedRecording* load(const std::string& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo& mappingInfo() { return mappingInfo_; }

private:
    SavedRecording(MappingInfo&& mapping,
                   std::vector<uint8_t>&& playerFighterIDs,
                   std::vector<std::string>&& playerTags,
                   uint16_t stageID);

    // Using void* here to avoid json objects leaking into the rest of the
    // program
    static SavedRecording* loadVersion_1_0(const void* jptr);
    static SavedRecording* loadVersion_1_1(const void* jptr);
    static SavedRecording* loadVersion_1_2(const void* jptr);
    static SavedRecording* loadVersion_1_3(const void* jptr);
    static SavedRecording* loadVersion_1_4(const void* jptr);
};

}
