#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

extern template class RFCOMMON_TEMPLATE_API Vector<uint32_t>;
extern template class RFCOMMON_TEMPLATE_API HashMap<FighterStatus, String>;
extern template class RFCOMMON_TEMPLATE_API HashMap<FighterID, HashMap<FighterStatus, String>>;

/*!
 * \brief Character animation states are all described with a single u16 integer
 * value. This class stores information for mapping a state value to a name.
 *
 * Mappings are not unique among all fighters. There are a base set of enum
 * values that match the pattern
 *
 *     FIGHTER_STATUS_KIND_xxx
 *
 * and then there is a character specific set that matches the pattern
 *
 *     FIGHTER_CHARNAME_STATUS_KIND_xxx
 *
 * The character specific sets share values among each other, so it's not
 * possible to simply map int <-> name.
 *
 * We also have multiple layers of names to go through. The first mapping is
 * integer to the enum name. The second layer is a mapping from enum name to
 * a more readable name such as "utilt" or "fsmash". The final layer is a
 * mapping to user defined names, as this application lets the user give their
 * own names to states.
 */
class RFCOMMON_PUBLIC_API FighterStatusMapping
{
public:
    const String* statusToBaseEnumName(FighterStatus status) const;
    const String* statusToFighterSpecificEnumName(FighterStatus status, FighterID fighterID) const;

    void addBaseEnumName(FighterStatus status, const String& name);
    void addFighterSpecificEnumName(FighterStatus status, FighterID fighterID, const String& name);

    const HashMap<FighterStatus, String>& baseEnumNames() const { return baseEnumNames_; }
    const HashMap<FighterID, HashMap<FighterStatus, String>>& fighterSpecificEnumNames() const { return fighterSpecificEnumNames_; }

private:
    HashMap<FighterStatus, String> baseEnumNames_;
    HashMap<FighterID, HashMap<FighterStatus, String>> fighterSpecificEnumNames_;
};

}
