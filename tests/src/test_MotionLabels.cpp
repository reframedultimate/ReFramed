#include "gmock/gmock.h"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/hash40.hpp"

#define NAME MotionLabels

using namespace testing;
using namespace rfcommon;

const FighterMotion anair = hash40("attack_air_n");
const FighterMotion lnair = hash40("landing_air_n");
const FighterMotion adair = hash40("attack_air_lw");
const FighterMotion ldair = hash40("landing_air_lw");

TEST(NAME, load_hash40)
{
	MotionLabels ml;
	EXPECT_THAT(ml.lookupHash40(anair), IsNull());
	EXPECT_THAT(ml.lookupHash40(lnair), IsNull());

	ASSERT_THAT(ml.updateHash40FromCSV("share/reframed/data/tests/hash40_1.csv"), IsTrue());
}
