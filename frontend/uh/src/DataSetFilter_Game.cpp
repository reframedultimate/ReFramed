#include "uh/DataSetFilter_Game.hpp"
#include "uh/DataSet.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"

namespace uh {

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
        const Recording* rec = p->recording();
        uint64_t length = rec->gameLengthMs();

        bool lengthInRange = (length >= minLength_ && length <= maxLength_);
        bool formatType = (anySetFormat_ ? true : rec->format().type() == format_.type());
        bool formatName = (rec->format().type() == SetFormat::OTHER ? rec->format().description() == format_.description() : true);

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
        const Recording* rec = p->recording();
        uint64_t length = rec->gameLengthMs();

        bool lengthInRange = (length >= minLength_ && length <= maxLength_);
        bool formatType = (anySetFormat_ ? true : rec->format().type() == format_.type());
        bool formatName = (rec->format().type() == SetFormat::OTHER ? rec->format().description() == format_.description() : true);

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
