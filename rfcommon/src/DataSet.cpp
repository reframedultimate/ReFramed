#include "rfcommon/DataSet.hpp"
#include "rfcommon/SavedGameSession.hpp"
#include <algorithm>

namespace rfcommon {

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
    if (points_.count() > 0)
        assert(points_.back().state().timeStampMs() <= dataPoint.state().timeStampMs());
#endif
    points_.push(dataPoint);
}

// ----------------------------------------------------------------------------
void DataSet::addSessionNoSort(SavedGameSession* gameSession)
{
    for (int p = 0; p != gameSession->fighterCount(); ++p)
        for (int i = 0; i != gameSession->frameCount(); ++i)
            points_.emplace(gameSession->frame(p, i), gameSession, p);
}

// ----------------------------------------------------------------------------
void DataSet::mergeDataFrom(const DataSet* other)
{
    if (other->dataPointCount() == 0)
        return;

    const size_t offset = std::lower_bound(points_.begin(), points_.end(), *other->dataPointsBegin(), [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    }) - points_.begin();

    points_.reserve(points_.count() + other->dataPointCount());
    for (const DataPoint* p = other->dataPointsBegin(); p != other->dataPointsEnd(); ++p)
        points_.emplace(*p);

    std::sort(points_.begin() + offset, points_.end(), [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    });
}

// ----------------------------------------------------------------------------
void DataSet::replaceDataWith(const DataSet* other)
{
    clear();
    mergeDataFrom(other);
}

// ----------------------------------------------------------------------------
void DataSet::sort()
{
    std::sort(points_.begin(), points_.end(), [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    });
}

// ----------------------------------------------------------------------------
void DataSet::clear()
{
    points_.clearCompact();
}

}
