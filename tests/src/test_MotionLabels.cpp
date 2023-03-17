#include "gmock/gmock.h"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/hash40.hpp"

#define NAME MotionLabels

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override
    {
        // hash40() doesn't work until rfcommon::init() is called
        anair = rfcommon::hash40("attack_air_n");
        lnair = rfcommon::hash40("landing_air_n");
        adair = rfcommon::hash40("attack_air_lw");
        ldair = rfcommon::hash40("landing_air_lw");
        qa1 = rfcommon::hash40("special_hi_start");
    }

protected:
    rfcommon::FighterMotion anair = rfcommon::FighterMotion::makeInvalid();
    rfcommon::FighterMotion lnair = rfcommon::FighterMotion::makeInvalid();
    rfcommon::FighterMotion adair = rfcommon::FighterMotion::makeInvalid();
    rfcommon::FighterMotion ldair = rfcommon::FighterMotion::makeInvalid();
    rfcommon::FighterMotion qa1 = rfcommon::FighterMotion::makeInvalid();

    rfcommon::FighterID pika = rfcommon::FighterID::fromValue(8);
};

TEST_F(NAME, load_hash40)
{
    rfcommon::MotionLabels ml;
    EXPECT_THAT(ml.lookupHash40(anair), IsNull());
    EXPECT_THAT(ml.lookupHash40(lnair), IsNull());
    EXPECT_THAT(ml.lookupHash40(qa1), IsNull());

    ASSERT_THAT(ml.updateHash40FromCSV("share/reframed/data/tests/hash40_1.csv"), IsTrue());
    EXPECT_THAT(ml.lookupHash40(anair), StrEq("attack_air_n"));
    EXPECT_THAT(ml.lookupHash40(lnair), StrEq("landing_air_n"));
    EXPECT_THAT(ml.lookupHash40(qa1), IsNull());

    ASSERT_THAT(ml.updateHash40FromCSV("share/reframed/data/tests/hash40_2.csv"), IsTrue());
    EXPECT_THAT(ml.lookupHash40(anair), StrEq("attack_air_n"));
    EXPECT_THAT(ml.lookupHash40(lnair), StrEq("landing_air_n"));
    EXPECT_THAT(ml.lookupHash40(qa1), StrEq("special_hi_start"));
}

TEST_F(NAME, lookup_hash40_motion)
{
    rfcommon::MotionLabels ml;
    ASSERT_THAT(ml.updateHash40FromCSV("share/reframed/data/tests/hash40_1.csv"), IsTrue());
    EXPECT_THAT(ml.toMotion("attack_air_n"), Eq(anair));
    EXPECT_THAT(ml.toMotion("special_hi_start"), Eq(rfcommon::FighterMotion::makeInvalid()));
}

TEST_F(NAME, save_load_binary_hash40s)
{
    rfcommon::MotionLabels ml1, ml2;
    ASSERT_THAT(ml1.updateHash40FromCSV("share/reframed/data/tests/hash40_2.csv"), IsTrue());
    ASSERT_THAT(ml1.save("save_load_binary_hash40s.dat"), Eq(true));

    ASSERT_THAT(ml2.load("save_load_binary_hash40s.dat"), Eq(true));
    EXPECT_THAT(ml2.lookupHash40(anair), StrEq("attack_air_n"));
    EXPECT_THAT(ml2.lookupHash40(lnair), StrEq("landing_air_n"));
    EXPECT_THAT(ml2.lookupHash40(qa1), StrEq("special_hi_start"));
}

