#include "rfcommon/MappingInfo.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
MappingInfo::MappingInfo(uint32_t checksum)
    : checksum_(checksum)
{}

// ----------------------------------------------------------------------------
MappingInfo::~MappingInfo()
{}

// ----------------------------------------------------------------------------
MappingInfo* MappingInfo::load(FILE* fp, uint32_t size)
{
    return nullptr;
}

// ----------------------------------------------------------------------------
uint32_t MappingInfo::save(FILE* fp) const
{
    return 0;
}

// ----------------------------------------------------------------------------
uint32_t MappingInfo::checksum() const
{
    return checksum_;
}

}
