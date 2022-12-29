#include "rfcommon/BracketType.hpp"
#include "rfcommon/Profiler.hpp"
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
BracketType::BracketType(Type type, const char* otherDesc)
    : type_(type)
    , otherDesc_(otherDesc)
{
}

// ----------------------------------------------------------------------------
BracketType::BracketType(const char* desc)
    : type_([&desc]() -> Type {
#define X(type, str) if (strcmp(desc, str) == 0) return type;
        BRACKET_TYPE_LIST
#undef X
        return OTHER;
      }())
{
    if (type_ == OTHER)
        otherDesc_ = desc;
}

BracketType BracketType::makeOther(const char* description) { return BracketType(OTHER, description); }
BracketType BracketType::fromDescription(const char* description) { return BracketType(description); }
BracketType BracketType::fromType(Type type) { assert(type != OTHER); return BracketType(type, ""); }
BracketType BracketType::fromIndex(int index) { assert(index >= 0 && index < OTHER); return BracketType(static_cast<Type>(index), ""); }

// ----------------------------------------------------------------------------
const char* BracketType::description() const
{
    PROFILE(BracketType, description);

    if (type_ == OTHER)
        return otherDesc_.cStr();

    switch (type_)
    {
#define X(type, str) case type: return str;
        BRACKET_TYPE_LIST
#undef X
        default: std::terminate();
    }
}

// ----------------------------------------------------------------------------
bool BracketType::operator==(const BracketType& rhs) const
{
    if (type_ == OTHER)
        return type_ == rhs.type_ && otherDesc_ == rhs.otherDesc_;
    return type_ == rhs.type_;
}

// ----------------------------------------------------------------------------
bool BracketType::operator!=(const BracketType& rhs) const
{
    return !operator==(rhs);
}

}