TEST_F(NAME, create_layers_and_add_labels)
{
    using U = rfcommon::MotionLabels::Usage;
    using C = rfcommon::MotionLabels::Category;
    using Vec = rfcommon::SmallVector<rfcommon::FighterMotion, 4>;
    rfcommon::MotionLabels ml;

    ASSERT_THAT(ml.newLayer("specific", U::NOTATION), Eq(0));
    EXPECT_THAT(ml.addNewLabel(pika, anair, C::AERIAL_ATTACKS, 0, "anair"), Eq(0));
    EXPECT_THAT(ml.addNewLabel(pika, lnair, C::AERIAL_ATTACKS, 0, "lnair"), Eq(1));
    EXPECT_THAT(ml.addNewLabel(pika, adair, C::AERIAL_ATTACKS, 0, "adair"), Eq(2));
    EXPECT_THAT(ml.addNewLabel(pika, ldair, C::AERIAL_ATTACKS, 0, "ldair"), Eq(3));

    ASSERT_THAT(ml.newLayer("general", U::NOTATION), Eq(1));
    EXPECT_THAT(ml.addNewLabel(pika, anair, C::AERIAL_ATTACKS, 1, "nair"), Eq(0));
    EXPECT_THAT(ml.addNewLabel(pika, lnair, C::AERIAL_ATTACKS, 1, "nair"), Eq(1));
    EXPECT_THAT(ml.addNewLabel(pika, adair, C::AERIAL_ATTACKS, 1, "fair"), Eq(2));  // wrong on purpose
    EXPECT_THAT(ml.addNewLabel(pika, ldair, C::AERIAL_ATTACKS, 1, "fair"), Eq(3));  // wrong on purpose

    EXPECT_THAT(ml.addNewLabel(pika, adair, C::AERIAL_ATTACKS, 1, "dair"), Eq(-1));
    EXPECT_THAT(ml.addNewLabel(pika, ldair, C::AERIAL_ATTACKS, 1, "dair"), Eq(-1));

    ml.changeLabel(pika, 2, 1, "dair");
    ml.changeLabel(pika, 3, 1, "dair");

    EXPECT_THAT(ml.lookupLayer(pika, anair, 0), StrEq("anair"));
    EXPECT_THAT(ml.lookupLayer(pika, lnair, 0), StrEq("lnair"));
    EXPECT_THAT(ml.lookupLayer(pika, adair, 0), StrEq("adair"));
    EXPECT_THAT(ml.lookupLayer(pika, ldair, 0), StrEq("ldair"));

    EXPECT_THAT(ml.lookupLayer(pika, anair, 1), StrEq("nair"));
    EXPECT_THAT(ml.lookupLayer(pika, lnair, 1), StrEq("nair"));
    EXPECT_THAT(ml.lookupLayer(pika, adair, 1), StrEq("dair"));
    EXPECT_THAT(ml.lookupLayer(pika, ldair, 1), StrEq("dair"));

    EXPECT_THAT(ml.toMotions(pika, "anair"), Eq(Vec{anair}));
    EXPECT_THAT(ml.toMotions(pika, "lnair"), Eq(Vec{lnair}));
    EXPECT_THAT(ml.toMotions(pika, "adair"), Eq(Vec{adair}));
    EXPECT_THAT(ml.toMotions(pika, "ldair"), Eq(Vec{ldair}));

    EXPECT_THAT(ml.toMotions(pika, "nair"), Eq(Vec{anair, lnair}));
    EXPECT_THAT(ml.toMotions(pika, "dair"), Eq(Vec{adair, ldair}));
}

