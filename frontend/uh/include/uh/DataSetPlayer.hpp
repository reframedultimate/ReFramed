#pragma once

#include "uh/config.hpp"
#include "uh/DataPoint.hpp"
#include <string>

namespace uh {

class UH_PUBLIC_API DataSetPlayer
{
public:
    DataSetPlayer(const std::string& playerName);

    void appendPlayerStatesFromRecording(int player, Recording* recording);
    void removePlayerStatesForRecording(int player, Recording* recording);
    void clear();

    const std::string& playerName() const { return playerName_; }
    const std::vector<DataPoint>& dataPoints() const { return points_; }

private:
    std::string playerName_;
    std::vector<DataPoint> points_;
};

}
