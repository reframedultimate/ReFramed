#include "rfcommon/SetFormat.hpp"
#include <cassert>
#include <exception>
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
SetFormat::SetFormat(Type type, const char* otherDesc)
    : type_(type)
    , otherDesc_(otherDesc)
{
}

// ----------------------------------------------------------------------------
SetFormat::SetFormat(const char* desc)
    : type_([&desc]() -> Type {
#define X(type, shortstr, longstr) if (strcmp(desc, shortstr) == 0 || strcmp(desc, longstr) == 0) return type;
        SET_FORMAT_LIST
#undef X
        return OTHER;
      }())
{
    if (type_ == OTHER)
        otherDesc_ = desc;
}

SetFormat SetFormat::makeOther(const char* description) { return SetFormat(OTHER, description); }
SetFormat SetFormat::fromDescription(const char* description) { return SetFormat(description); }
SetFormat SetFormat::fromType(Type type) { assert(type != OTHER); return SetFormat(type, ""); }
SetFormat SetFormat::fromIndex(int index) { assert(index < OTHER); return SetFormat(static_cast<Type>(index), ""); }

// ----------------------------------------------------------------------------
const char* SetFormat::shortDescription() const
{
    if (type_ == OTHER)
        return otherDesc_.cStr();

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
    if (type_ == OTHER)
        return otherDesc_.cStr();

    switch (type_)
    {
#define X(type, shortstr, longstr) case type: return longstr;
        SET_FORMAT_LIST
#undef X
        default: std::terminate();
    }
}

// ----------------------------------------------------------------------------
bool SetFormat::operator==(const SetFormat& rhs)
{
    if (type_ == OTHER)
        return type_ == rhs.type_ && otherDesc_ == rhs.otherDesc_;
    return type_ == rhs.type_;
}

// ----------------------------------------------------------------------------
bool SetFormat::operator!=(const SetFormat& rhs)
{
    return !operator==(rhs);
}

}
