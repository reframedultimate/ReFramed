#pragma once

#include "uh/config.hpp"
#include "uh/RefCounted.hpp"
#include "uh/DataPoint.hpp"
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
    void reserve(int count);
    //void addDataPoint(const DataPoint& dataPoint);
    void addDataPointToEnd(const DataPoint& dataPoint);
    void addRecording(Recording* recording);
    void mergeDataFrom(const DataSet* other);
    void replaceDataWith(const DataSet* other);
    void clear();

    int dataPointCount() const { return static_cast<int>(points_.size()); }
    const DataPoint* dataPointsBegin() const { return points_.data(); }
    const DataPoint* dataPointsEnd() const { return points_.data() + dataPointCount(); }

private:
    std::vector<DataPoint> points_;
};

}