TEST_F(NAME, save_and_load_layers_binary)
{
    using U = rfcommon::MotionLabels::Usage;
    using C = rfcommon::MotionLabels::Category;
    using Vec = rfcommon::SmallVector<rfcommon::FighterMotion, 4>;
    rfcommon::MotionLabels ml1, ml2;

    ASSERT_THAT(ml1.newLayer("specific", U::NOTATION), Eq(0));
    EXPECT_THAT(ml1.addNewLabel(pika, anair, C::AERIAL_ATTACKS, 0, "anair"), Eq(0));
    //EXPECT_THAT(ml1.addNewLabel(pika, lnair, C::AERIAL_ATTACKS, 0, "lnair"), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, adair, C::AERIAL_ATTACKS, 0, "adair"), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, ldair, C::AERIAL_ATTACKS, 0, "ldair"), Eq(2));

    ASSERT_THAT(ml1.newLayer("general", U::NOTATION), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, anair, C::AERIAL_ATTACKS, 1, "nair"), Eq(0));
    EXPECT_THAT(ml1.addNewLabel(pika, lnair, C::AERIAL_ATTACKS, 1, "nair"), Eq(3));
    EXPECT_THAT(ml1.addNewLabel(pika, adair, C::AERIAL_ATTACKS, 1, "dair"), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, ldair, C::AERIAL_ATTACKS, 1, "dair"), Eq(2));

    ml1.save("save_and_load_layers_binary.dat");
    ml2.load("save_and_load_layers_binary.dat");

    EXPECT_THAT(ml2.lookupLayer(pika, anair, 0), StrEq("anair"));
    EXPECT_THAT(ml2.lookupLayer(pika, lnair, 0), IsNull());
    EXPECT_THAT(ml2.lookupLayer(pika, adair, 0), StrEq("adair"));
    EXPECT_THAT(ml2.lookupLayer(pika, ldair, 0), StrEq("ldair"));

    EXPECT_THAT(ml2.lookupLayer(pika, anair, 1), StrEq("nair"));
    EXPECT_THAT(ml2.lookupLayer(pika, lnair, 1), StrEq("nair"));
    EXPECT_THAT(ml2.lookupLayer(pika, adair, 1), StrEq("dair"));
    EXPECT_THAT(ml2.lookupLayer(pika, ldair, 1), StrEq("dair"));

    EXPECT_THAT(ml2.toMotions(pika, "anair"), Eq(Vec{anair}));
    EXPECT_THAT(ml2.toMotions(pika, "lnair"), Eq(Vec{}));
    EXPECT_THAT(ml2.toMotions(pika, "adair"), Eq(Vec{adair}));
    EXPECT_THAT(ml2.toMotions(pika, "ldair"), Eq(Vec{ldair}));

    EXPECT_THAT(ml2.toMotions(pika, "nair"), Eq(Vec{anair, lnair}));
    EXPECT_THAT(ml2.toMotions(pika, "dair"), Eq(Vec{adair, ldair}));
}

