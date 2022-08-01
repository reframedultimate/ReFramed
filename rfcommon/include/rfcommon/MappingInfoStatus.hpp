#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API MappingInfoStatus
{
public:
    MappingInfoStatus();
    ~MappingInfoStatus();

    //const char* statusString(FighterStatus status) const;
    const char* toName(FighterID fighterID, FighterStatus status) const;
    FighterStatus toStatus(const char* enumName) const;

    void addBaseName(FighterStatus status, const char* name);
    void addSpecificName(FighterID fighterID, FighterStatus status, const char* name);

    Vector<FighterID> fighterIDs() const;

    Vector<SmallString<31>> baseNames() const;
    Vector<FighterStatus> baseStatuses() const;

    Vector<SmallString<31>> specificNames(FighterID fighterID) const;
    Vector<FighterStatus> specificStatuses(FighterID fighterID) const;

private:
    HashMap<FighterStatus, SmallString<31>, FighterStatus::Hasher> statusMap_;
    HashMap<SmallString<31>, FighterStatus> enumNameMap_;
    HashMap<FighterID, HashMap<FighterStatus, SmallString<31>, FighterStatus::Hasher>, FighterID::Hasher> specificStatusMap_;
};

}
