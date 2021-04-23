#include "uh/DataSet.hpp"
#include "uh/DataSetPlayer.hpp"
#include "uh/Recording.hpp"

namespace uh {

// ----------------------------------------------------------------------------
void DataSet::appendRecording(Recording* recording)
{
    for (int i = 0; i != recording->playerCount(); ++i)
    {
        const std::string& name = recording->playerName(i);
        auto result = players_.emplace(name, DataSetPlayer(name));
        DataSetPlayer& ds = result.first->second;
        ds.appendPlayerStatesFromRecording(i, recording);
    }
}

// ----------------------------------------------------------------------------
void DataSet::removeRecording(Recording* recording)
{
    for (int i = 0; i != recording->playerCount(); ++i)
    {
        auto it = players_.find(recording->playerName(i));
        if (it == players_.end())
            continue;

        DataSetPlayer& ds = it->second;;
        ds.removePlayerStatesForRecording(i, recording);
        if (ds.dataPoints().size() == 0)
            players_.erase(it);
    }
}

// ----------------------------------------------------------------------------
const DataSetPlayer* DataSet::playerDataSet(const std::string& name)
{
    auto it = players_.find(name);
    if (it != players_.end())
        return &it->second;
    return nullptr;
}

// ----------------------------------------------------------------------------
std::vector<std::string> DataSet::playerNames() const
{
    std::vector<std::string> names(players_.size());
    for (const auto& [name, dp] : players_)
        names.push_back(name);
    return names;
}

}
