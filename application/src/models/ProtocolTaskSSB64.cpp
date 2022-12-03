#include "application/models/ProtocolTaskSSB64.hpp"

#include "rfcommon/FighterID.hpp"
#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/StageID.hpp"
#include "rfcommon/TrainingMetaData.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/time.h"

#if defined(RFCOMMON_PLATFORM_WINDOWS)
#   include <windows.h>
#   include <tlhelp32.h>
#endif

#define RDRAM_START 0x80000000
#define RDRAM_SIZE  0x800000
#define IS_RDRAM(x) ((x) >= RDRAM_START && (x) < RDRAM_START + RDRAM_SIZE)

#define RDRAM_TO_PHYS(gameBaseAddr, x) ((gameBaseAddr) + ((x) & 0x1FFFFFFF))

namespace rfapp {

enum ROMVersion
{
    JAP,
    AUS,
    EU,
    US,
    iQue
};

enum SSB64Screen
{
    SCREEN_TITLE                     = 1,
    SCREEN_MODE_SELECT               = 7,
    SCREEN_1P_MODE                   = 8,
    SCREEN_VS_MODE                   = 9,
    SCREEN_VS_OPTIONS                = 10,
    SCREEN_VS_ITEM_SWITCH            = 10,
    SCREEN_1P_LOADING_SCREEN         = 14,
    SCREEN_SCREEN_ADJUST             = 15,
    SCREEN_VS_CHARACTER_SELECT       = 16,
    SCREEN_1P_CHARACTER_SELECT       = 17,
    SCREEN_TRAINING_CHARACTER_SELECT = 18,
    SCREEN_BONUS1_CHARACTER_SELECT   = 19,
    SCREEN_BONUS2_CHARACTER_SELECT   = 20,
    SCREEN_STAGE_SELECT              = 21,  /* Applies to all modes */
    SCREEN_VS_GAME                   = 22,
    SCREEN_VS_RESULTS                = 24,
    SCREEN_DATA_VS_RECORD            = 25,
    SCREEN_DATA_CHARACTERS           = 26,
    SCREEN_N64_LOGO                  = 27,  /* Intro cutscene starts here */
    SCREEN_INTRO_SCENE1              = 28,  /* Logo + Room + toys come to life */
    SCREEN_INTRO_SCENE2              = 29,  /* Characters appearing in strips */
    SCREEN_INTRO_SCENE3              = 30,  /* Mario! */
    SCREEN_INTRO_SCENE4              = 31,  /* DK! */
    SCREEN_INTRO_SCENE5              = 34,  /* Link! */
    SCREEN_INTRO_SCENE6              = 32,  /* Samus! */
    SCREEN_INTRO_SCENE7              = 35,  /* Yoshi! */
    SCREEN_INTRO_SCENE8              = 37,  /* Kirby! */
    SCREEN_INTRO_SCENE9              = 33,  /* Fox! */
    SCREEN_INTRO_SCENE10             = 36,  /* Pikachu! */
    SCREEN_INTRO_SCENE11             = 38,  /* All running to the left */
    SCREEN_INTRO_SCENE12             = 40,  /* Link drinking some shit on a hill */
    SCREEN_INTRO_SCENE13             = 42,  /* Pikachu being smaller than a pokeball */
    SCREEN_INTRO_SCENE14             = 45,  /* DK in Congo Jungle getting roll -> charge shot */
    SCREEN_INTRO_SCENE15             = 39,  /* Yoshi laying eggs */
    SCREEN_INTRO_SCENE16             = 44,  /* Fox */
    SCREEN_INTRO_SCENE17             = 41,  /* Mario vs Kirby */
    SCREEN_INTRO_SCENE18             = 43,  /* All characters come together */
    SCREEN_INTRO_SCENE19             = 46,  /* Unlockable characters */
                                            /* (transitions to SCREEN_TITLE) */
    SCREEN_BACKUP_CLEAR              = 47,
    SCREEN_OUTRO_SCENE               = 48,  /* Character on table, camera slowly walks away, fade to white */
    SCREEN_1P_RESULTS                = 51,  /* Results after clearing a stage or bonus stage */
    SCREEN_1P_BONUS_STAGE            = 53,  /* Break the targets and board the platforms */
    SCREEN_TRAINING_MODE             = 54,
    SCREEN_CONGRATULATIONS           = 55,
    SCREEN_CREDITS                   = 56,
    SCREEN_OPTION                    = 57,
    SCREEN_DATA                      = 58,
    SCREEN_DATA_SOUND_TEST           = 59,
    SCREEN_HOW_TO_PLAY               = 60,
    SCREEN_DEMO_FIGHT                = 61,  /* 4 CPUs on dreamland or on planet zebes */
};

enum SSB64ControlledBy
{
    CONTROLLED_BY_HUMAN = 0,
    CONTROLLED_BY_AI = 1,
    CONTROLLED_BY_NONE = 2
};

enum EmulatorType
{
    PROJECT64_1_6,
    PROJECT64_2_3_2,
    PROJECT64_3_0_1,
    PROJECT64K,
    PROJECT64KVE,
    PROJECT64KSE,
};

static const struct Emulator
{
    const char* PROC_NAME;
    const char* EXPECTED_WINDOW_TITLE;
    uint32_t VERSION_STRING_OFFSET;
    uint32_t GAME_MEMORY;
} EMULATOR[] = {
    {"Project64.exe",    "Project64 Version 1.6",     0x0,    0xD6A1C  },
    {"Project64.exe",    "Project64 2.3.2",           0x8518, 0x10F72C },
    {"Project64.exe",    "Project64 3.0.1",           0x0,    0x1F9740 },
    {"Project64k.exe",   "Project64", 0, 0x9262C},
    {"Project64KVE.exe", "Project64", 0, 0x9262C},
    {"Project64KSE.exe", "Project64", 0, 0x9262C}
};
#define EMULATOR_COUNT (sizeof(EMULATOR) / sizeof(*EMULATOR))

static const struct SSB64Globals
{                                     /*      Japan   Australia      Europe         USA        iQue */
    uint32_t SCREEN_FRAME_COUNTER[5] = {          0,          0,          0, 0x8003B6E4,          0 };
    uint32_t ITEM_LIST[5]            = { 0x800466F0, 0x80046E20, 0x80046E60, 0x80046700, 0x80098450 };
    uint32_t MUSIC[5]                = { 0x80098BD3, 0x80099833, 0x800A2E63, 0x80099113, 0x80092993 };
    uint32_t UNLOCKED_STUFF[5]       = { 0x800A28F4, 0x800A5074, 0x800AD194, 0x800A4934, 0x800A4988 };
    uint32_t SCREENS[5]              = {          0,          0,          0, 0x800A4AD2,          0 };
    uint32_t MATCH_TIMERS[5]         = {          0,          0,          0, 0x800A4D1C,          0 };
    uint32_t MATCH_SETTINGS[5]       = { 0x800A30A8, 0x800A5828, 0x800AD948, 0x800A50E8, 0x800A5C68 };
    uint32_t HURTBOX_COLOR_RG[5]     = {          0,          0,          0, 0x800F2786,          0 };
    uint32_t HURTBOX_COLOR_BA[5]     = {          0,          0,          0, 0x800F279E,          0 };
    uint32_t RED_HITBOX_PATCH[5]     = {          0,          0,          0, 0x800F33BC,          0 };
    uint32_t PURPLE_HITBOX_PATCH[5]  = {          0,          0,          0, 0x800F2FD0,          0 };
    uint32_t FIGHTER_STRUCT[5]       = { 0x8012E914, 0x80131594, 0x80139A74, 0x80130D84, 0x80130F04 };
    uint32_t MATCH_DATA[5]           = {          0,          0,          0, 0x801317CC,          0 };
    uint32_t ITEM_HITBOX_OFFSET[5]   = {      0x370,          0,          0,      0x374,      0x374 };
} GLOBALS;

static const struct SSB64Screens
{
    uint32_t PREV    = 0x00;
    uint32_t CURRENT = 0x01;
} SCREENS;

static const struct SSB64MatchTimers
{
    uint32_t REMAINING = 0x00;
    uint32_t ELAPSED   = 0x04;
} MATCH_TIMERS;

static const struct SSB64MatchData
{
    uint32_t STOCKS[4] = { 0x03, 0x02, 0x01, 0x00 };  /* Offsets are based on port index, not on fighter index */
    uint32_t TIME_SINCE_LAST_PAUSE = 0x30;
    uint32_t NOT_PAUSED = 0x34;
} MATCH_DATA;

static const struct SSB64Fighter
{
    uint32_t NEXT_FIGHTER = 0x00;
    uint32_t CHARACTER = 0x0B;
    uint32_t COSTUME = 0x10;
    uint32_t MOVEMENT_FRAME = 0x1C;
    uint32_t MOVEMENT_STATE = 0x24;
    uint32_t PERCENT = 0x2C;
    uint32_t SHIELD_SIZE = 0x34;
    uint32_t FACING_DIRECTION = 0x44;
    uint32_t VELOCITY_X = 0x48;
    uint32_t VELOCITY_Y = 0x4C;
    uint32_t ACCELERATION_X = 0x60;
    uint32_t ACCELERATION_Y = 0x64;
    uint32_t POSITION_VECTOR_PTR = 0x78;
    struct Position {
        uint32_t POS_X = 0x00;
        uint32_t POS_Y = 0x04;
        uint32_t POS_Z = 0x08;
    } POSITION;
    uint32_t JUMP_COUNTER = 0x148;
    uint32_t GROUNDED = 0x14C;
    uint32_t CONTROLLER_INPUT_PTR = 0x1B0;
    uint32_t SHIELD_BREAK_RECOVERY_TIME = 0x26C;
    uint32_t INVINCIBILITY_STATE = 0x5AC;
    uint32_t HITSTUN = 0xB18;
    /* a bunch more stuff we probably don't care about */
    uint32_t SHOW_HITBOX = 0xB4C;
} FIGHTER;

static const struct SSB64MatchSettings
{
    uint32_t TYPE   = 0x00;  /* 1=time, 2=stock, 3=timed stock */
    uint32_t MODE   = 0x01;  /* 0=free for all, 1=team battle */
    uint32_t STAGE  = 0x02;
    uint32_t STOCKS = 0x04;  /* number of stocks set in the menu, where 0==1 stock, 1==2 stocks, etc. */
    uint32_t TIME = 0x05;    /* time set in menu, in minutes. 0x64 == infinity */
    uint32_t PLAYER_DATA_OFFSET[4] = { 0x20, 0x94, 0x108, 0x17C };
    struct PlayerData {
        uint32_t CHARACTER = 0x00;
        uint32_t CONTROLLED_BY = 0x01;  /* 0=Human, 1=CPU, 2=Closed */
        uint32_t COSTUME = 0x05;  /* 0=Normal, 1=C-Right, 2=C-Down, 3=C-Left */
    } PLAYER_DATA;
} MATCH_SETTINGS;

// ----------------------------------------------------------------------------
ProtocolTaskSSB64::ProtocolTaskSSB64(rfcommon::Log* log, QObject* parent)
    : log_(log)
    , requestShutdown_(false)
{
}

// ----------------------------------------------------------------------------
ProtocolTaskSSB64::~ProtocolTaskSSB64()
{
    mutex_.lock();
        requestShutdown_ = true;
    mutex_.unlock();
    wait();
}

// ----------------------------------------------------------------------------
struct WindowTitleData
{
    DWORD procId;
    int emuIdx;
};
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    WindowTitleData* data = (WindowTitleData*)lParam;

