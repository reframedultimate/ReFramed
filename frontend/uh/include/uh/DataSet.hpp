#pragma once

#include "uh/config.hpp"
#include "uh/RefCounted.hpp"
#include "uh/DataPoint.hpp"
#include "uh/Vector.hpp"
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
    void addRecordingNoSort(Recording* recording);
    void mergeDataFrom(const DataSet* other);
    void replaceDataWith(const DataSet* other);
    void sort();
    void clear();

    int dataPointCount() const { return static_cast<int>(points_.count()); }
    const DataPoint* dataPointsBegin() const { return points_.begin(); }
    const DataPoint* dataPointsEnd() const { return points_.end(); }

private:
    Vector<DataPoint> points_;
};

}
