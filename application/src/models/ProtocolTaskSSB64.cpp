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

namespace rfapp {

enum EmulatorType
{
    PROJECT64_1_6,
    PROJECT64_2_4,
    PROJECT64K,
    PROJECT64KVE,
    PROJECT64KSE,
};

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
    SCREEN_MODE_SELECT = 3,
    SCREEN_TITLE = 4,
    SCREEN_VS_MODE = 5,
    SCREEN_INTRO_CUTSCENE = 6,
    SCREEN_GAME = 8,
    SCREEN_STAGE_SELECT = 9,
    SCREEN_NO_CONTEST = 10,
    SCREEN_RESULTS = 11,
    SCREEN_CHARACTER_SELECT = 14,
};

enum SSB64ControlledBy
{
    CONTROLLED_BY_HUMAN = 0,
    CONTROLLED_BY_AI = 1,
    CONTROLLED_BY_NONE = 2
};

static const struct Emulator
{
    const char* PROC_NAME;
    uint32_t GAME_MEMORY;
} EMULATOR[] = {
    {"Project64.exe", 0xD6A1C},
    {"", 0},
    {"", 0},
    {"", 0},
    {"Project64KSE.exe", 0x9262C}
};

static const struct SSB64Memory
{
    uint32_t MUSIC;
    uint32_t UNLOCKED_STUFF;
    uint32_t MATCH_SETTINGS_PTR;
    uint32_t HURTBOX_COLOR_RG;
    uint32_t HURTBOX_COLOR_BA;
    uint32_t RED_HITBOX_PATCH;
    uint32_t PURPLE_HITBOX_PATCH;
    uint32_t FIGHTER_STRUCT_PTR;
    uint32_t ITEM_LIST_PTR;
    uint32_t ITEM_HITBOX_OFFSET;
} MEMORY[] = {
    {0x80098BD3, 0x800A28F4, 0x800A30A8, 0,          0,          0,          0,          0x8012E914, 0x800466F0, 0x370},  /* Japan */
    {0x80099833, 0x800A5074, 0x800A5828, 0,          0,          0,          0,          0x80131594, 0x80046E20, 0    },  /* Australia */
    {0x800A2E63, 0x800AD194, 0x800AD948, 0,          0,          0,          0,          0x80139A74, 0x80046E60, 0    },  /* Europe */
    {0x80099113, 0x800A4934, 0x800A50E8, 0x800F2786, 0x800F279E, 0x800F33BC, 0x800F2FD0, 0x80130D84, 0x80046700, 0x374},  /* USA */
    {0x80092993, 0x800A4988, 0x800A5C68, 0,          0,          0,          0,          0x80130F04, 0x80098450, 0x374}   /* iQue */
};

static const struct SSB64Fighter
{
    uint32_t CHARACTER = 0x0B;
    uint32_t COSTUME = 0x10;
    uint32_t MOVEMENT_FRAME = 0x1C;
    uint32_t MOVEMENT_STATE = 0x26;
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
    } PLAYER_DATA;
} MATCH_SETTINGS;

static const uint32_t STOCK_COUNTERS = 0x801317CC;
static const uint32_t WHISPY_BLOWING = 0x80304BFC;
static const uint32_t CURRENT_SCREEN = 0x80046A4C;
static const uint32_t FRAME_TIMER    = 0x8003B6E4;
static const uint32_t ELAPSED_TIMER  = 0x800A4D20;

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
int scanForEmulators(void** processHandle, uintptr_t* moduleBaseAddr, rfcommon::Log* log)
{
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    DWORD pid = 0;
    BYTE* modBaseAddr = 0;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        log->error("CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL) failed");
        return 0;
    }

    auto procNameToIndex = [](const char* procName) -> int {
        for (int i = 0; i != sizeof(EMULATOR) / sizeof(*EMULATOR); ++i)
            if (strcmp(procName, EMULATOR[i].PROC_NAME) == 0)
                return i;
        return -1;
    };

    int emuIdx;
    if (Process32First(hSnapshot, &entry) == TRUE)
    {
        do
        {
            emuIdx = procNameToIndex(entry.szExeFile);
            if (emuIdx != EmulatorType(-1))
            {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &entry));
    }
    CloseHandle(hSnapshot);

    if (emuIdx == -1)
        return -1;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (strcmp(modEntry.szModule, EMULATOR[emuIdx].PROC_NAME) == 0)
                {
                    modBaseAddr = modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);

    if (modBaseAddr == 0)
        return -1;

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, 0, pid);
    if (hProcess == NULL)
        return -1;

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
uint8_t getCurrentScreen(void* processHandle, uint32_t gameBaseAddr, rfcommon::Log* log)
{
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    HANDLE hProcess = (HANDLE)processHandle;
    uint8_t currentScreen;
    if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (CURRENT_SCREEN & 0x1FFFFFFF)), (void*)&currentScreen, 1, NULL) == 0)
    {
        log->error("Failed to read process memory for game base address");
        return 0;
    }
    return currentScreen;