    DWORD dwProcId;
    GetWindowThreadProcessId(hWnd, &dwProcId);
    if (data->procId != dwProcId)
        return TRUE;  // Continue enumeration

    char title[128];
    if (GetWindowTextA(hWnd, title, 128) == 0)
        return TRUE;

    for (int i = 0; i != EMULATOR_COUNT; ++i)
        if (strstr(title, EMULATOR[i].EXPECTED_WINDOW_TITLE) != nullptr)
        {
            data->emuIdx = i;
            return FALSE;
        }
}

// ----------------------------------------------------------------------------
static int scanForEmulators(uint32_t* pid, void** processHandle, uintptr_t* moduleBaseAddr, rfcommon::Log* log)
{
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        log->error("CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL) failed");
        return 0;
    }

    int emuIdx = -1;
    DWORD procId = 0;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &entry) == TRUE)
    {
        auto procNameToIndex = [](const char* procName) -> int {
            for (int i = 0; i != EMULATOR_COUNT; ++i)
                if (strcmp(procName, EMULATOR[i].PROC_NAME) == 0)
                    return i;
            return -1;
        };

        do
        {
            emuIdx = procNameToIndex(entry.szExeFile);
            if (emuIdx != EmulatorType(-1))
            {
                procId = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &entry));
    }
    CloseHandle(hSnapshot);

    if (emuIdx == -1)
        return -1;

    BYTE* modBaseAddr = 0;
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        log->error("CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, %u) failed", procId);
        return 0;
    }

    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnapshot, &modEntry))
        {
            do
            {
                if (strcmp(modEntry.szModule, EMULATOR[emuIdx].PROC_NAME) == 0)
                {
                    modBaseAddr = modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &modEntry));
        }
    }
    CloseHandle(hSnapshot);

    if (modBaseAddr == 0)
        return -1;

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, procId);
    if (hProcess == NULL)
        return -1;

    // There are multiple versions of Project64
    // Attempt to get the version from the title in the window
    WindowTitleData titleData;
    titleData.procId = procId;
    titleData.emuIdx = -1;
    EnumWindows(EnumWindowsProc, (LPARAM)&titleData);
    if (titleData.emuIdx == -1)
    {
        CloseHandle(hProcess);
        return -1;
    }
    emuIdx = titleData.emuIdx;

    *pid = procId;
    *processHandle = (void*)hProcess;
    *moduleBaseAddr = (uintptr_t)modBaseAddr;

    return emuIdx;
