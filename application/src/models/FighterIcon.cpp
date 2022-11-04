#include "application/models/FighterIcon.hpp"
#include "rfcommon/FighterID.hpp"
#include <QHash>

namespace rfapp {

static const char* files[] = {
    "mario",
    "donkey",
    "link",
    "samus",
    "samusd",
    "yoshi",
    "kirby",
    "fox",
    "pikachu",
    "luigi",
    "ness",
    "captain",
    "purin",
    "peach",
    "daisy",
    "koopa",
    "sheik",
    "zelda",
    "mariod",
    "pichu",
    "falco",
    "marth",
    "lucina",
    "younglink",
    "ganon",
    "mewtwo",
    "roy",
    "chrom",
    "gamewatch",
    "metaknight",
    "pit",
    "pitb",
    "szerosuit",
    "wario",
    "snake",
    "ike",
    "pzenigame",
    "pfushigisou",
    "plizardon",
    "diddy",
    "lucas",
    "sonic",
    "dedede",
    "pikmin",
    "lucario",
    "robot",
    "toonlink",
    "wolf",
    "murabito",
    "rockman",
    "wiifit",
    "rosetta",
    "littlemac",
    "gekkouga",
    "palutena",
    "pacman",
    "reflet",
    "shulk",
    "koopajr",
    "duckhunt",
    "ryu",
    "ken",
    "cloud",
    "kamui",
    "bayonetta",
    "inkling",
    "ridley",
    "simon",
    "richter",
    "krool",
    "shizue",
    "gaogaen",
    "miifighter",
    "miiswordsman",
    "miigunner",
    "ice_climber",
    "ice_climber",
	"",
    "miifighter",
    "miiswordsman",
    "miigunner",
    "packun",
    "jack",
    "brave",
    "buddy",
    "dolly",
    "master",
    "tantan",
    "pickel",
    "edge",
	"eflame",
	"elight",
    "demon",
    "trail",
};
    
// ----------------------------------------------------------------------------
QPixmap FighterIcon::fromFighterName(const char* name, int skin)
{
    static QHash<QString, rfcommon::FighterID> map = {
        {"Mario",            rfcommon::FighterID::fromValue(0)},
        {"Donkey Kong",      rfcommon::FighterID::fromValue(1)},
        {"Link",             rfcommon::FighterID::fromValue(2)},
        {"Samus",            rfcommon::FighterID::fromValue(3)},
        {"Dark Samus",       rfcommon::FighterID::fromValue(4)},
        {"Yoshi",            rfcommon::FighterID::fromValue(5)},
        {"Kirby",            rfcommon::FighterID::fromValue(6)},
        {"Fox",              rfcommon::FighterID::fromValue(7)},
        {"Pikachu",          rfcommon::FighterID::fromValue(8)},
        {"Luigi",            rfcommon::FighterID::fromValue(9)},
        {"Ness",             rfcommon::FighterID::fromValue(10)},
        {"Captain Falcon",   rfcommon::FighterID::fromValue(11)},
        {"Jigglypuff",       rfcommon::FighterID::fromValue(12)},
        {"Peach",            rfcommon::FighterID::fromValue(13)},
        {"Daisy",            rfcommon::FighterID::fromValue(14)},
        {"Bowser",           rfcommon::FighterID::fromValue(15)},
        {"Sheik",            rfcommon::FighterID::fromValue(16)},
        {"Zelda",            rfcommon::FighterID::fromValue(17)},
        {"Doctor Mario",     rfcommon::FighterID::fromValue(18)},
        {"Pichu",            rfcommon::FighterID::fromValue(19)},
        {"Falco",            rfcommon::FighterID::fromValue(20)},
        {"Marth",            rfcommon::FighterID::fromValue(21)},
        {"Lucina",           rfcommon::FighterID::fromValue(22)},
        {"Young Link",       rfcommon::FighterID::fromValue(23)},
        {"Ganondorf",        rfcommon::FighterID::fromValue(24)},
        {"Mewto",            rfcommon::FighterID::fromValue(25)},
        {"Roy",              rfcommon::FighterID::fromValue(26)},
        {"Chrom",            rfcommon::FighterID::fromValue(27)},
        {"Mr. Game & Watch", rfcommon::FighterID::fromValue(28)},
        {"Meta Knight",      rfcommon::FighterID::fromValue(29)},
        {"Pit",              rfcommon::FighterID::fromValue(30)},
        {"Dark Pit",         rfcommon::FighterID::fromValue(31)},
        {"Zero Suit Samus",  rfcommon::FighterID::fromValue(32)},
        {"Wario",            rfcommon::FighterID::fromValue(33)},
        {"Snake",            rfcommon::FighterID::fromValue(34)},
        {"Ike",              rfcommon::FighterID::fromValue(35)},
        {"Squirtle",         rfcommon::FighterID::fromValue(36)},
        {"Ivysaur",          rfcommon::FighterID::fromValue(37)},
        {"Charizard",        rfcommon::FighterID::fromValue(38)},
        {"Diddy Kong",       rfcommon::FighterID::fromValue(39)},
        {"Lucas",            rfcommon::FighterID::fromValue(40)},
        {"Sonic",            rfcommon::FighterID::fromValue(41)},
        {"King Dedede",      rfcommon::FighterID::fromValue(42)},
        {"Olimar",           rfcommon::FighterID::fromValue(43)},
        {"Lucario",          rfcommon::FighterID::fromValue(44)},
        {"R.O.B.",           rfcommon::FighterID::fromValue(45)},
        {"Toon Link",        rfcommon::FighterID::fromValue(46)},
        {"Wolf",             rfcommon::FighterID::fromValue(47)},
        {"Villager",         rfcommon::FighterID::fromValue(48)},
        {"Mega Man",         rfcommon::FighterID::fromValue(49)},
        {"Wii Fit Trainer",  rfcommon::FighterID::fromValue(50)},
        {"Rosalina",         rfcommon::FighterID::fromValue(51)},
        {"Little Mac",       rfcommon::FighterID::fromValue(52)},
        {"Greninja",         rfcommon::FighterID::fromValue(53)},
        {"Palutena",         rfcommon::FighterID::fromValue(54)},
        {"Pac-Man",          rfcommon::FighterID::fromValue(55)},
        {"Robin",            rfcommon::FighterID::fromValue(56)},
        {"Shulk",            rfcommon::FighterID::fromValue(57)},
        {"Bowser Jr",        rfcommon::FighterID::fromValue(58)},
        {"Duck Hunt Duo",    rfcommon::FighterID::fromValue(59)},
        {"Ryu",              rfcommon::FighterID::fromValue(60)},
        {"Ken",              rfcommon::FighterID::fromValue(61)},
        {"Cloud",            rfcommon::FighterID::fromValue(62)},
        {"Corrin",           rfcommon::FighterID::fromValue(63)},
        {"Bayonetta",        rfcommon::FighterID::fromValue(64)},
        {"Inkling",          rfcommon::FighterID::fromValue(65)},
        {"Ridley",           rfcommon::FighterID::fromValue(66)},
        {"Simon",            rfcommon::FighterID::fromValue(67)},
        {"Richter",          rfcommon::FighterID::fromValue(68)},
        {"K. Rool",          rfcommon::FighterID::fromValue(69)},
        {"Isabelle",         rfcommon::FighterID::fromValue(70)},
        {"Incineroar",       rfcommon::FighterID::fromValue(71)},
        {"Mii Brawler",      rfcommon::FighterID::fromValue(72)},
        {"Mii Swordfighter", rfcommon::FighterID::fromValue(73)},
        {"Mii Gunner",       rfcommon::FighterID::fromValue(74)},
        {"Popo",             rfcommon::FighterID::fromValue(75)},
        {"Nana",             rfcommon::FighterID::fromValue(76)},
        {"Giga Bowser",      rfcommon::FighterID::fromValue(77)},
        {"Mii Brawler",      rfcommon::FighterID::fromValue(78)},
        {"Mii Swordfighter", rfcommon::FighterID::fromValue(79)},
        {"Mii Gunner",       rfcommon::FighterID::fromValue(80)},
        {"Piranha Plant",    rfcommon::FighterID::fromValue(81)},
        {"Joker",            rfcommon::FighterID::fromValue(82)},
        {"Hero",             rfcommon::FighterID::fromValue(83)},
        {"Banjo & Kazooie",  rfcommon::FighterID::fromValue(84)},
        {"Terry",            rfcommon::FighterID::fromValue(85)},
        {"Byleth",           rfcommon::FighterID::fromValue(86)},
        {"Min Min",          rfcommon::FighterID::fromValue(87)},
        {"Steve",            rfcommon::FighterID::fromValue(88)},
        {"Sephiroth",        rfcommon::FighterID::fromValue(89)},
        {"Pyra",             rfcommon::FighterID::fromValue(90)},
        {"Mythra",           rfcommon::FighterID::fromValue(91)},
        {"Kazuya",           rfcommon::FighterID::fromValue(92)},
        {"Sora",             rfcommon::FighterID::fromValue(93)},
        //{"Pyra & Mythra",    rfcommon::FighterID::fromValue(117)}
    };

    const auto it = map.find(name);
    if (it == map.end())
        return QPixmap();

    rfcommon::FighterID fighterID = it.value();
    QString fileName = QString(":/ssbu_icons/chara_2_") + files[fighterID.value()] + "_0" + QString::number(skin) + ".png";

    return QPixmap(fileName);
}

}
