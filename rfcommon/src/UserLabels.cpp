#include "rfcommon/UserLabels.hpp"

namespace rfcommon {

FighterUserMotionLabels::FighterUserMotionLabels()
{}

FighterUserMotionLabels::~FighterUserMotionLabels()
{}

UserMotionLabels::UserMotionLabels(Hash40Strings* hash40Strings)
    : hash40Strings_(hash40Strings)
{}

UserMotionLabels::~UserMotionLabels()
{}

}
