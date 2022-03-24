#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/LinearMap.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

#include "rfcommon/DataSetFilter.hpp"
#include "rfcommon/DataSetFilterListener.hpp"
#include "rfcommon/DataSetProcessorListener.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/SavedGameSession.hpp"

namespace rfcommon {

template class HashMap<FighterID, String, FighterIDHasher>;
template class HashMap<FighterStatus, String, FighterStatusHasher>;
template class HashMap<FighterID, HashMap<FighterStatus, String, FighterStatusHasher>, FighterIDHasher>;
template class ListenerDispatcher<DataSetFilterListener>;
template class ListenerDispatcher<DataSetProcessorListener>;
template class ListenerDispatcher<SessionListener>;
template class Reference<DataSetFilter>;
template class Reference<GameSession>;
template class Reference<SavedSession>;
template class Reference<SavedGameSession>;
template class SmallLinearMap<FighterHitStatus, String, 6>;
template class SmallLinearMap<StageID, String, 10>;
template class SmallString<15>;
template class SmallVector<DataSetFilterListener*, 4>;
template class SmallVector<DataSetProcessorListener*, 4>;
template class SmallVector<FighterID, 2>;
template class SmallVector<FighterHitStatus, 6>;
template class SmallVector<SessionListener*, 4>;
template class SmallVector<Reference<DataSetFilter>, 8>;
template class SmallVector<SmallString<15>, 2>;
template class SmallVector<StageID, 10>;
template class SmallVector<String, 6>;
template class SmallVector<String, 10>;
template class SmallVector<Vector<FighterState>, 2>;
template class Vector<uint32_t>;
template class Vector<char>;

}
