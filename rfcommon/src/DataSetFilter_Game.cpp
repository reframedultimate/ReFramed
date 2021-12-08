#include "rfcommon/DataSetFilter_Game.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/SavedGameSession.hpp"
#include "rfcommon/PlayerState.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSetFilter_Game::DataSetFilter_Game()
    : format_(SetFormat::FRIENDLIES)
{
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Game::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    out->reserve(dataSet->dataPointCount());
    for (const DataPoint* p = dataSet->dataPointsBegin(); p != dataSet->dataPointsEnd(); ++p)
    {
        const SavedGameSession* s = p->session();
        DeltaTimeMS length = s->lengthMs();

        bool lengthInRange = (length >= minLength_ && length <= maxLength_);
        bool formatType = (anySetFormat_ ? true : s->format().type() == format_.type());
        bool formatName = (s->format().type() == SetFormat::OTHER ? s->format().description() == format_.description() : true);

        if (lengthInRange && formatType && formatName)
            out->addDataPointToEnd(*p);
    }

    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Game::applyInverse(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    out->reserve(dataSet->dataPointCount());
    for (const DataPoint* p = dataSet->dataPointsBegin(); p != dataSet->dataPointsEnd(); ++p)
    {
        const SavedGameSession* s = p->session();
        TimeStampMS length = s->lengthMs();

        bool lengthInRange = (length >= minLength_ && length <= maxLength_);
        bool formatType = (anySetFormat_ ? true : s->format().type() == format_.type());
        bool formatName = (s->format().type() == SetFormat::OTHER ? s->format().description() == format_.description() : true);

        if (!(lengthInRange && formatType && formatName))
            out->addDataPointToEnd(*p);
    }

    return out;
}

// ----------------------------------------------------------------------------
void DataSetFilter_Game::setSetFormat(const SetFormat& format)
{
    format_ = format;
    notifyDirty();
}

// ----------------------------------------------------------------------------
void DataSetFilter_Game::setAnySetFormat(bool filter)
{
    anySetFormat_ = filter;
    notifyDirty();
}

// ----------------------------------------------------------------------------
void DataSetFilter_Game::setWinner(const String& name)
{
    winner_ = name;
    notifyDirty();
}

// ----------------------------------------------------------------------------
void DataSetFilter_Game::setMinLengthMs(uint64_t length)
{
    minLength_ = length;
    notifyDirty();
}

// ----------------------------------------------------------------------------
void DataSetFilter_Game::setMaxLengthMs(uint64_t length)
{
    maxLength_ = length;
    notifyDirty();
}

}
