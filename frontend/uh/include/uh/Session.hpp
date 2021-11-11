#pragma once

#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/MappingInfo.hpp"
#include "uh/RefCounted.hpp"

namespace uh {

class PlayerState;
class SessionListener;

extern template class UH_TEMPLATE_API SmallVector<SmallString<15>, 8>;
extern template class UH_TEMPLATE_API SmallVector<FighterID, 8>;

class UH_PUBLIC_API Session : public RefCounted
{
public:
    Session(MappingInfo&& mapping,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags,
            StageID stageID);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    const MappingInfo& mappingInfo() const { return mappingInfo_; }

    /*!
     * \brief Gets the number of players
     */
    int playerCount() const { return static_cast<int>(playerTags_.count()); }

    /*!
     * \brief Gets the tag used by the player. This is the string that appears
     * above the player in-game and is created when the player sets their controls.
     * \param index Which player to get
     */
    const SmallString<15>& playerTag(int index) const { return playerTags_[index]; }

    /*!
     * \brief Gets the fighter ID being used by the specified player.
     * \param index The player to get
     */
    FighterID playerFighterID(int index) const { return playerFighterIDs_[index]; }

    /*!
     * \brief Gets the stage ID being played on.
     */
    StageID stageID() const { return stageID_; }

protected:
    MappingInfo mappingInfo_;
    SmallVector<FighterID, 8> playerFighterIDs_;
    SmallVector<SmallString<15>, 8> playerTags_;
    StageID stageID_;
};

}
