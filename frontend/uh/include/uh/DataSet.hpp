#pragma once

#include "uh/config.hpp"
#include "uh/RefCounted.hpp"
#include "uh/DataSetPlayer.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace uh {

class Recording;

/*!
 * \brief This is the top-level structure used to hold the player state data
 * for analyzers to work with.
 */
class UH_PUBLIC_API DataSet : public RefCounted
{
public:
    void appendRecording(Recording* recording);
    void removeRecording(Recording* recording);

    const DataSetPlayer* playerDataSet(const std::string& name);
    std::vector<std::string> playerNames() const;

private:
    std::unordered_map<std::string, DataSetPlayer> players_;
};

}
