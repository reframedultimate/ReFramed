#include "uh/DataSet.hpp"
#include "uh/Recording.hpp"
#include <algorithm>

namespace uh {

// ----------------------------------------------------------------------------
void DataSet::reserve(int count)
{
    points_.reserve(count);
}

// ----------------------------------------------------------------------------
/*
void DataSet::addDataPoint(const DataPoint& dataPoint)
{
    const auto insertIt = std::lower_bound(points_.begin(), points_.end(), dataPoint, [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    });

    points_.insert(insertIt, dataPoint);
}*/

// ----------------------------------------------------------------------------
void DataSet::addDataPointToEnd(const DataPoint& dataPoint)
{
#ifndef NDEBUG
    if (points_.size() > 0)
        assert(points_.back().state().timeStampMs() <= dataPoint.state().timeStampMs());
#endif
    points_.push_back(dataPoint);
}

// ----------------------------------------------------------------------------
void DataSet::addRecording(Recording* recording)
{
    int dataPointCount = 0;
    for (int p = 0; p != recording->playerCount(); ++p)
        dataPointCount += recording->playerStateCount(p);

    std::vector<DataPoint> newPoints;
    newPoints.reserve(dataPointCount);
    for (int p = 0; p != recording->playerCount(); ++p)
        for (int i = 0; i != recording->playerStateCount(p); ++i)
            newPoints.emplace_back(recording->playerStateAt(p, i), recording, p);

    const int insertOffset = std::lower_bound(points_.begin(), points_.end(), DataPoint(recording->firstReceivedState(), recording, 0), [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    }) - points_.begin();

    points_.insert(points_.begin() + insertOffset, std::make_move_iterator(newPoints.begin()), std::make_move_iterator(newPoints.end()));

    std::sort(points_.begin() + insertOffset, points_.end(), [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    });
}

// ----------------------------------------------------------------------------
void DataSet::mergeDataFrom(const DataSet* other)
{
    if (other->dataPointCount() == 0)
        return;

    const auto insertIt = std::lower_bound(points_.begin(), points_.end(), *other->dataPointsBegin(), [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    });

    points_.insert(insertIt, other->dataPointsBegin(), other->dataPointsEnd());
}

// ----------------------------------------------------------------------------
void DataSet::replaceDataWith(const DataSet* other)
{
    clear();
    mergeDataFrom(other);
}

// ----------------------------------------------------------------------------
void DataSet::clear()
{
    points_.clear();
}

}
