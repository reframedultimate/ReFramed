#include "rfcommon/SetFormat.hpp"
#include "rfcommon/Profiler.hpp"
#include <cassert>
#include <exception>
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
SetFormat::SetFormat(Type type)
    : type_(type)
{
}

// ----------------------------------------------------------------------------
SetFormat::SetFormat(const char* desc)
    : type_([&desc]() -> Type {
#define X(type, shortstr, longstr) if (strcmp(desc, shortstr) == 0 || strcmp(desc, longstr) == 0) return type;
        SET_FORMAT_LIST
#undef X
        return FREE;
      }())
{
}

// ----------------------------------------------------------------------------
SetFormat SetFormat::fromDescription(const char* description) { return SetFormat(description); }
SetFormat SetFormat::fromType(Type type) { return SetFormat(type); }
SetFormat SetFormat::fromIndex(int index) { assert(index >= 0 && index <= SetFormat::FREE); return SetFormat(static_cast<Type>(index)); }

// ----------------------------------------------------------------------------
const char* SetFormat::shortDescription() const
{
    PROFILE(SetFormat, shortDescription);

    switch (type_)
    {
#define X(type, shortstr, longstr) case type: return shortstr;
        SET_FORMAT_LIST
#undef X
        default: std::terminate();
    }
}

// ----------------------------------------------------------------------------
const char* SetFormat::longDescription() const
{
    PROFILE(SetFormat, longDescription);

    switch (type_)
    {
#define X(type, shortstr, longstr) case type: return longstr;
        SET_FORMAT_LIST
#undef X
        default: std::terminate();
    }
}

// ----------------------------------------------------------------------------
bool SetFormat::operator==(const SetFormat& rhs) const
{
    return type_ == rhs.type_;
}

// ----------------------------------------------------------------------------
bool SetFormat::operator!=(const SetFormat& rhs) const
{
    return !operator==(rhs);
}

}

