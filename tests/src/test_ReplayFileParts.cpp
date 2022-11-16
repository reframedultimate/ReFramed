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

TEST(NAME, event_parsing)
{
    auto p1 = ReplayFileParts::fromFileName("2022-09-11 - Singles Bracket - ");
    auto p2 = ReplayFileParts::fromFileName("2022-09-11 - Practice - ");
    auto p3 = ReplayFileParts::fromFileName("2022-09-11 - Grind - ");
    auto p4 = ReplayFileParts::fromFileName("2022-09-11 - Bo5(42) - ");

    EXPECT_THAT(p1.event().type(), Eq(BracketType::SINGLES));
    EXPECT_THAT(p1.event().description(), StrEq("Singles Bracket"));

    EXPECT_THAT(p2.event().type(), Eq(BracketType::PRACTICE));
    EXPECT_THAT(p2.event().description(), StrEq("Practice"));

    EXPECT_THAT(p3.event().type(), Eq(BracketType::OTHER));
    EXPECT_THAT(p3.event().description(), StrEq("Grind"));

    EXPECT_THAT(p4.event().type(), Eq(BracketType::OTHER));
    EXPECT_THAT(p4.event().description(), StrEq("Bo5(42)"));
}

TEST(NAME, set_format_parsing)
{
    auto p1 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (1) - ");
    auto p2 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (226) - ");
    auto p3 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo5(42) - ");
    auto p4 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies -Bo5(82)- ");
    auto p5 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies -Bo5(82- ");
    auto p6 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies -Bo5 82- ");
    auto p7 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - TheComet (Pika) vs ");

    EXPECT_THAT(p1.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p1.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p1.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p1.round().number().value(), Eq(1));

    EXPECT_THAT(p2.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p2.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p2.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p2.round().number().value(), Eq(226));

    EXPECT_THAT(p3.setFormat().type(), Eq(SetFormat::BO5));
    EXPECT_THAT(p3.setFormat().shortDescription(), StrEq("Bo5"));
    EXPECT_THAT(p3.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p3.round().number().value(), Eq(42));

    EXPECT_THAT(p4.setFormat().type(), Eq(SetFormat::BO5));
    EXPECT_THAT(p4.setFormat().shortDescription(), StrEq("Bo5"));
    EXPECT_THAT(p4.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p4.round().number().value(), Eq(82));

    EXPECT_THAT(p5.setFormat().type(), Eq(SetFormat::FREE));
    EXPECT_THAT(p5.setFormat().shortDescription(), StrEq("Free"));
    EXPECT_THAT(p5.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p5.round().number().value(), Eq(1));

    EXPECT_THAT(p6.setFormat().type(), Eq(SetFormat::FREE));
    EXPECT_THAT(p6.setFormat().shortDescription(), StrEq("Free"));
    EXPECT_THAT(p6.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p6.round().number().value(), Eq(1));

    EXPECT_THAT(p7.setFormat().type(), Eq(SetFormat::FREE));
    EXPECT_THAT(p7.setFormat().shortDescription(), StrEq("Free"));
    EXPECT_THAT(p7.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p7.round().number().value(), Eq(1));
}

TEST(NAME, player_parsing)
{
    auto p1 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (1) - TheComet (Pika)");
    auto p2 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (1) - Dumb. Ass (Pika) vs TAEL (C. Falcon)");
    auto p3 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (1) - Dumb. Ass (Pika) vs TAEL (C. Falcon) - ");
    auto p4 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (1) - Dumb. Ass (Pika");
    auto p5 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (1) - Dumb. Ass ((unknown fighter))");
    auto p6 = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Bo3 (1) - Wave(s) (Sonic)");

    ASSERT_THAT(p1.playerCount(), Eq(1));
    EXPECT_THAT(p1.playerName(0).cStr(), StrEq("TheComet"));
    EXPECT_THAT(p1.fighterName(0).cStr(), StrEq("Pika"));

    ASSERT_THAT(p2.playerCount(), Eq(2));
    EXPECT_THAT(p2.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p2.fighterName(0).cStr(), StrEq("Pika"));
    EXPECT_THAT(p2.playerName(1).cStr(), StrEq("TAEL"));
    EXPECT_THAT(p2.fighterName(1).cStr(), StrEq("C. Falcon"));

    ASSERT_THAT(p3.playerCount(), Eq(2));
    EXPECT_THAT(p3.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p3.fighterName(0).cStr(), StrEq("Pika"));
    EXPECT_THAT(p3.playerName(1).cStr(), StrEq("TAEL"));
    EXPECT_THAT(p3.fighterName(1).cStr(), StrEq("C. Falcon"));

    ASSERT_THAT(p4.playerCount(), Eq(0));

    ASSERT_THAT(p5.playerCount(), Eq(1));
    EXPECT_THAT(p5.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p5.fighterName(0).cStr(), StrEq("unknown fighter"));

    ASSERT_THAT(p6.playerCount(), Eq(1));
    EXPECT_THAT(p6.playerName(0).cStr(), StrEq("Wave(s)"));
    EXPECT_THAT(p6.fighterName(0).cStr(), StrEq("Sonic"));
}

TEST(NAME, parse_all_1)
{
    auto p = ReplayFileParts::fromFileName("2022-09-11 - Singles Bracket - Bo3 (LR4) - Dumb. Ass (K. Rool) vs LM  AOO  AUU (C. Falcon) Game 232.rfr");

    ASSERT_THAT(p.playerCount(), Eq(2));
    EXPECT_THAT(p.event().type(), Eq(BracketType::SINGLES));
    EXPECT_THAT(p.event().description(), StrEq("Singles Bracket"));
    EXPECT_THAT(p.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p.fighterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p.isLoserSide(0), IsFalse());
    EXPECT_THAT(p.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p.fighterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p.isLoserSide(1), IsFalse());
    EXPECT_THAT(p.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p.setFormat().longDescription(), StrEq("Best of 3"));
    EXPECT_THAT(p.round().number().value(), Eq(4));
    EXPECT_THAT(p.round().type(), Eq(Round::LOSERS_ROUND));
    EXPECT_THAT(p.score().gameNumber().value(), Eq(232));
    EXPECT_THAT(p.score().left(), Eq(0));
    EXPECT_THAT(p.score().right(), Eq(0));
    EXPECT_THAT(p.stage().cStr(), StrEq(""));
}

TEST(NAME, parse_all_2)
{
    auto p = ReplayFileParts::fromFileName("2022-09-11 - Singles Bracket - Bo3 (78) - Dumb. Ass (K. Rool) vs LM  AOO  AUU (C. Falcon) - Game 323 - Kalos.rfr");

    ASSERT_THAT(p.playerCount(), Eq(2));
    EXPECT_THAT(p.event().type(), Eq(BracketType::SINGLES));
    EXPECT_THAT(p.event().description(), StrEq("Singles Bracket"));
    EXPECT_THAT(p.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p.fighterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p.isLoserSide(0), IsFalse());
    EXPECT_THAT(p.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p.fighterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p.isLoserSide(1), IsFalse());
    EXPECT_THAT(p.setFormat().type(), Eq(SetFormat::BO3));
    EXPECT_THAT(p.setFormat().shortDescription(), StrEq("Bo3"));
    EXPECT_THAT(p.round().number().value(), Eq(78));
    EXPECT_THAT(p.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p.score().gameNumber().value(), Eq(323));
    EXPECT_THAT(p.score().left(), Eq(0));
    EXPECT_THAT(p.score().right(), Eq(0));
    EXPECT_THAT(p.stage().cStr(), StrEq("Kalos"));
}

TEST(NAME, parse_all_3)
{
    auto p = ReplayFileParts::fromFileName("2022-09-11 - Grind - Free (GF) - Dumb. Ass (K. Rool) [L] vs LM  AOO  AUU (C. Falcon) - Game 420 - Town. City.rfr");

    ASSERT_THAT(p.playerCount(), Eq(2));
    EXPECT_THAT(p.event().type(), Eq(BracketType::OTHER));
    EXPECT_THAT(p.event().description(), StrEq("Grind"));
    EXPECT_THAT(p.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p.fighterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p.isLoserSide(0), IsTrue());
    EXPECT_THAT(p.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p.fighterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p.isLoserSide(1), IsFalse());
    EXPECT_THAT(p.setFormat().type(), Eq(SetFormat::FREE));
    EXPECT_THAT(p.setFormat().shortDescription(), StrEq("Free"));
    EXPECT_THAT(p.setFormat().longDescription(), StrEq("Free Play"));
    EXPECT_THAT(p.round().number().value(), Eq(1));
    EXPECT_THAT(p.round().type(), Eq(Round::GRAND_FINALS));
    EXPECT_THAT(p.score().gameNumber().value(), Eq(420));
    EXPECT_THAT(p.score().left(), Eq(0));
    EXPECT_THAT(p.score().right(), Eq(0));
    EXPECT_THAT(p.stage().cStr(), StrEq("Town. City"));
}

TEST(NAME, parse_all_4)
{
    auto p = ReplayFileParts::fromFileName("2022-09-11 - Practice - FT10 (GF) - Dumb. Ass (K. Rool) [L] vs LM  AOO  AUU (C. Falcon) [L] - Game 69 (5-8) - Town. City.rfr");
    ASSERT_THAT(p.playerCount(), Eq(2));
    EXPECT_THAT(p.event().type(), Eq(BracketType::PRACTICE));
    EXPECT_THAT(p.event().description(), StrEq("Practice"));
    EXPECT_THAT(p.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p.fighterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p.isLoserSide(0), IsTrue());
    EXPECT_THAT(p.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p.fighterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p.isLoserSide(1), IsTrue());
    EXPECT_THAT(p.setFormat().type(), Eq(SetFormat::FT10));
    EXPECT_THAT(p.setFormat().shortDescription(), StrEq("FT10"));
    EXPECT_THAT(p.setFormat().longDescription(), StrEq("First to 10"));
    EXPECT_THAT(p.round().number().value(), Eq(2));
    EXPECT_THAT(p.round().type(), Eq(Round::GRAND_FINALS));
    EXPECT_THAT(p.score().gameNumber().value(), Eq(14));
    EXPECT_THAT(p.score().left(), Eq(5));
    EXPECT_THAT(p.score().right(), Eq(8));
    EXPECT_THAT(p.stage().cStr(), StrEq("Town. City"));
}

TEST(NAME, parse_all_5)
{
    auto p = ReplayFileParts::fromFileName("2022-09-11 - Friendlies - Free (3) - Dumb. Ass (K. Rool) vs LM  AOO  AUU (C. Falcon) - Game 80 (35-44) - Town. City.rfr");

    ASSERT_THAT(p.playerCount(), Eq(2));
    EXPECT_THAT(p.event().type(), Eq(BracketType::FRIENDLIES));
    EXPECT_THAT(p.event().description(), StrEq("Friendlies"));
    EXPECT_THAT(p.playerName(0).cStr(), StrEq("Dumb. Ass"));
    EXPECT_THAT(p.fighterName(0).cStr(), StrEq("K. Rool"));
    EXPECT_THAT(p.isLoserSide(0), IsFalse());
    EXPECT_THAT(p.playerName(1).cStr(), StrEq("LM  AOO  AUU"));
    EXPECT_THAT(p.fighterName(1).cStr(), StrEq("C. Falcon"));
    EXPECT_THAT(p.isLoserSide(1), IsFalse());
    EXPECT_THAT(p.setFormat().type(), Eq(SetFormat::FREE));
    EXPECT_THAT(p.setFormat().shortDescription(), StrEq("Free"));
    EXPECT_THAT(p.setFormat().longDescription(), StrEq("Free Play"));
    EXPECT_THAT(p.round().number().value(), Eq(3));
    EXPECT_THAT(p.round().type(), Eq(Round::FREE));
    EXPECT_THAT(p.score().gameNumber().value(), Eq(80));
    EXPECT_THAT(p.score().left(), Eq(35));
    EXPECT_THAT(p.score().right(), Eq(44));
    EXPECT_THAT(p.stage().cStr(), StrEq("Town. City"));
}

TEST(NAME, to_string_1)
{
    ReplayFileParts parts(
            "",
            {"TheComet", "TAEL"},
            {"Pikachu", "C. Falcon"},
            "2022-09-11", "19:45:22",
            "Town & City",
            BracketType::fromType(BracketType::SINGLES),
            Round::fromType(Round::WINNERS_ROUND, SessionNumber::fromValue(6)),
            SetFormat::fromType(SetFormat::BO3),
            ScoreCount::fromScore(2, 1),
            0x00);

    EXPECT_THAT(parts.toFileName().cStr(), StrEq("2022-09-11_19-45-22 - Singles Bracket - Bo3 (WR6) - TheComet (Pikachu) vs TAEL (C. Falcon) - Game 4 (2-1) - Town & City.rfr"));
}

TEST(NAME, to_string_2)
{
    ReplayFileParts parts(
            "",
            {"TheComet", "TAEL"},
            {"Pikachu", "C. Falcon"},
            "2022-09-11", "19:45:22",
            "Town & City",
            BracketType::fromType(BracketType::FRIENDLIES),
            Round::fromType(Round::FREE, SessionNumber::fromValue(6)),
            SetFormat::fromType(SetFormat::FREE),
            ScoreCount::fromScore(2, 1),
            0x00);

    EXPECT_THAT(parts.toFileName().cStr(), StrEq("2022-09-11_19-45-22 - Friendlies - Free (6) - TheComet (Pikachu) vs TAEL (C. Falcon) - Game 4 (2-1) - Town & City.rfr"));
}
