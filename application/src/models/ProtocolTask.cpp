#include "application/models/ProtocolTask.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/time.h"
#include "rfcommon/tcp_socket.h"
#include "rfcommon/Vector.hpp"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

namespace rfapp {

// ----------------------------------------------------------------------------
ProtocolTask::ProtocolTask(const QString& ipAddress, quint16 port, uint32_t mappingInfoChecksum, rfcommon::Log* log, QObject* parent)
    : QThread(parent)
    , log_(log)
    , ipAddress_(ipAddress)
    , mappingInfoChecksum_(mappingInfoChecksum)
    , port_(port)
    , requestShutdown_(false)
{
}

// ----------------------------------------------------------------------------
ProtocolTask::~ProtocolTask()
{
    mutex_.lock();
        requestShutdown_ = true;
    mutex_.unlock();
    wait();
}

// ----------------------------------------------------------------------------
void ProtocolTask::run()
{
    PROFILE(ProtocolTask, run);

    // Connects to the switch and requests/validates the protocol version
    // being used. If this function returns a valid socket handle, then it
    // means we're successfully connected and compatible, and from this
    // point on we are responsible for closing the socket and emitting
    // connectionClosed() once we're done
    void* tcp_socket_handle = connectAndCheckVersion();
    if (tcp_socket_handle)
    {
        // Try to update our copy of mapping info if necessary. If successful,
        // enter normal protocol handling
        if (negotiateMappingInfo(tcp_socket_handle))
            handleProtocol(tcp_socket_handle);

        // Socket was shutdown, have to close it
        tcp_socket socket = tcp_socket_from_handle(tcp_socket_handle);
        tcp_socket_close(&socket);

        emit connectionClosed();
    }
}

// ----------------------------------------------------------------------------
void* ProtocolTask::connectAndCheckVersion()
{
    PROFILE(ProtocolTask, connectAndCheckVersion);

    // Attempt to connect to the host
    tcp_socket socket;
    QByteArray ba = ipAddress_.toLocal8Bit();
    log_->info("Connecting to %s:%d", ba.constData(), port_);
    if (tcp_socket_connect_to_host(&socket, ba.constData(), port_) != 0)
    {
        log_->error("Failed to connect to %s:%d: %s", ba.constData(), port_, tcp_socket_get_connect_error(&socket));
        emit connectionFailure(
                    tcp_socket_get_connect_error(&socket),
                    ipAddress_, port_);
        return nullptr;
    }

    // Request protocol version from switch so we can verify we're compatible
    char buf[2] = {ProtocolTask::ProtocolVersion};
    log_->info("Requesting protocol version");
    if (tcp_socket_write(&socket, buf, 1) != 1)
    {
        log_->error("Socket write error, closing");
        emit connectionFailure("Failed to write to socket", ipAddress_, port_);
        goto socket_error;
    }

    // Wait until we receive protocol version (or an error)
    while (true)
    {
        mutex_.lock();
            if (requestShutdown_)
            {
                log_->info("Shutdown requested, closing socket");
                mutex_.unlock();
                break;
            }
        mutex_.unlock();

        // Read message type
        log_->info("Waiting for version...");
        if (tcp_socket_read_exact(&socket, buf, 1) != 1)
        {
            log_->error("Socket read error, closing");
            emit connectionFailure("Failed to read from socket", ipAddress_, port_);
            goto socket_error;
        }

        switch (buf[0])
        {
            case ProtocolVersion: {
                if (tcp_socket_read_exact(&socket, buf, 2) != 2)
                {
                    log_->error("Socket read error, closing");
                    emit connectionFailure("Failed to read from socket", ipAddress_, port_);
                    goto socket_error;
                }

                // Protocol version matches ours
                const char major = 0x01;
                const char minor = 0x00;
                if (buf[0] == major && buf[1] == minor)
                {
                    log_->info("Protocol version is %d.%d, we support %d.%d. Accepting", buf[0], buf[1], major, minor);
                    emit connectionSuccess(ipAddress_, port_);
                    return tcp_socket_to_handle(&socket);
                }

                // Protocol version does not match
                log_->error("Protocol version is %d.%d, we support %d.%d. Aborting.", buf[0], buf[1], major, minor);
                emit connectionFailure(
                            QString("Unsupported protocol version major=%1, minor=%2").arg(buf[0]).arg(buf[1]),
                            ipAddress_, port_);
                goto socket_error;
            } break;

            case MappingInfoChecksum:
            case MappingInfoRequest:
            case MappingInfoFighterKinds:
            case MappingInfoFighterStatusKinds:
            case MappingInfoStageKinds:
            case MappingInfoHitStatusKinds:
            case MappingInfoRequestComplete:
            case GameStart:
            case GameEnd:
            case TrainingStart:
            case TrainingReset:
            case TrainingEnd:
            case FighterState:
                break;
        }
    }

socket_error:
    tcp_socket_close(&socket);
    return nullptr;
}

// ----------------------------------------------------------------------------
bool ProtocolTask::negotiateMappingInfo(void* tcp_socket_handle)
{
    PROFILE(ProtocolTask, negotiateMappingInfo);

    tcp_socket socket = tcp_socket_from_handle(tcp_socket_handle);
    rfcommon::Reference<rfcommon::MappingInfo> mappingInfo;

    // Request a checksum for mapping info to see if we need to update our
    // global mapping info
    char buf[1] = {ProtocolTask::MappingInfoChecksum};
    if (tcp_socket_write(&socket, buf, 1) != 1)
        goto disconnect_error;

    while (true)
    {
        mutex_.lock();
            if (requestShutdown_)
            {
                log_->info("Shutdown requested, closing socket");
                mutex_.unlock();
                goto disconnect_error;
            }
        mutex_.unlock();

        uint8_t msg;
        if (tcp_socket_read_exact(&socket, &msg, 1) != 1)
        {
            log_->error("Failed to read message type from socket, closing");
            goto disconnect_error;
        }

        switch (msg)
        {
            case MappingInfoChecksum: {
                uint8_t buf[4];
                if (tcp_socket_read_exact(&socket, buf, 4) != 4)
                {
                    log_->error("Failed to read mapping info checksum from socket, closing");
                    goto disconnect_error;
                }

                // Checksum is the same as our local copy, return success and
                // start normal protocol stuff
                uint32_t checksum = (buf[0] << 24) | (buf[1] << 16) | (buf[2] <<  8) | (buf[3] <<  0);
                log_->info("Received mapping info checksum %d, ours is %d", checksum, mappingInfoChecksum_);
                if (mappingInfoChecksum_ == checksum)
                    return true;

                // Our mapping info is out of date, request new
                log_->info("Requesting mapping info");
                buf[0] = MappingInfoRequest;
                if (tcp_socket_write(&socket, buf, 1) != 1)
                {
                    log_->error("Socket write error, closing");
                    goto disconnect_error;
                }
            } break;

            case MappingInfoRequest: {
                uint8_t buf[4];
                if (tcp_socket_read_exact(&socket, buf, 4) != 4)
                {
                    log_->error("Failed to read new mapping info checksum from socket");
                    goto disconnect_error;
                }

                // Prepare a new mapping info structure to fill in
                uint32_t checksum = (buf[0] << 24) | (buf[1] << 16) | (buf[2] <<  8) | (buf[3] <<  0);
                log_->info("Received mapping info checksum %d", checksum);
                log_->beginDropdown("Mapping Info");
                mappingInfo = new rfcommon::MappingInfo(checksum);
            } break;

            case MappingInfoFighterKinds: {
                uint8_t fighterID;
                uint8_t len;
                char name[256];
                if (tcp_socket_read_exact(&socket, &fighterID, 1) != 1) { log_->error("Failed to read fighter ID"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, &len, 1) != 1) { log_->error("Failed to read fighter len"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, name, len) != len) { log_->error("Failed to read fighter name"); goto disconnect_error; }
                name[static_cast<int>(len)] = '\0';

                // Something is weird
                if (mappingInfo.isNull())
                {
                    log_->error("MappingInfoFighterKinds received even though it wasn't requested?");
                    goto disconnect_error;
                }

                log_->info("Fighter ID: %d - %s", fighterID, name);
                mappingInfo->fighter.add(rfcommon::FighterID::fromValue(fighterID), name);
            } break;

            case MappingInfoFighterStatusKinds: {
                uint8_t fighterIDValue;
                uint8_t statusID_l, statusID_h, len;
                char name[256];
                if (tcp_socket_read_exact(&socket, &fighterIDValue, 1) != 1) { log_->error("Failed to read fighter ID for status"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, &statusID_h, 1) != 1) { log_->error("Failed to read fighter status_h"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, &statusID_l, 1) != 1) { log_->error("Failed to read fighter status_l"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, &len, 1) != 1) { log_->error("Failed to read fighter status len"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, name, len) != len) { log_->error("Failed to read fighter status name"); goto disconnect_error; }
                name[static_cast<int>(len)] = '\0';

                const auto statusID = rfcommon::FighterStatus::fromValue((statusID_h << 8) | (statusID_l << 0));
                const auto fighterID = rfcommon::FighterID::fromValue(fighterIDValue);

                // Something is weird
                if (mappingInfo.isNull())
                {
                    log_->error("MappingInfoFighterStatusKinds received even though it wasn't requested?");
                    goto disconnect_error;
                }

                if (fighterIDValue == 255)
                {
                    log_->info("Base status: %d - %s", statusID, name);
                    mappingInfo->status.addBaseName(statusID, name);
                }
                else
                {
                    log_->info("Specific status %d: %d - %s", fighterID, statusID, name);
                    mappingInfo->status.addSpecificName(fighterID, statusID, name);
                }
            } break;

            case MappingInfoStageKinds: {
                uint8_t stageID_l, stageID_h, len;
                char name[256];
                if (tcp_socket_read_exact(&socket, &stageID_h, 1) != 1) { log_->error("Failed to read stage_h"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, &stageID_l, 1) != 1) { log_->error("Failed to read stage_l"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, &len, 1) != 1) { log_->error("Failed to read stage len"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, name, len) != len) { log_->error("Failed to read stage name"); goto disconnect_error; }
                name[static_cast<int>(len)] = '\0';

                auto stageID = rfcommon::StageID::fromValue((stageID_h << 8) | (stageID_l << 0));

                // Something is weird
                if (mappingInfo.isNull())
                {
                    log_->error("MappingInfoStageKinds received even though it wasn't requested?");
                    goto disconnect_error;
                }

                log_->info("Stage ID: %d - %s", stageID, name);
                mappingInfo->stage.add(stageID, name);
            } break;

            case MappingInfoHitStatusKinds: {
                uint8_t status;
                uint8_t len;
                char name[256];
                if (tcp_socket_read_exact(&socket, &status, 1) != 1) { log_->error("Failed to read hit status ID"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, &len, 1) != 1) { log_->error("Failed to read hit status len"); goto disconnect_error; }
                if (tcp_socket_read_exact(&socket, name, len) != len) { log_->error("Failed to read hit status name"); goto disconnect_error; }
                name[static_cast<int>(len)] = '\0';

                // Something is weird
                if (mappingInfo.isNull())
                {
                    log_->error("MappingInfoHitStatusKinds received even though it wasn't requested?");
                    goto disconnect_error;
                }

                log_->info("Hit Status ID: %d - %s", status, name);
                mappingInfo->hitStatus.add(rfcommon::FighterHitStatus::fromValue(status), name);
            } break;

            case MappingInfoRequestComplete: {

                // Something is weird
                if (mappingInfo.isNull())
                {
                    log_->error("MappingInfoRequestComplete received even though it wasn't requested?");
                    goto disconnect_error;
                }

                log_->endDropdown();
                log_->notice("All mapping info received");
                emit mappingInfoReceived(mappingInfo.detach());
                return true;
            } break;

            case ProtocolVersion:
            case GameStart:
            case GameResume:
            case GameEnd:
            case TrainingStart:
            case TrainingResume:
            case TrainingReset:
            case TrainingEnd:
            case FighterState:
                break;
        }
    }

disconnect_error:
    log_->endDropdown();
    tcp_socket_shutdown(&socket);
    return false;
}

// ----------------------------------------------------------------------------
void ProtocolTask::handleProtocol(void* tcp_socket_handle)
{
    PROFILE(ProtocolTask, handleProtocol);

    tcp_socket socket = tcp_socket_from_handle(tcp_socket_handle);

    // Each fighter is assigned to a "slot" in the game, which means in a 1v1
    // it's possible that the fighters are referred to by slot 2 and slot 5.
    // We want to map this to a "fighter index" of 0 and 1.
    rfcommon::SmallVector<uint8_t, 8> fighterSlots;

    // If a game or training session is running, try to resume
    uint8_t buf[2] = {GameResume, TrainingResume};
    if (tcp_socket_write(&socket, buf, 2) != 2)
        goto disconnect;

    while (true)
    {
        mutex_.lock();
            if (requestShutdown_)
            {
                log_->info("Shutdown requested, closing socket");
                mutex_.unlock();
                goto disconnect;
            }
        mutex_.unlock();

        uint8_t msg;
        if (tcp_socket_read_exact(&socket, &msg, 1) != 1)
        {
            log_->error("Failed to read message type from socket, closing");
            goto disconnect;
        }

        switch (msg)
        {

            case TrainingStart:
            case TrainingResume: {
                log_->beginDropdown("Training Session");
                log_->info("Training %s", msg == TrainingStart ? "started" : "resumed");

                uint8_t buf[4];
                if (tcp_socket_read_exact(&socket, buf, 4) != 4)
                {
                    log_->error("Failed to read training start/resume data");
                    goto disconnect;
                }

#define stageH          buf[0]
#define stageL          buf[1]
#define playerFighterID buf[2]
#define cpuFighterID    buf[3]
                const auto stageID = rfcommon::StageID::fromValue((stageH << 8) | (stageL << 0));
                rfcommon::SmallVector<rfcommon::FighterID, 2> fighterIDs({
                    rfcommon::FighterID::fromValue(playerFighterID),
                    rfcommon::FighterID::fromValue(cpuFighterID)
                });
                rfcommon::SmallVector<rfcommon::String, 2> tags({"Player 1", "CPU"});

                log_->info("stageID: %d, player 1: %d, cpu: %d", stageID, playerFighterID, cpuFighterID);
#undef stageH
#undef stageL
#undef playerFighterID
#undef cpuFighterID
                // With training mode we always assume there are 2 fighters
                // with entry IDs 0 and 1
                fighterSlots.resize(2);
                fighterSlots[0] = 0;
                fighterSlots[1] = 1;

                rfcommon::MetaData* meta = rfcommon::MetaData::newActiveTrainingSession(
                        stageID,
                        std::move(fighterIDs),
                        std::move(tags));

                if (msg == TrainingStart)
                    emit trainingStarted(meta);
                else
                    emit trainingResumed(meta);
            } break;

            case TrainingEnd: {
                log_->info("Training ended");
                log_->endDropdown();
                emit trainingEnded();
            } break;

            case GameStart:
            case GameResume: {
                log_->beginDropdown("Game Session");
                log_->info("Game %s", msg == GameStart ? "started" : "resumed");

                uint8_t buf[3];
                char tag[256];
                if (tcp_socket_read_exact(&socket, buf, 3) != 3)
                {
                    log_->error("Failed to read game start/resume data");
                    goto disconnect;
                }

#define stageH buf[0]
#define stageL buf[1]
#define playerCount buf[2]
                const auto stageID = rfcommon::StageID::fromValue((stageH << 8) | (stageL << 0));
                auto fighterIDValues = rfcommon::SmallVector<uint8_t, 2>::makeResized(playerCount);
                auto tags = rfcommon::SmallVector<rfcommon::String, 2>::makeReserved(playerCount);

                log_->info("stageID: %d, player count: %d", stageID, playerCount);

                fighterSlots.resize(playerCount);
                if (tcp_socket_read_exact(&socket, fighterSlots.data(), playerCount) != playerCount)
                {
                    log_->error("Failed to read fighter slots");
                    goto disconnect;
                }
                for (int i = 0; i != fighterSlots.count(); ++i)
                    log_->info("idx %d -> slot %d", i, fighterSlots[i]);

                if (tcp_socket_read_exact(&socket, fighterIDValues.data(), playerCount) != playerCount)
                {
                    log_->error("Failed to read fighter ID values");
                    goto disconnect;
                }

                auto fighterIDs = rfcommon::SmallVector<rfcommon::FighterID, 2>::makeReserved(playerCount);
                for (int i = 0; i != fighterIDValues.count(); ++i)
                {
                    log_->info("idx %d: FighterID: %d", i, fighterIDValues[i]);
                    fighterIDs.push(rfcommon::FighterID::fromValue(fighterIDValues[i]));
                }

                for (int i = 0; i < playerCount; ++i)
                {
                    // TODO Tags on switch are stored as UTF-16. Have to update
                    // protocol at some point
                    uint8_t len;
                    if (tcp_socket_read_exact(&socket, &len, 1) != 1) { log_->error("Failed to player tag len"); goto disconnect; }
                    if (tcp_socket_read_exact(&socket, tag, len) != len) { log_->error("Failed to player tag string"); goto disconnect; }
                    tag[static_cast<int>(len)] = '\0';
                    tags.push(tag);

                    log_->info("idx %d: Tag: %s", i, tags[i].cStr());
                }
#undef stageH
#undef stageL
#undef playerCount

                rfcommon::MetaData* meta = rfcommon::MetaData::newActiveGameSession(
                        stageID,
                        std::move(fighterIDs),
                        std::move(tags));

                if (msg == GameStart)
                    emit gameStarted(meta);
                else
                    emit gameResumed(meta);
            } break;

            case GameEnd: {
                log_->info("Game ended");
                log_->endDropdown();
                emit gameEnded();
            } break;


            case FighterState: {
                uint8_t buf[29];
#define frame0 buf[0]
#define frame1 buf[1]
#define frame2 buf[2]
#define frame3 buf[3]
#define slotIdx buf[4]
#define posx0 buf[5]
#define posx1 buf[6]
#define posx2 buf[7]
#define posx3 buf[8]
#define posy0 buf[9]
#define posy1 buf[10]
#define posy2 buf[11]
#define posy3 buf[12]
#define damage0 buf[13]
#define damage1 buf[14]
#define hitstun0 buf[15]
#define hitstun1 buf[16]
#define shield0 buf[17]
#define shield1 buf[18]
#define status0 buf[19]
#define status1 buf[20]
#define motion0 buf[21]
#define motion1 buf[22]
#define motion2 buf[23]
#define motion3 buf[24]
#define motion4 buf[25]
#define hit_status buf[26]
#define stocks buf[27]
#define flags buf[28]

                /*
                 * server.broadcast(&[
                 *     MessageType::FighterState.into(),
                 *     frame0, frame1, frame2, frame3,
                 *     entry_id as u8,
                 *     posx0, posx1, posx2, posx3,
                 *     posy0, posy1, posy2, posy3,
                 *     damage0, damage1,
                 *     hitstun0, hitstun1,
                 *     shield0, shield1,
                 *     status0, status1,
                 *     motion0, motion1, motion2, motion3, motion4,
                 *     hit_status_status as u8,
                 *     stock_count,
                 *     flags
                 * ]);
                 */

                // Do this as early as possible to reduce latency as much as possible
                // (reading data might skew the time)
                quint64 frameTimeStamp = time_milli_seconds_since_epoch();

                if (tcp_socket_read_exact(&socket, buf, sizeof(buf)) != sizeof(buf))
                    break;

                quint32 frame = (frame0 << 24)
                            | (frame1 << 16)
                            | (frame2 << 8)
                            | (frame3 << 0);
                quint32 posx_le = ((quint32)posx0 << 24)
                                | ((quint32)posx1 << 16)
                                | ((quint32)posx2 << 8)
                                | ((quint32)posx3 << 0);
                quint32 posy_le = ((quint32)posy0 << 24)
                                | ((quint32)posy1 << 16)
                                | ((quint32)posy2 << 8)
                                | ((quint32)posy3 << 0);
                float posx = *reinterpret_cast<float*>(&posx_le);
                float posy = *reinterpret_cast<float*>(&posy_le);
                float damage = (((quint16)damage0 << 8)
                            | ((quint16)damage1 << 0)) / 50.0;
                float hitstun = (((quint16)hitstun0 << 8)
                            | ((quint16)hitstun1 << 0)) / 100.0;
                float shield = (((quint16)shield0 << 8)
                            | ((quint16)shield1 << 0)) / 200.0;
                quint16 status = (status0 << 8)
                            | (status1 << 0);
                quint64 motion = ((quint64)motion0 << 32)
                            | ((quint64)motion1 << 24)
                            | ((quint64)motion2 << 16)
                            | ((quint64)motion3 << 8)
                            | ((quint64)motion4 << 0);
                bool attack_connected = !!(flags & 0x01);
                bool facing_direction = !!(flags & 0x02);
                bool opponent_in_hitlag = !!(flags & 0x04);

                // Map the entry ID to an index
                for (int i = 0; i != fighterSlots.count(); ++i)
                    if (fighterSlots[i] == slotIdx)
                    {
                        emit fighterState(
                                    frameTimeStamp,
                                    frame,
                                    i,
                                    posx, posy,
                                    damage,
                                    hitstun,
                                    shield,
                                    status,
                                    motion,
                                    hit_status,
                                    stocks,
                                    attack_connected,
                                    facing_direction,
                                    opponent_in_hitlag);
                        goto successfully_mapped_to_slot;
                    }
                log_->warning("Received fighter state for slot %d, but this slot is not mapped!", slotIdx);
                successfully_mapped_to_slot:;
#undef frame0
#undef frame1
#undef frame2
#undef frame3
#undef slotIdx
#undef posx0
#undef posx1
#undef posx2
#undef posx3
#undef posy0
#undef posy1
#undef posy2
#undef posy3
#undef hitstun0
#undef hitstun1
#undef damage0
#undef damage1
#undef shield0
#undef shield1
#undef status0
#undef status1
#undef motion0
#undef motion1
#undef motion2
#undef motion3
#undef motion4
#undef hit_status
#undef stocks
#undef flags
            }
        }
    }

disconnect:
    log_->info("socket shutdown");
    log_->endDropdown();
    tcp_socket_shutdown(&socket);
}

}