#else
    return -1;
#endif
}

// ----------------------------------------------------------------------------
static uint32_t getGameBaseAddress(int emuIdx, void* processHandle, uintptr_t moduleBaseAddr, rfcommon::Log* log)
{
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    HANDLE hProcess = (HANDLE)processHandle;
    uint32_t gameBaseAddr;
    if (ReadProcessMemory(hProcess, (void*)(moduleBaseAddr + EMULATOR[emuIdx].GAME_MEMORY), (void*)&gameBaseAddr, 4, NULL) == 0)
    {
        log->error("Failed to read process memory for game base address");
        return 0;
    }
    return gameBaseAddr;
#else
    return 0;
#endif
}

// ----------------------------------------------------------------------------
static uint8_t getCurrentScreen(void* processHandle, uint32_t gameBaseAddr, rfcommon::Log* log)
{
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    HANDLE hProcess = (HANDLE)processHandle;
    uint8_t screen;
    if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + ((GLOBALS.SCREENS[US] + SCREENS.CURRENT) & 0x1FFFFFFF)), (void*)&screen, 1, NULL) == 0)
    {
        log->error("Failed to read process memory for current screen");
        return 0;
    }
    return screen;
#else
    return 0;
#endif
}

// ----------------------------------------------------------------------------
static bool getMatchSettings(
    void* processHandle,
    uint32_t gameBaseAddr,
    rfcommon::SmallVector<int, 4>* portMap,
    rfcommon::SmallVector<rfcommon::String, 2>* tags,
    rfcommon::SmallVector<rfcommon::FighterID, 2>* fighterIDs,
    rfcommon::SmallVector<rfcommon::Costume, 2>* fighterCostumes,
    rfcommon::StageID* stageID,
    rfcommon::Log* log)
{
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    HANDLE hProcess = (HANDLE)processHandle;
    uint32_t matchSettingsAddr;
    if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (GLOBALS.MATCH_SETTINGS[US] & 0x1FFFFFFF)), (void*)&matchSettingsAddr, 4, NULL) == 0)
    {
        log->error("Failed to read process memory for match settings address");
        return false;
    }
    if (!IS_RDRAM(matchSettingsAddr))
    {
        log->error("Match settings pointer is outside of RDRAM");
        return false;
    }

    uint8_t stage;
    if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + ((matchSettingsAddr + MATCH_SETTINGS.STAGE) & 0x1FFFFFFF)), (void*)&stage, 1, NULL) == 0)
    {
        log->error("Failed to read process memory for stage");
        return false;
    }
    *stageID = rfcommon::StageID::fromValue(stage);

    portMap->clear();
    fighterIDs->clear();
    for (int i = 0; i != 4; ++i)
    {
        uint32_t controlledByAddr = matchSettingsAddr + MATCH_SETTINGS.PLAYER_DATA_OFFSET[i] + MATCH_SETTINGS.PLAYER_DATA.CONTROLLED_BY;
        uint32_t fighterAddr = matchSettingsAddr + MATCH_SETTINGS.PLAYER_DATA_OFFSET[i] + MATCH_SETTINGS.PLAYER_DATA.CHARACTER;
        uint32_t costumeAddr = matchSettingsAddr + MATCH_SETTINGS.PLAYER_DATA_OFFSET[i] + MATCH_SETTINGS.PLAYER_DATA.COSTUME;
        uint8_t controlledBy, fighter, costume;
        if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (controlledByAddr & 0x1FFFFFFF)), (void*)&controlledBy, 1, NULL) == 0)
        {
            log->error("Failed to read process memory for fighter control");
            return false;
        }
        if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (fighterAddr & 0x1FFFFFFF)), (void*)&fighter, 1, NULL) == 0)
        {
            log->error("Failed to read process memory for fighter ID");
            return false;
        }
        if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (costumeAddr & 0x1FFFFFFF)), (void*)&costume, 1, NULL) == 0)
        {
            log->error("Failed to read process memory for fighter costume");
            return false;
        }

        if (controlledBy == CONTROLLED_BY_HUMAN)
            tags->push("Player " + rfcommon::String::decimal(i + 1));
        else if (controlledBy == CONTROLLED_BY_AI)
            tags->push("CPU " + rfcommon::String::decimal(i + 1));
        else
            continue;
        portMap->push(i);
        fighterIDs->push(rfcommon::FighterID::fromValue(fighter));
        fighterCostumes->push(rfcommon::Costume::fromValue(costume));
    }

    return true;