TEST_F(NAME, export_and_import_layers)
{
    using U = rfcommon::MotionLabels::Usage;
    using C = rfcommon::MotionLabels::Category;
    using Vec = rfcommon::SmallVector<rfcommon::FighterMotion, 4>;
    rfcommon::MotionLabels ml1, ml2, ml3;

    ASSERT_THAT(ml1.newLayer("specific", U::NOTATION), Eq(0));
    EXPECT_THAT(ml1.addNewLabel(pika, anair, C::AERIAL_ATTACKS, 0, "anair"), Eq(0));
    //EXPECT_THAT(ml1.addNewLabel(pika, lnair, C::AERIAL_ATTACKS, 0, "lnair"), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, adair, C::AERIAL_ATTACKS, 0, "adair"), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, ldair, C::AERIAL_ATTACKS, 0, "ldair"), Eq(2));

    ASSERT_THAT(ml1.newLayer("general", U::NOTATION), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, anair, C::AERIAL_ATTACKS, 1, "nair"), Eq(0));
    EXPECT_THAT(ml1.addNewLabel(pika, lnair, C::AERIAL_ATTACKS, 1, "nair"), Eq(3));
    EXPECT_THAT(ml1.addNewLabel(pika, adair, C::AERIAL_ATTACKS, 1, "dair"), Eq(1));
    EXPECT_THAT(ml1.addNewLabel(pika, ldair, C::AERIAL_ATTACKS, 1, "dair"), Eq(2));

    ml1.exportLayers({0}, "export_and_import_layers_1.json");
    ml1.exportLayers({1}, "export_and_import_layers_2.json");
    ml1.exportLayers({0, 1}, "export_and_import_layers_3.json");

    ml2.importLayers("export_and_import_layers_1.json");
    ASSERT_THAT(ml2.layerCount(), Eq(1));
    EXPECT_THAT(ml2.lookupLayer(pika, anair, 0), StrEq("anair"));
    EXPECT_THAT(ml2.lookupLayer(pika, lnair, 0), IsNull());
    EXPECT_THAT(ml2.lookupLayer(pika, adair, 0), StrEq("adair"));
    EXPECT_THAT(ml2.lookupLayer(pika, ldair, 0), StrEq("ldair"));

    EXPECT_THAT(ml2.toMotions(pika, "anair"), Eq(Vec{anair}));
    EXPECT_THAT(ml2.toMotions(pika, "lnair"), Eq(Vec{}));
    EXPECT_THAT(ml2.toMotions(pika, "adair"), Eq(Vec{adair}));
    EXPECT_THAT(ml2.toMotions(pika, "ldair"), Eq(Vec{ldair}));

    EXPECT_THAT(ml2.toMotions(pika, "nair"), Eq(Vec{}));
    EXPECT_THAT(ml2.toMotions(pika, "dair"), Eq(Vec{}));

    ml2.importLayers("export_and_import_layers_2.json");
    ASSERT_THAT(ml2.layerCount(), Eq(2));
    EXPECT_THAT(ml2.lookupLayer(pika, anair, 0), StrEq("anair"));
    EXPECT_THAT(ml2.lookupLayer(pika, lnair, 0), IsNull());
    EXPECT_THAT(ml2.lookupLayer(pika, adair, 0), StrEq("adair"));
    EXPECT_THAT(ml2.lookupLayer(pika, ldair, 0), StrEq("ldair"));

    EXPECT_THAT(ml2.lookupLayer(pika, anair, 1), StrEq("nair"));
    EXPECT_THAT(ml2.lookupLayer(pika, lnair, 1), StrEq("nair"));
    EXPECT_THAT(ml2.lookupLayer(pika, adair, 1), StrEq("dair"));
    EXPECT_THAT(ml2.lookupLayer(pika, ldair, 1), StrEq("dair"));

    EXPECT_THAT(ml2.toMotions(pika, "anair"), Eq(Vec{anair}));
    EXPECT_THAT(ml2.toMotions(pika, "lnair"), Eq(Vec{}));
    EXPECT_THAT(ml2.toMotions(pika, "adair"), Eq(Vec{adair}));
    EXPECT_THAT(ml2.toMotions(pika, "ldair"), Eq(Vec{ldair}));

    EXPECT_THAT(ml2.toMotions(pika, "nair"), Eq(Vec{anair, lnair}));
    EXPECT_THAT(ml2.toMotions(pika, "dair"), Eq(Vec{adair, ldair}));

    ml3.importLayers("export_and_import_layers_3.json");
    ASSERT_THAT(ml3.layerCount(), Eq(2));
    EXPECT_THAT(ml3.lookupLayer(pika, anair, 0), StrEq("anair"));
    EXPECT_THAT(ml3.lookupLayer(pika, lnair, 0), IsNull());
    EXPECT_THAT(ml3.lookupLayer(pika, adair, 0), StrEq("adair"));
    EXPECT_THAT(ml3.lookupLayer(pika, ldair, 0), StrEq("ldair"));

    EXPECT_THAT(ml3.lookupLayer(pika, anair, 1), StrEq("nair"));
    EXPECT_THAT(ml3.lookupLayer(pika, lnair, 1), StrEq("nair"));
    EXPECT_THAT(ml3.lookupLayer(pika, adair, 1), StrEq("dair"));
    EXPECT_THAT(ml3.lookupLayer(pika, ldair, 1), StrEq("dair"));

    EXPECT_THAT(ml3.toMotions(pika, "anair"), Eq(Vec{anair}));
    EXPECT_THAT(ml3.toMotions(pika, "lnair"), Eq(Vec{}));
    EXPECT_THAT(ml3.toMotions(pika, "adair"), Eq(Vec{adair}));
    EXPECT_THAT(ml3.toMotions(pika, "ldair"), Eq(Vec{ldair}));

    EXPECT_THAT(ml3.toMotions(pika, "nair"), Eq(Vec{anair, lnair}));
    EXPECT_THAT(ml3.toMotions(pika, "dair"), Eq(Vec{adair, ldair}));
}
