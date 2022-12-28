#pragma once

#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfapp {

class PlayerDetails
{
public:
    struct Player
    {
        rfcommon::String name;
        rfcommon::String sponsor;
        rfcommon::String social;
        rfcommon::String pronouns;
    };

    PlayerDetails();
    ~PlayerDetails();

    void load();
    void save();

    const Player* findTag(const rfcommon::String& tag) const;
    const Player* findName(const rfcommon::String& name) const;
    void addOrModifyPlayer(const char* tag,
            const char* name,
            const char* sponsor,
            const char* social,
            const char* pronouns);

private:
    rfcommon::HashMap<rfcommon::String, Player> players_;
};

}