#else
    return false;
#endif
}

// ----------------------------------------------------------------------------
void ProtocolTaskSSB64::run()
{
    void* processHandle;
    uintptr_t emuBaseAddr;
    uint32_t pid;
    int emuIdx = scanForEmulators(&pid, &processHandle, &emuBaseAddr, log_);
    if (emuIdx == -1)
    {
        emit connectionFailure("No emulator process found", "", pid);
        return;
    }

    emit connectionSuccess(EMULATOR[emuIdx].PROC_NAME, pid);

    SSB64Screen prevScreen = SCREEN_N64_LOGO;
    uint32_t lastFrame = 0;
    rfcommon::SmallVector<int, 4> portMap;
    while (true)
    {
        mutex_.lock();
        if (requestShutdown_)
        {
            log_->info("Shutdown requested");
            mutex_.unlock();
            break;
        }
        mutex_.unlock();

        // The n64 memory block begins at this address. All N64 pointers will be relative to this value
        uint32_t gameBaseAddr = getGameBaseAddress(emuIdx, processHandle, emuBaseAddr, log_);
        if (gameBaseAddr == 0)
        {
            break;
        }

        // We detect start/stop events by reading the current screen ID
        SSB64Screen currentScreen = (SSB64Screen)getCurrentScreen(processHandle, gameBaseAddr, log_);
        if (currentScreen == SCREEN_VS_GAME && prevScreen != SCREEN_VS_GAME)
        {
            rfcommon::SmallVector<rfcommon::String, 2> tags;
            rfcommon::SmallVector<rfcommon::FighterID, 2> fighterIDs;
            rfcommon::SmallVector<rfcommon::Costume, 2> costumes;
            rfcommon::StageID stageID = rfcommon::StageID::makeInvalid();
            if (getMatchSettings(processHandle, gameBaseAddr, &portMap, &tags, &fighterIDs, &costumes, &stageID, log_) == false)
                continue;

            if (fighterIDs.count() < 2)
            {
                portMap.clear();
                log_->error("Match settings read, but there were less than 2 fighters configured (%d). Can't start game without more fighters", fighterIDs.count());
                continue;
            }

            log_->beginDropdown("Game Session");
            log_->info("Stage: %d", stageID.value());
            for (int i = 0; i != fighterIDs.count(); ++i)
                log_->info("%d: Slot %d, Fighter %d, Costume %d, Tag \"%s\"", i, portMap[i], fighterIDs[i].value(), costumes[i].slot(), tags[i].cStr());
            log_->endDropdown();

            emit gameStarted(rfcommon::MetaData::newActiveGameSession(
                stageID, std::move(fighterIDs), std::move(costumes), std::move(tags)
            ));
        }
        else if (currentScreen != SCREEN_VS_GAME && prevScreen == SCREEN_VS_GAME)
        {
            emit gameEnded();
        }
        else if (currentScreen == SCREEN_TRAINING_MODE && prevScreen != SCREEN_TRAINING_MODE)
        {
        }
        else if (currentScreen != SCREEN_TRAINING_MODE && prevScreen == SCREEN_TRAINING_MODE)
        {
        }
        prevScreen = currentScreen;

        // If a game is loaded (or training mode), start reading out fighter information
        if (currentScreen == SCREEN_VS_GAME)
        {
            // It looks like the screen's "frame counter" is updated quite late relative to the
            // data we're interested in reading, so we use it to synchronize to frame updates.
            uint32_t frame;
            HANDLE hProcess = (HANDLE)processHandle;
            if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, GLOBALS.SCREEN_FRAME_COUNTER[US]), (void*)&frame, 4, NULL) == 0)
                continue;
            if (frame == lastFrame)
                continue;
            lastFrame = frame;

            // If the game is paused, don't update
            uint8_t notPaused;
            if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, GLOBALS.MATCH_DATA[US] + MATCH_DATA.NOT_PAUSED), (void*)&notPaused, 1, NULL) == 0)
                continue;
            if (!notPaused)
                continue;

            // Get the number of frames left
            uint32_t framesLeft;
            if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, GLOBALS.MATCH_TIMERS[US] + MATCH_TIMERS.REMAINING), (void*)&framesLeft, 4, NULL) == 0)
                continue;

            // Get the address of the first fighter. The fighters are in a linked list.
            uint32_t fighterAddr;
            if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, GLOBALS.FIGHTER_STRUCT[US]), (void*)&fighterAddr, 4, NULL) == 0)
                continue;
            for (int fighterIdx = 0; fighterIdx != portMap.count(); ++fighterIdx)
            {
                if (IS_RDRAM(fighterAddr) == false)
                    goto read_fighter_failed;

                // X, Y position
                uint32_t posVecAddr;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, fighterAddr + FIGHTER.POSITION_VECTOR_PTR), (void*)&posVecAddr, 4, NULL) == 0)
                    goto read_fighter_failed;
                uint32_t xposBits, yposBits;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, posVecAddr + FIGHTER.POSITION.POS_X), (void*)&xposBits, 4, NULL) == 0)
                    goto read_fighter_failed;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, posVecAddr + FIGHTER.POSITION.POS_Y), (void*)&yposBits, 4, NULL) == 0)
                    goto read_fighter_failed;
                float posx = *reinterpret_cast<float*>(&xposBits);
                float posy = *reinterpret_cast<float*>(&yposBits);

                // Read damage (percent)
                uint32_t damage;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, fighterAddr + FIGHTER.PERCENT), (void*)&damage, 4, NULL) == 0)
                    goto read_fighter_failed;

                // Read damage (percent)
                uint32_t hitstun;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, fighterAddr + FIGHTER.HITSTUN), (void*)&hitstun, 4, NULL) == 0)
                    goto read_fighter_failed;

                // Read shield health
                uint32_t shield;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, fighterAddr + FIGHTER.SHIELD_SIZE), (void*)&shield, 4, NULL) == 0)
                    goto read_fighter_failed;

                // Read shield health
                uint32_t status;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, fighterAddr + FIGHTER.MOVEMENT_STATE), (void*)&status, 4, NULL) == 0)
                    goto read_fighter_failed;

                // Stock count for this fighter
                uint8_t stocks;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, GLOBALS.MATCH_DATA[US] + MATCH_DATA.STOCKS[portMap[fighterIdx]]), (void*)&stocks, 1, NULL) == 0)
                    goto read_fighter_failed;

                // Facing direction
                int32_t facingDirection;
                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, fighterAddr + FIGHTER.FACING_DIRECTION), (void*)&facingDirection, 4, NULL) == 0)
                    goto read_fighter_failed;

                quint64 frameTimeStamp = time_milli_seconds_since_epoch();
                emit fighterState(
                    frameTimeStamp,
                    framesLeft,
                    fighterIdx,
                    posx, posy,
                    static_cast<float>(damage),
                    static_cast<float>(hitstun),
                    static_cast<float>(shield),
                    status,
                    rfcommon::hash40("wait").value(),
                    0,  // hit_status
                    stocks,
                    false,
                    facingDirection > 0,
                    false);

                if (ReadProcessMemory(hProcess, (void*)RDRAM_TO_PHYS(gameBaseAddr, fighterAddr + FIGHTER.NEXT_FIGHTER), (void*)&fighterAddr, 4, NULL) == 0)
                    goto read_fighter_failed;
            }

            read_fighter_failed:;
        }
    }

    CloseHandle((HANDLE)processHandle);
    emit connectionClosed();
}

}
