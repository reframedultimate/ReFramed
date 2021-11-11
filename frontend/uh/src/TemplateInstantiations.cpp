#include "uh/Types.hpp"
#include "uh/String.hpp"
#include "uh/Vector.hpp"
#include "uh/HashMap.hpp"
#include "uh/LinearMap.hpp"
#include "uh/Reference.hpp"
#include "uh/ListenerDispatcher.hpp"

#include "uh/DataPoint.hpp"
#include "uh/DataSetFilter.hpp"
#include "uh/DataSetFilterListener.hpp"
#include "uh/DataSetProcessorListener.hpp"
#include "uh/PlayerState.hpp"
#include "uh/SavedGameSession.hpp"

namespace uh {

template class HashMap<FighterID, String>;
template class HashMap<FighterStatus, String>;
template class HashMap<FighterID, HashMap<FighterStatus, String>>;
template class ListenerDispatcher<DataSetFilterListener>;
template class ListenerDispatcher<DataSetProcessorListener>;
template class Reference<DataSetFilter>;
template class Reference<GameSession>;
template class SmallLinearMap<FighterHitStatus, String, 6>;
template class SmallLinearMap<StageID, String, 10>;
template class SmallString<0>;
template class SmallVector<DataSetFilterListener*, 4>;
template class SmallVector<DataSetProcessorListener*, 4>;
template class SmallVector<FighterID, 8>;
template class SmallVector<FighterHitStatus, 6>;
template class SmallVector<SessionListener*, 4>;
template class SmallVector<Reference<DataSetFilter>, 8>;
template class SmallVector<SmallString<15>, 8>;
template class SmallVector<StageID, 10>;
template class SmallVector<String, 6>;
template class SmallVector<String, 10>;
template class SmallVector<Vector<PlayerState>, 8>;
template class Vector<uint32_t>;
template class Vector<DataPoint>;
template class Vector<char>;

}
