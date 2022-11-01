#include "gmock/gmock.h"
#include "rfcommon/ReplayFileParts.hpp"

#define NAME replay_file_parts

using namespace rfcommon;
using namespace testing;

TEST(NAME, datetime_parsing)
{
    auto p1 = ReplayFileParts::fromFileName("202");
    auto p2 = ReplayFileParts::fromFileName("2022-");
    auto p3 = ReplayFileParts::fromFileName("2022-09-11");
    auto p4 = ReplayFileParts::fromFileName("2022-09-11_");
    auto p5 = ReplayFileParts::fromFileName("2022-09-11_19");
    auto p6 = ReplayFileParts::fromFileName("2022-09-11_19-");
    auto p7 = ReplayFileParts::fromFileName("2022-09-11_19-45-11");
    auto p8 = ReplayFileParts::fromFileName("2022-09-11_19-45-11 - ");

    EXPECT_THAT(p1.date().cStr(), StrEq(""));
    EXPECT_THAT(p1.time().cStr(), StrEq(""));

    EXPECT_THAT(p2.date().cStr(), StrEq(""));
    EXPECT_THAT(p2.time().cStr(), StrEq(""));

    EXPECT_THAT(p3.date().cStr(), StrEq("2022-09-11"));
    EXPECT_THAT(p3.time().cStr(), StrEq(""));

    EXPECT_THAT(p4.date().cStr(), StrEq("2022-09-11"));
    EXPECT_THAT(p4.time().cStr(), StrEq(""));

    EXPECT_THAT(p5.date().cStr(), StrEq("2022-09-11"));
    EXPECT_THAT(p5.time().cStr(), StrEq(""));

    EXPECT_THAT(p6.date().cStr(), StrEq("2022-09-11"));
    EXPECT_THAT(p6.time().cStr(), StrEq(""));

    EXPECT_THAT(p7.date().cStr(), StrEq("2022-09-11"));
    EXPECT_THAT(p7.time().cStr(), StrEq("19:45:11"));

    EXPECT_THAT(p8.date().cStr(), StrEq("2022-09-11"));
    EXPECT_THAT(p8.time().cStr(), StrEq("19:45:11"));
}

TEST(NAME, set_format_parsing)
{
    auto p1 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (1) - ");
    auto p2 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (226) - ");
    auto p3 = ReplayFileParts::fromFileName("2022-09-11 - Bo5(42) - ");
    auto p4 = ReplayFileParts::fromFileName("2022-09-11 -Bo5(82)- ");
    auto p5 = ReplayFileParts::fromFileName("2022-09-11 -Bo5(82- ");
    auto p6 = ReplayFileParts::fromFileName("2022-09-11 -Bo5 82- ");
    auto p7 = ReplayFileParts::fromFileName("2022-09-11 - TheComet (Pika) vs ");

    EXPECT_THAT(p1.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p1.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p1.setNumber().value(), Eq(1));

    EXPECT_THAT(p2.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p2.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p2.setNumber().value(), Eq(226));

    EXPECT_THAT(p3.setFormat().type(), Eq(SetFormat::BO5));
    EXPECT_THAT(p3.setFormat().shortDescription(), StrEq("Bo5"));
    EXPECT_THAT(p3.setNumber().value(), Eq(42));

    EXPECT_THAT(p4.setFormat().type(), Eq(SetFormat::BO5));
    EXPECT_THAT(p4.setFormat().shortDescription(), StrEq("Bo5"));
    EXPECT_THAT(p4.setNumber().value(), Eq(82));

    EXPECT_THAT(p5.setFormat().type(), Eq(SetFormat::OTHER));
    EXPECT_THAT(p5.setFormat().shortDescription(), StrEq(""));
    EXPECT_THAT(p5.setNumber().value(), Eq(1));

    EXPECT_THAT(p6.setFormat().type(), Eq(SetFormat::OTHER));
    EXPECT_THAT(p6.setFormat().shortDescription(), StrEq(""));
    EXPECT_THAT(p6.setNumber().value(), Eq(1));

    EXPECT_THAT(p7.setFormat().type(), Eq(SetFormat::OTHER));
    EXPECT_THAT(p7.setFormat().shortDescription(), StrEq(""));
    EXPECT_THAT(p7.setNumber().value(), Eq(1));
}

