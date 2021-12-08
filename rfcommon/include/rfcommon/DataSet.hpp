#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/DataPoint.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class SavedGameSession;

extern template class RFCOMMON_TEMPLATE_API Vector<DataPoint>;

/*!
 * \brief This is the top-level structure used to hold the player state data
 * for analyzers to work with.
 */
class RFCOMMON_PUBLIC_API DataSet : public RefCounted
{
public:
    void reserve(int count);
    //void addDataPoint(const DataPoint& dataPoint);
    void addDataPointToEnd(const DataPoint& dataPoint);
    void addSessionNoSort(SavedGameSession* recording);
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
