#include "uh/DataSetPlayer.hpp"
#include "uh/DataPoint.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"
#include <cassert>
#include <cstdlib>
#include <new>
#include <algorithm>

namespace uh {

// ----------------------------------------------------------------------------
DataSetPlayer::DataSetPlayer(const std::string& playerName)
    : playerName_(playerName)
{
}

// ----------------------------------------------------------------------------
void DataSetPlayer::appendDataPoint(const DataPoint& dataPoint)
{
    points_.push_back(dataPoint);
}

// ----------------------------------------------------------------------------
void DataSetPlayer::appendPlayerStatesFromRecording(int player, Recording* recording)
{
    if (recording->playerStateCount(player) == 0)
        return;

    std::vector<DataPoint> newPoints;
    for (int i = 0; i != recording->playerStateCount(player); ++i)
        newPoints.emplace_back(recording->playerState(player, i), recording);

    const auto insertIt = std::lower_bound(points_.begin(), points_.end(), newPoints[0], [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    });

    points_.insert(insertIt, newPoints.begin(), newPoints.end());
}

// ----------------------------------------------------------------------------
void DataSetPlayer::removePlayerStatesForRecording(int player, Recording* recording)
{
    if (recording->playerStateCount(player) == 0)
        return;

    auto it = std::lower_bound(points_.begin(), points_.end(),
        DataPoint(recording->playerState(player, 0), recording),
        [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
            return lhs.state().timeStampMs() < rhs.state().timeStampMs();
        }
    );

    const auto upper_bound = std::upper_bound(points_.begin(), points_.end(),
        DataPoint(recording->playerState(player, recording->playerStateCount(player) - 1), recording),
        [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
            return lhs.state().timeStampMs() < rhs.state().timeStampMs();
        }
    );

    while (it != upper_bound)
    {
        if (it->recording() == recording)
            it = points_.erase(it);
        else
            ++it;
    }
}

// ----------------------------------------------------------------------------
void DataSetPlayer::clear()
{
    points_.clear();
}

// ----------------------------------------------------------------------------
void DataSetPlayer::mergeDataFrom(const DataSetPlayer& other)
{
    if (other.points_.size() == 0)
        return;

    const auto insertIt = std::lower_bound(points_.begin(), points_.end(), other.points_[0], [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    });

    points_.insert(insertIt, other.points_.begin(), other.points_.end());
}

// ----------------------------------------------------------------------------
void DataSetPlayer::replaceDataWith(const DataSetPlayer& other)
{
    points_ = other.points_;
}

}
