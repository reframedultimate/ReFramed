#include "overextension/models/OverextensionModel.hpp"

// ----------------------------------------------------------------------------
void OverextensionModel::startNewSession(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata)
{

}

// ----------------------------------------------------------------------------
void OverextensionModel::addFrame(int frameIdx, const rfcommon::FrameData* fdata)
{

}

void OverextensionModel::addAllFrames(const rfcommon::FrameData* fdata)
{

}

// ----------------------------------------------------------------------------
void OverextensionModel::clearAll()
{

}

// ----------------------------------------------------------------------------
int OverextensionModel::fighterCount() const
{
    return 0;
}

// ----------------------------------------------------------------------------
int OverextensionModel::currentFighter() const
{
    return -1;
}

// ----------------------------------------------------------------------------
void OverextensionModel::setCurrentFighter(int fighterIdx) const
{

}

// ----------------------------------------------------------------------------
const char* OverextensionModel::playerName(int fighterIdx) const
{
    return "";
}

// ----------------------------------------------------------------------------
const char* OverextensionModel::fighterName(int fighterIdx) const
{
    return "";
}

// ----------------------------------------------------------------------------
int OverextensionModel::numTotal(int fighterIdx) const
{
    return 0;
}

// ----------------------------------------------------------------------------
int OverextensionModel::numTrueCombos(int fighterIdx) const
{
    return 0;
}

// ----------------------------------------------------------------------------
int OverextensionModel::numPositiveOverextensions(int fighterIdx) const
{
    return 0;
}

// ----------------------------------------------------------------------------
int OverextensionModel::numNegativeOverextensions(int fighterIdx) const
{
    return 0;
}

// ----------------------------------------------------------------------------
int OverextensionModel::numFrameTraps(int fighterIdx) const
{
    return 0;
}