TEST(NAME, player_parsing)
{
    auto p1 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (1) - TheComet (Pika)");
    auto p2 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (1) - Dumb. Ass (Pika) vs TAEL (C. Falcon)");
    auto p3 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (1) - Dumb. Ass (Pika) vs TAEL (C. Falcon) - ");
    auto p4 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (1) - Dumb. Ass (Pika");
    auto p5 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (1) - Dumb. Ass ((unknown fighter))");

    ASSERT_THAT(p1.playerCount(), Eq(1));
    EXPECT_THAT(p1.playerName(0).cStr(), StrEq("TheComet"));
    EXPECT_THAT(p1.characterName(0).cStr(), StrEq("Pika"));

    ASSERT_THAT(p2.playerCount(), Eq(2));
    EXPECT_THAT(p2.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p2.characterName(0).cStr(), StrEq("Pika"));
    EXPECT_THAT(p2.playerName(1).cStr(), StrEq("TAEL"));
    EXPECT_THAT(p2.characterName(1).cStr(), StrEq("C. Falcon"));

    ASSERT_THAT(p3.playerCount(), Eq(2));
    EXPECT_THAT(p3.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p3.characterName(0).cStr(), StrEq("Pika"));
    EXPECT_THAT(p3.playerName(1).cStr(), StrEq("TAEL"));
    EXPECT_THAT(p3.characterName(1).cStr(), StrEq("C. Falcon"));

    ASSERT_THAT(p4.playerCount(), Eq(0));

    ASSERT_THAT(p5.playerCount(), Eq(1));
    EXPECT_THAT(p5.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p5.characterName(0).cStr(), StrEq("unknown fighter"));
}

TEST(NAME, parse_all)
{
    auto p1 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (43) - Dumb. Ass (K. Rool) vs LM  AOO  AUU (C. Falcon) Game 232.rfr");
    auto p2 = ReplayFileParts::fromFileName("2022-09-11 - Bo3 (78) - Dumb. Ass (K. Rool) vs LM  AOO  AUU (C. Falcon) - Game 323 - Kalos.rfr");
    auto p3 = ReplayFileParts::fromFileName("2022-09-11 - Grind (22) - Dumb. Ass (K. Rool) vs LM  AOO  AUU (C. Falcon) - Game 420 - Town. City.rfr");

    ASSERT_THAT(p1.playerCount(), Eq(2));
    EXPECT_THAT(p1.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p1.characterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p1.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p1.characterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p1.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p1.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p1.setNumber().value(), Eq(43));
    EXPECT_THAT(p1.gameNumber().value(), Eq(232));
    EXPECT_THAT(p1.stage().cStr(), StrEq(""));

    ASSERT_THAT(p2.playerCount(), Eq(2));
    EXPECT_THAT(p2.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p2.characterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p2.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p2.characterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p2.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p2.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p2.setNumber().value(), Eq(78));
    EXPECT_THAT(p2.gameNumber().value(), Eq(323));
    EXPECT_THAT(p2.stage().cStr(), StrEq("Kalos"));

    ASSERT_THAT(p3.playerCount(), Eq(2));
    EXPECT_THAT(p3.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p3.characterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p3.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p3.characterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p3.setFormat().type(), Eq(SetFormat::OTHER));
    EXPECT_THAT(p3.setFormat().shortDescription(), StrEq("Grind"));
    EXPECT_THAT(p3.setNumber().value(), Eq(22));
    EXPECT_THAT(p3.gameNumber().value(), Eq(420));
    EXPECT_THAT(p3.stage().cStr(), StrEq("Town. City"));
}

TEST(NAME, to_string)
{
    ReplayFileParts parts(
            {"TheComet", "TAEL"},
            {"Pikachu", "C. Falcon"},
            "2022-09-11", "19:45:22",
            "Town & City",
            SetNumber::fromValue(2),
            GameNumber::fromValue(3),
            SetFormat::fromType(SetFormat::BO3));

    EXPECT_THAT(parts.toFileName().cStr(), StrEq("2022-09-11_19-45-22 - Bo3 (2) - TheComet (Pikachu) vs TAEL (C. Falcon) - Game 3 - Town & City.rfr"));
}
