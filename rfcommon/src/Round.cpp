#include "rfcommon/Profiler.hpp"
#include "rfcommon/Round.hpp"

namespace rfcommon {

static int parse_string(const char* s, const char* prefix, int* num)
{
    PROFILE(RoundGlobal, parse_string);

    while(*prefix)
    {
        if(*prefix++ != *s++)
            return 0;
    }

    while (*s && *s == ' ')
        ++s;
    *num = atoi(s);

    return 1;
}

// ----------------------------------------------------------------------------
Round::Round(Type type, SessionNumber number)
    : type_(type)
    , number_(number)
{}

// ----------------------------------------------------------------------------
Round::Round(const char* desc)
    : type_(FREE)
    , number_(SessionNumber::fromValue(1))
{
    int sessionNumber = 0;
#define X(name, shortstr, longstr) \
        if (parse_string(desc, shortstr, &sessionNumber) || \
            parse_string(desc, longstr, &sessionNumber)) \
        { \
            type_ = name; \
            if (sessionNumber > 0) \
                number_ = SessionNumber::fromValue(sessionNumber); \
            goto success; \
        }
    ROUND_TYPES_LIST
#undef X
    success:

    if (type_ == FREE)
    {
        sessionNumber = atoi(desc);
        if (sessionNumber > 0)
            number_ = SessionNumber::fromValue(sessionNumber);
    }
}

// ----------------------------------------------------------------------------
Round::~Round()
{}

// ----------------------------------------------------------------------------
Round Round::makeFree()
{
    PROFILE(Round, makeFree);

    return Round(FREE, SessionNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
Round Round::fromType(Type type, SessionNumber number)
{
    PROFILE(Round, fromType);

    switch (type)
    {
        case WINNERS_ROUND:
        case LOSERS_ROUND:
        case POOLS:
        case FREE:
            return Round(type, number);

        case WINNERS_QUARTER:
        case WINNERS_SEMI:
        case WINNERS_FINALS:
        case LOSERS_QUARTER:
        case LOSERS_SEMI:
        case LOSERS_FINALS:
        case GRAND_FINALS:
            break;
    }

    return Round(type, rfcommon::SessionNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
Round Round::fromIndex(int index, SessionNumber number)
{
    PROFILE(Round, fromIndex);

    return Round(static_cast<Type>(index), number);
}

// ----------------------------------------------------------------------------
Round Round::fromSessionNumber(SessionNumber sessionNumber)
{
    PROFILE(Round, fromSessionNumber);

    return Round(FREE, sessionNumber);
}

// ----------------------------------------------------------------------------
Round Round::fromDescription(const char* desc)
{
    PROFILE(Round, fromDescription);

    return Round(desc);
}

// ----------------------------------------------------------------------------
String Round::shortDescription() const
{
    PROFILE(Round, shortDescription);

    static const char* table[] = {
#define X(name, shortstr, longstr) shortstr,
        ROUND_TYPES_LIST
#undef X
    };

    switch (type_)
    {
        case WINNERS_ROUND:
        case LOSERS_ROUND:
            return table[type_] + String::decimal(number_.value());

        case POOLS:
            return String(table[type_]) + " " + String::decimal(number_.value());

        case WINNERS_QUARTER:
        case WINNERS_SEMI:
        case WINNERS_FINALS:
        case LOSERS_QUARTER:
        case LOSERS_SEMI:
        case LOSERS_FINALS:
        case GRAND_FINALS:
            return table[type_];

        case FREE:
            return String::decimal(number_.value());
    }

    return "";
}

// ----------------------------------------------------------------------------
String Round::longDescription() const
{
    PROFILE(Round, longDescription);

    static const char* table[] = {
#define X(name, shortstr, longstr) longstr,
        ROUND_TYPES_LIST
#undef X
    };

    switch (type_)
    {
        case WINNERS_ROUND:
        case LOSERS_ROUND:
        case POOLS:
        case FREE:
            return String(table[type_]) + " " + String::decimal(number_.value());

        case WINNERS_QUARTER:
        case WINNERS_SEMI:
        case WINNERS_FINALS:
        case LOSERS_QUARTER:
        case LOSERS_SEMI:
        case LOSERS_FINALS:
        case GRAND_FINALS:
            return table[type_];
    }

    return "";
}

// ----------------------------------------------------------------------------
bool Round::operator==(const Round& rhs) const
{
    return type_ == rhs.type_ && number_ == rhs.number_;
}

// ----------------------------------------------------------------------------
bool Round::operator!=(const Round& rhs) const
{
    return !operator==(rhs);
}

}
