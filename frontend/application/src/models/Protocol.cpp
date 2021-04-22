#include "application/models/Protocol.hpp"
#include "uh/PlayerState.hpp"
#include "uh/ActiveRecording.hpp"
#include "uh/time.h"

#include <QDebug>

namespace uhapp {

enum MessageType
{
    MatchStart,
    MatchEnd,
    FighterState,
    FighterKinds,
    FighterStatusKinds,
    StageKinds,
    HitStatusKinds
};

// ----------------------------------------------------------------------------
Protocol::Protocol(tcp_socket socket, QObject* parent)
    : QThread(parent)
    , socket_(socket)
{
    // These signals are used to transfer data from the listener thread to the
    // main thread
    connect(this, &Protocol::_receiveMatchStarted,
            this, &Protocol::onReceiveMatchStarted);
    connect(this, &Protocol::_receivePlayerState,
            this, &Protocol::onReceivePlayerState);
    connect(this, &Protocol::_receiveMatchEnded,
            this, &Protocol::onReceiveMatchEnded);
}

// ----------------------------------------------------------------------------
Protocol::~Protocol()
{
    tcp_socket_shutdown(&socket_);

    mutex_.lock();
        requestShutdown_ = true;
    mutex_.unlock();
    wait();

    tcp_socket_close(&socket_);

    if (recording_)
        emit recordingEnded(recording_);
}

// ----------------------------------------------------------------------------
void Protocol::run()
{
    QVector<uint8_t> entryIDs;
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

        if (msg == FighterKinds)
        {
            uh::FighterID fighterID;
            uint8_t len;
            char name[256];
            static_assert(sizeof(uh::FighterID) == 1);
            if (tcp_socket_read(&socket_, &fighterID, 1) != 1) break;
            if (tcp_socket_read(&socket_, &len, 1) != 1) break;
            if (tcp_socket_read(&socket_, name, len) != len) break;
            name[(int)len] = '\0';

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
            name[(int)len] = '\0';

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
            name[(int)len] = '\0';

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
            name[(int)len] = '\0';

            mappingInfo.hitStatus.add(status, name);
            qDebug() << "hit status: " << status <<": " << name;

        }
        else if (msg == MatchStart)
        {
            uint8_t stageID_l, stageID_h, playerCount;
            if (tcp_socket_read(&socket_, &stageID_h, 1) != 1) break;
            if (tcp_socket_read(&socket_, &stageID_l, 1) != 1) break;
            if (tcp_socket_read(&socket_, &playerCount, 1) != 1) break;

            static_assert(sizeof(uh::StageID) == 2);
            uh::StageID stageID = (stageID_h << 8)
                                | (stageID_l << 0);

            std::vector<uh::FighterID> fighterIDs(playerCount);
            std::vector<std::string> tags(playerCount);
            entryIDs.resize(playerCount);
            for (int i = 0; i < playerCount; ++i)
            {
                uint8_t entryID;
                if (tcp_socket_read(&socket_, &entryID, 1) != 1) goto fail;
                entryIDs[i] = entryID;
            }
            for (int i = 0; i < playerCount; ++i)
            {
                uh::FighterID fighterID;
                static_assert(sizeof(uh::FighterID) == 1);
                if (tcp_socket_read(&socket_, &fighterID, 1) != 1) goto fail;
                fighterIDs[i] = fighterID;
            }
            for (int i = 0; i < playerCount; ++i)
            {
                // TODO Tags on switch are stored as UTF-16. Have to update
                // protocol at some point
                uint8_t len;
                char tag[256];
                if (tcp_socket_read(&socket_, &len, 1) != 1) goto fail;
                if (tcp_socket_read(&socket_, tag, len) != len) goto fail;
                tag[(int)len] = 0;
                tags[i] = tag;
            }

            qDebug() << "Match start: Stage: " << stageID << ", players: " << playerCount;

            emit _receiveMatchStarted(new uh::ActiveRecording(
                uh::MappingInfo(mappingInfo),
                std::move(fighterIDs),
                std::move(tags),
                stageID
            ));
            continue;

            fail: break;
        }
        else if (msg == MatchEnd)
        {
            qDebug() << "Match end";
            emit _receiveMatchEnded();
        }
        else if (msg == FighterState)
        {
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
            uint64_t frameTimeStamp = time_milli_seconds_since_epoch();

            uint8_t frame_[4], entryID, posx_[4], posy_[4], hitstun_[2];
            uint8_t damage_[2], shield_[2], status_[2], motion_[5], hit_status;
            uint8_t stocks, flags;
            if (tcp_socket_read(&socket_, &frame_[0], 4) != 4) break;
            if (tcp_socket_read(&socket_, &entryID, 1) != 1) break;
            if (tcp_socket_read(&socket_, &posx_[0], 4) != 4) break;
            if (tcp_socket_read(&socket_, &posy_[0], 4) != 4) break;
            if (tcp_socket_read(&socket_, &damage_[0], 2) != 2) break;
            if (tcp_socket_read(&socket_, &hitstun_[0], 2) != 2) break;
            if (tcp_socket_read(&socket_, &shield_[0], 2) != 2) break;
            if (tcp_socket_read(&socket_, &status_[0], 2) != 2) break;
            if (tcp_socket_read(&socket_, &motion_[0], 5) != 5) break;
            if (tcp_socket_read(&socket_, &hit_status, 1) != 1) break;
            if (tcp_socket_read(&socket_, &stocks, 1) != 1) break;
            if (tcp_socket_read(&socket_, &flags, 1) != 1) break;

            quint32 frame = (frame_[0] << 24)
                          | (frame_[1] << 16)
                          | (frame_[2] << 8)
                          | (frame_[3] << 0);
            quint32 posx_le = ((quint32)posx_[0] << 24)
                            | ((quint32)posx_[1] << 16)
                            | ((quint32)posx_[2] << 8)
                            | ((quint32)posx_[3] << 0);
            quint32 posy_le = ((quint32)posy_[0] << 24)
                            | ((quint32)posy_[1] << 16)
                            | ((quint32)posy_[2] << 8)
                            | ((quint32)posy_[3] << 0);
            float posx = *reinterpret_cast<float*>(&posx_le);
            float posy = *reinterpret_cast<float*>(&posy_le);
            float damage = (((quint16)damage_[0] << 8)
                          | ((quint16)damage_[1] << 0)) / 50.0;
            float hitstun = (((quint16)hitstun_[0] << 8)
                          | ((quint16)hitstun_[1] << 0)) / 100.0;
            float shield = (((quint16)shield_[0] << 8)
                          | ((quint16)shield_[1] << 0)) / 200.0;
            quint16 status = (status_[0] << 8)
                           | (status_[1] << 0);
            quint64 motion = ((quint64)motion_[0] << 32)
                           | ((quint64)motion_[1] << 24)
                           | ((quint64)motion_[2] << 16)
                           | ((quint64)motion_[3] << 8)
                           | ((quint64)motion_[4] << 0);
            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            qDebug() << "motion: " << motion;

            // Map the entry ID to an index
            for (int i = 0; i != entryIDs.size(); ++i)
                if (entryIDs[i] == entryID)
                {
                    emit _receivePlayerState(frameTimeStamp, frame, i, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction);
                    break;
                }
        }
    }

    tcp_socket_shutdown(&socket_);
    emit serverClosedConnection();
}

// ----------------------------------------------------------------------------
void Protocol::onReceiveMatchStarted(uh::ActiveRecording* recording)
{
    // Handle case where match end is not sent (should never happen but you never know)
    if (recording_.notNull())
        emit recordingEnded(recording_);

    recording_ = recording;
    emit recordingStarted(recording_);
}

// ----------------------------------------------------------------------------
void Protocol::onReceivePlayerState(
        uint64_t frameTimeStamp,
        quint32 frame,
        quint8 playerID,
        float posx,
        float posy,
        float damage,
        float hitstun,
        float shield,
        quint16 status,
        quint64 motion,
        quint8 hit_status,
        quint8 stocks,
        bool attack_connected,
        bool facing_direction)
{
    if (recording_.isNull())
        return;

    recording_->addPlayerState(playerID, uh::PlayerState(frameTimeStamp, frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
}

// ----------------------------------------------------------------------------
void Protocol::onReceiveMatchEnded()
{
    if (recording_.isNull())
        return;

    emit recordingEnded(recording_);
    recording_.reset();
}

}
