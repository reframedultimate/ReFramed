#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/Profiler.hpp"
#include "nlohmann/json.hpp"

namespace rfcommon {

using nlohmann::json;

// ----------------------------------------------------------------------------
VideoMeta::VideoMeta(const char* fileName, rfcommon::FrameIndex offset, bool embedded)
    : fileName_(fileName)
    , offset_(offset)
    , isEmbedded_(embedded)
{}

// ----------------------------------------------------------------------------
VideoMeta::~VideoMeta()
{}

// ----------------------------------------------------------------------------
VideoMeta* VideoMeta::load(const void* data, uint32_t size)
{
    PROFILE(VideoMeta, load);

    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(data);
    const unsigned char* const end = static_cast<const unsigned char*>(data) + size;
    json j = json::parse(begin, end, nullptr, false);
    if (j == json::value_t::discarded)
        return nullptr;

    if (j["version"] == "1.0")
    {
        json jFileName = j["filename"];
        json jOffset = j["offset"];
        json jEmbedded = j["embedded"];

        const std::string fileName = jFileName.is_string() ?
                    jFileName.get<std::string>() : "";
        const auto offset = jOffset.is_number_integer() ?
                    FrameIndex::fromValue(jOffset.get<FrameIndex::Type>()) : FrameIndex::fromValue(0);
        const bool embedded = jEmbedded.is_binary() ?
                    jEmbedded.get<bool>() : false;

        if (fileName == "")
            return nullptr;

        return new VideoMeta(fileName.c_str(), offset, embedded);
    }

    // unsupported version
    return nullptr;
}

// ----------------------------------------------------------------------------
uint32_t VideoMeta::save(FILE* fp)
{
    PROFILE(VideoMeta, save);

    json j = {
        {"version", "1.0"},
        {"filename", fileName_.cStr()},
        {"offset", offset_.index()},
        {"embedded", isEmbedded_}
    };

    const std::string jsonAsString = j.dump();
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
        return 0;

    return jsonAsString.length();
}

}
