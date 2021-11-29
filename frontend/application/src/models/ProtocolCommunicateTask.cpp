#include "application/models/ProtocolCommunicateTask.hpp"
#include "uh/PlayerState.hpp"
#include "uh/RunningGameSession.hpp"
#include "uh/RunningTrainingSession.hpp"
#include "uh/time.h"
#include <QDebug>

namespace uhapp {

// ----------------------------------------------------------------------------
ProtocolCommunicateTask::ProtocolCommunicateTask(tcp_socket socket, QObject* parent)
    : QThread(parent)
    , socket_(socket)
    , requestShutdown_(false)
{
}

// ----------------------------------------------------------------------------
ProtocolCommunicateTask::~ProtocolCommunicateTask()
{
    tcp_socket_shutdown(&socket_);

    mutex_.lock();
        requestShutdown_ = true;
    mutex_.unlock();
    wait();

    tcp_socket_close(&socket_);
}

// ----------------------------------------------------------------------------
void ProtocolCommunicateTask::run()
{
    uh::SmallVector<uint8_t, 8> entryIDs;
    uh::MappingInfo mappingInfo;

    while (true)
    {
        mutex_.lock();
            if (requestShutdown_)
            {
                mutex_.unlock();
                break;
            }
        mutex_.unlock();

        uint8_t msg;
        if (tcp_socket_read(&socket_, &msg, 1) != 1)
            break;

        if (msg == Version)
        {
            uint8_t version;
            if (tcp_socket_read(&socket_, &version, 1) != 1) break;
        }
        else if (msg == TrainingStart)
        {
            uint8_t buf[5];
            char tag[256];
#define stageH          buf[0]
#define stageL          buf[1]
#define playerFighterID buf[2]
#define cpuFighterID    buf[3]
#define playerTagLen    buf[4]

            if (tcp_socket_read(&socket_, buf, 5) != 5) break;
            if (tcp_socket_read(&socket_, tag, playerTagLen) != playerTagLen) break;
            tag[static_cast<int>(playerTagLen)] = '\0';

            static_assert(sizeof(uh::StageID) == 2);
            uh::StageID stageID = (stageH << 8)
                                | (stageL << 0);

            uh::SmallVector<uh::FighterID, 8> fighterIDs({playerFighterID, cpuFighterID});
            uh::SmallVector<uh::SmallString<15>, 8> tags({tag, ""});

            emit trainingStarted(new uh::RunningTrainingSession(
                uh::MappingInfo(mappingInfo),
                stageID,
                std::move(fighterIDs),
                std::move(tags)
            ));
#undef stageH
#undef stageL
#undef playerFighterID
#undef cpuFighterID
#undef playerTagLen
        }
        else if (msg == TrainingEnd)
        {
            qDebug() << "Traininig ended";
            emit trainingEnded();
        }
        else if (msg == TrainingReset)
        {
            qDebug() << "Traininig reset";
            emit trainingReset();
        }
        else if (msg == MatchStart)
        {
            uint8_t buf[3];
            char tag[256];
#define stageH buf[0]
#define stageL buf[1]
#define playerCount buf[2]

            if (tcp_socket_read(&socket_, buf, 3) != 3)
                break;

            static_assert(sizeof(uh::StageID) == 2);
            uh::StageID stageID = (stageH << 8) | (stageL << 0);

            uh::SmallVector<uh::FighterID, 8> fighterIDs(playerCount);
            uh::SmallVector<uh::SmallString<15>, 8> tags(playerCount);
            uh::SmallVector<uh::SmallString<15>, 8> names(playerCount);

            entryIDs.resize(playerCount);
            if (tcp_socket_read(&socket_, entryIDs.data(), playerCount) != playerCount)
                break;

            static_assert(sizeof(uh::FighterID) == 1);
            if (tcp_socket_read(&socket_, fighterIDs.data(), playerCount) != playerCount)
                break;

            for (int i = 0; i < playerCount; ++i)
            {
                // TODO Tags on switch are stored as UTF-16. Have to update
                // protocol at some point
                uint8_t len;
                if (tcp_socket_read(&socket_, &len, 1) != 1) goto fail;
                if (tcp_socket_read(&socket_, tag, len) != len) goto fail;
                tag[static_cast<int>(len)] = '\0';
                tags[i] = tag;
                names[i] = tag;
            } fail: break;

            qDebug() << "Match start: Stage: " << stageID << ", players: " << playerCount;

            emit matchStarted(new uh::RunningGameSession(
                uh::MappingInfo(mappingInfo),
                stageID,
                std::move(fighterIDs),
                std::move(tags),
                std::move(names)
            ));
#undef stageH
#undef stageL
#undef playerCount
        }
        else if (msg == MatchEnd)
        {
            qDebug() << "Match end";
            emit matchEnded();
        }
        else if (msg == FighterKinds)
        {
            uh::FighterID fighterID;
            uint8_t len;
            char name[256];
            static_assert(sizeof(uh::FighterID) == 1);
            if (tcp_socket_read(&socket_, &fighterID, 1) != 1) break;
            if (tcp_socket_read(&socket_, &len, 1) != 1) break;
            if (tcp_socket_read(&socket_, name, len) != len) break;
            name[static_cast<int>(len)] = '\0';

            mappingInfo.fighterID.add(fighterID, name);
            qDebug() << "fighter kind: " << fighterID <<": " << name;
        }
        else if (msg == FighterStatusKinds)
        {
            uh::FighterID fighterID;
            uint8_t statusID_l, statusID_h, len;
            char name[256];
            static_assert(sizeof(uh::FighterID) == 1);
            if (tcp_socket_read(&socket_, &fighterID, 1) != 1) break;
            if (tcp_socket_read(&socket_, &statusID_h, 1) != 1) break;
            if (tcp_socket_read(&socket_, &statusID_l, 1) != 1) break;
            if (tcp_socket_read(&socket_, &len, 1) != 1) break;
            if (tcp_socket_read(&socket_, name, len) != len) break;
            name[static_cast<int>(len)] = '\0';

            static_assert(sizeof(uh::FighterStatus) == 2);
            uh::FighterStatus statusID = (statusID_h << 8)
                                       | (statusID_l << 0);

            if (fighterID == 255)
            {
                mappingInfo.fighterStatus.addBaseEnumName(statusID, name);
                qDebug() << "base status: " << statusID <<": " << name;
            }
            else
            {
                mappingInfo.fighterStatus.addFighterSpecificEnumName(statusID, fighterID, name);
                qDebug() << "specific status: " << statusID <<": " << name;
            }
        }
        else if (msg == StageKinds)
        {
            uint8_t stageID_l, stageID_h, len;
            char name[256];
            if (tcp_socket_read(&socket_, &stageID_h, 1) != 1) break;
            if (tcp_socket_read(&socket_, &stageID_l, 1) != 1) break;
            if (tcp_socket_read(&socket_, &len, 1) != 1) break;
            if (tcp_socket_read(&socket_, name, len) != len) break;
            name[static_cast<int>(len)] = '\0';

            static_assert(sizeof(uh::StageID) == 2);
            uh::StageID stageID = (stageID_h << 8)
                                | (stageID_l << 0);

            mappingInfo.stageID.add(stageID, name);
            qDebug() << "stage kind: " << stageID <<": " << name;
        }
        else if (msg == HitStatusKinds)
        {
            uh::FighterHitStatus status;
            uint8_t len;
            char name[256];
            static_assert(sizeof(uh::FighterHitStatus) == 1);
            if (tcp_socket_read(&socket_, &status, 1) != 1) break;
            if (tcp_socket_read(&socket_, &len, 1) != 1) break;
            if (tcp_socket_read(&socket_, name, len) != len) break;
            name[static_cast<int>(len)] = '\0';

            mappingInfo.hitStatus.add(status, name);
            qDebug() << "hit status: " << status <<": " << name;

        }
        else if (msg == FighterState)
        {
            uint8_t buf[29];
#define frame0 buf[0]
#define frame1 buf[1]
#define frame2 buf[2]
#define frame3 buf[3]
#define entryID buf[4]
#define posx0 buf[5]
#define posx1 buf[6]
#define posx2 buf[7]
#define posx3 buf[8]
#define posy0 buf[9]
#define posy1 buf[10]
#define posy2 buf[11]
#define posy3 buf[12]
#define hitstun0 buf[13]
#define hitstun1 buf[14]
#define damage0 buf[15]
#define damage1 buf[16]
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

            if (tcp_socket_read(&socket_, buf, sizeof(buf)) != sizeof(buf))
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

            // Map the entry ID to an index
            for (int i = 0; i != entryIDs.count(); ++i)
                if (entryIDs[i] == entryID)
                {
                    emit playerState(frameTimeStamp, frame, i, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction);
                    break;
                }
#undef frame0
#undef frame1
#undef frame2
#undef frame3
#undef entryID
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

    tcp_socket_shutdown(&socket_);
    emit connectionClosed();
}

}