#else
    return 0;
#endif
}

// ----------------------------------------------------------------------------
static bool getMatchSettings(
    void* processHandle,
    uint32_t gameBaseAddr,
    rfcommon::SmallVector<int, 4>* slotMap,
    rfcommon::SmallVector<rfcommon::String, 2>* tags,
    rfcommon::SmallVector<rfcommon::FighterID, 2>* fighterIDs,
    rfcommon::StageID* stageID,
    rfcommon::Log* log)
{
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    HANDLE hProcess = (HANDLE)processHandle;
    uint32_t matchSettingsAddr;
    if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (MEMORY[US].MATCH_SETTINGS_PTR & 0x1FFFFFFF)), (void*)&matchSettingsAddr, 4, NULL) == 0)
    {
        log->error("Failed to read process memory for match settings address");
        return false;
    }
    if (!IS_RDRAM(matchSettingsAddr))
    {
        log->error("Match settings pointer is outside of RDRAM");
        return false;
    }

    uint32_t stageAddr = matchSettingsAddr + MATCH_SETTINGS.STAGE;
    uint8_t stage;
    if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (stageAddr & 0x1FFFFFFF)), (void*)&stage, 1, NULL) == 0)
    {
        log->error("Failed to read process memory for stage");
        return false;
    }
    *stageID = rfcommon::StageID::fromValue(stage);

    slotMap->clear();
    fighterIDs->clear();
    for (int i = 0; i != 4; ++i)
    {
        uint32_t controlledByAddr = matchSettingsAddr + MATCH_SETTINGS.PLAYER_DATA_OFFSET[i] + MATCH_SETTINGS.PLAYER_DATA.CONTROLLED_BY;
        uint32_t fighterAddr = matchSettingsAddr + MATCH_SETTINGS.PLAYER_DATA_OFFSET[i] + MATCH_SETTINGS.PLAYER_DATA.CHARACTER;
        uint8_t controlledBy, fighter;
        if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (controlledByAddr & 0x1FFFFFFF)), (void*)&controlledBy, 1, NULL) == 0)
        {
            log->error("Failed to read process memory for fighter control");
            return false;
        }
        if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (fighterAddr & 0x1FFFFFFF)), (void*)&fighter, 1, NULL) == 0)
        {
            log->error("Failed to read process memory for fighter");
            return false;
        }

        if (controlledBy == CONTROLLED_BY_HUMAN)
            tags->push("Player " + rfcommon::String::decimal(i + 1));
        else if (controlledBy == CONTROLLED_BY_AI)
            tags->push("CPU " + rfcommon::String::decimal(i + 1));
        else
            continue;
        slotMap->push(i);
        fighterIDs->push(rfcommon::FighterID::fromValue(fighter));
    }

    if (fighterIDs->count() < 2)
    {
        slotMap->clear();
        fighterIDs->clear();
        tags->clear();
        return false;
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
    int emuIdx = scanForEmulators(&processHandle, &emuBaseAddr, log_);
    if (emuIdx == -1)
    {
        emit connectionFailure("No emulator process found", "", 0);
        return;
    }

    emit connectionSuccess(EMULATOR[emuIdx].PROC_NAME, 0);

    SSB64Screen prevScreen = SCREEN_INTRO_CUTSCENE;
    uint32_t lastFrame = 0;
    rfcommon::SmallVector<int, 4> slotMap;
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

        uint32_t gameBaseAddr = getGameBaseAddress(emuIdx, processHandle, emuBaseAddr, log_);
        if (gameBaseAddr == 0)
            continue;

        SSB64Screen currentScreen = (SSB64Screen)getCurrentScreen(processHandle, gameBaseAddr, log_);
        if (currentScreen == SCREEN_GAME && prevScreen != SCREEN_GAME)
        {
            rfcommon::SmallVector<rfcommon::String, 2> tags;
            rfcommon::SmallVector<rfcommon::FighterID, 2> fighterIDs;
            rfcommon::StageID stageID = rfcommon::StageID::makeInvalid();
            if (getMatchSettings(processHandle, gameBaseAddr, &slotMap, &tags, &fighterIDs, &stageID, log_) == false)
                continue;
/*
            HANDLE hProcess = (HANDLE)processHandle;
            if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (ELAPSED_TIMER & 0x1FFFFFFF)), (void*)&lastFrame, 4, NULL) == 0)
                continue;*/

            log_->beginDropdown("Game Session");
            log_->info("Stage: %d", stageID.value());
            for (int i = 0; i != fighterIDs.count(); ++i)
                log_->info("%d: Slot %d, Fighter %d, Tag \"%s\"", i, slotMap[i], fighterIDs[i].value(), tags[i].cStr());
            log_->endDropdown();

            emit gameStarted(rfcommon::MetaData::newActiveGameSession(
                stageID, std::move(fighterIDs), std::move(tags)
            ));
            prevScreen = currentScreen;

        }
        else if (currentScreen != SCREEN_GAME && prevScreen == SCREEN_GAME)
        {
            emit gameEnded();
            prevScreen = currentScreen;
        }

        if (currentScreen == SCREEN_GAME)
        {/*
            uint32_t frame;
            HANDLE hProcess = (HANDLE)processHandle;
            if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (ELAPSED_TIMER & 0x1FFFFFFF)), (void*)&frame, 4, NULL) == 0)
                continue;
            if (frame == lastFrame)
                continue;
            lastFrame = frame;

            for (int i = 0; i != slotMap.count(); ++i)
            {
                uint32_t fighterAddr;
                if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + (MEMORY[US].FIGHTER_STRUCT_PTR & 0x1FFFFFFF)), (void*)&fighterAddr, 4, NULL) == 0)
                    continue;
                fighterAddr += i * 0xB50;
                uintptr_t emufighterAddr = gameBaseAddr + (fighterAddr & 0x1FFFFFFF);

                uint32_t posVecAddr;
                if (ReadProcessMemory(hProcess, (void*)(emufighterAddr + FIGHTER.POSITION_VECTOR_PTR), (void*)&posVecAddr, 4, NULL) == 0)
                    continue;
                uintptr_t emuPosVecAddr = gameBaseAddr + (posVecAddr & 0x1FFFFFFF);
                uint32_t xposBits, yposBits;
                if (ReadProcessMemory(hProcess, (void*)(emuPosVecAddr + FIGHTER.POSITION.POS_X), (void*)&xposBits, 4, NULL) == 0)
                    continue;
                if (ReadProcessMemory(hProcess, (void*)(emuPosVecAddr + FIGHTER.POSITION.POS_Y), (void*)&yposBits, 4, NULL) == 0)
                    continue;
                float posx = *reinterpret_cast<float*>(&xposBits);
                float posy = *reinterpret_cast<float*>(&yposBits);

                uint32_t damageBits;
                if (ReadProcessMemory(hProcess, (void*)(emuPosVecAddr + FIGHTER.PERCENT), (void*)&yposBits, 4, NULL) == 0)
                    continue;

                uint16_t timer, elapsed;
                if (ReadProcessMemory(hProcess, (void*)(gameBaseAddr + 0xA4D1C), (void*)&timer, 2, NULL) &&
                    ReadProcessMemory(hProcess, (void*)(gameBaseAddr + 0xA4D20), (void*)&elapsed, 2, NULL))
                    log_->info("timer: %d, elapsed: %d, frame: %u", timer, elapsed, frame);

                log_->info("i: %d, pos x: %f, posy: %f", i, posx, posy);

                quint64 frameTimeStamp = time_milli_seconds_since_epoch();
                emit fighterState(
                    frameTimeStamp,
                    frame,
                    i,
                    posx, posy,
                    0.0,
                    0.0,
                    0.0,
                    0,
                    rfcommon::hash40("wait").value(),
                    0,
                    5,
                    false,
                    false,
                    false);
            }*/
        }
    }

out:
    emit connectionClosed();
}

}
