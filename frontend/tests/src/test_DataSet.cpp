#include "gmock/gmock.h"
#include "uh/ActiveRecording.hpp"
#include "uh/DataSet.hpp"
#include "uh/DataPoint.hpp"
#include "uh/MappingInfo.hpp"
#include "uh/PlayerState.hpp"
#include "uh/Reference.hpp"

#define NAME DataSet

class NAME : public testing::Test
{
public:
    void SetUp() override
    {
        recording = new uh::ActiveRecording(uh::MappingInfo(), {1, 2}, {"P1", "P2"}, 0);
        recording->addPlayerState(0, uh::PlayerState(1000/60, 100, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, false, false));
        recording->addPlayerState(0, uh::PlayerState(2000/60,  99, 0.0, 0.0, 0.0, 0.0, 0.0, 1, 1, 0, 0, false, false));
        recording->addPlayerState(0, uh::PlayerState(3000/60,  98, 0.0, 0.0, 0.0, 0.0, 0.0, 2, 2, 0, 0, false, false));
        recording->addPlayerState(0, uh::PlayerState(4000/60,  97, 0.0, 0.0, 0.0, 0.0, 0.0, 3, 3, 0, 0, false, false));
        recording->addPlayerState(0, uh::PlayerState(5000/60,  96, 0.0, 0.0, 0.0, 0.0, 0.0, 4, 4, 0, 0, false, false));
        recording->addPlayerState(0, uh::PlayerState(6000/60,  95, 0.0, 0.0, 0.0, 0.0, 0.0, 5, 5, 0, 0, false, false));
    }

    void TearDown() override
    {
    }

    uh::Reference<uh::ActiveRecording> recording;
};

using namespace testing;

TEST_F(NAME, append_single_state)
{
    uh::Reference<uh::DataSet> ds = new uh::DataSet;
    ds->appendPlayerState(recording, recording->playerState(0, 3));
    ASSERT_THAT(ds->begin(), NotNull());
    ASSERT_THAT(ds->count(), Eq(1));
    EXPECT_THAT(ds->begin()->recording(), Eq(recording));
    EXPECT_THAT(ds->begin()->state(), Eq(recording->playerState(0, 3)));
}

TEST_F(NAME, appending_multiple_single_states_inserts_in_order_of_timestamps)
{
    uh::Reference<uh::DataSet> ds = new uh::DataSet;
    ds->appendPlayerState(recording, recording->playerState(0, 3));
    ds->appendPlayerState(recording, recording->playerState(0, 0));
    ds->appendPlayerState(recording, recording->playerState(0, 5));
    ds->appendPlayerState(recording, recording->playerState(0, 1));
    ASSERT_THAT(ds->begin(), NotNull());
    ASSERT_THAT(ds->count(), Eq(4));
    EXPECT_THAT((ds->begin()+0)->recording(), Eq(recording));
    EXPECT_THAT((ds->begin()+0)->state(), Eq(recording->playerState(0, 0)));
    EXPECT_THAT((ds->begin()+1)->recording(), Eq(recording));
    EXPECT_THAT((ds->begin()+1)->state(), Eq(recording->playerState(0, 1)));
    EXPECT_THAT((ds->begin()+2)->recording(), Eq(recording));
    EXPECT_THAT((ds->begin()+2)->state(), Eq(recording->playerState(0, 3)));
    EXPECT_THAT((ds->begin()+3)->recording(), Eq(recording));
    EXPECT_THAT((ds->begin()+3)->state(), Eq(recording->playerState(0, 5)));
}

TEST_F(NAME, erasing_datapoints_preserves_order)
{
    uh::Reference<uh::DataSet> ds = new uh::DataSet;
    ds->appendPlayerState(recording, recording->playerState(0, 3));
    ds->appendPlayerState(recording, recording->playerState(0, 0));
    ds->appendPlayerState(recording, recording->playerState(0, 5));
    ds->appendPlayerState(recording, recording->playerState(0, 1));
    ds->erase(ds->begin() + 3);
    ds->erase(ds->begin() + 0);
    ASSERT_THAT(ds->begin(), NotNull());
    ASSERT_THAT(ds->count(), Eq(2));
    EXPECT_THAT((ds->begin()+0)->recording(), Eq(recording));
    EXPECT_THAT((ds->begin()+0)->state(), Eq(recording->playerState(0, 1)));
    EXPECT_THAT((ds->begin()+1)->recording(), Eq(recording));
    EXPECT_THAT((ds->begin()+1)->state(), Eq(recording->playerState(0, 3)));
}

TEST_F(NAME, append_states_from_recording)
{
    uh::Reference<uh::DataSet> ds = new uh::DataSet;
    ds->appendPlayerStatesFromRecording(0, recording);
    ASSERT_THAT(ds->begin(), NotNull());
    ASSERT_THAT(ds->count(), Eq(6));
    for (int i = 0; i != 6; ++i)
    {
        EXPECT_THAT((ds->begin()+i)->recording(), Eq(recording));
        EXPECT_THAT((ds->begin()+i)->state(), Eq(recording->playerState(0, i)));
    }
}
