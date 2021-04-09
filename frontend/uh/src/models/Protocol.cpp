#include "uh/models/Protocol.hpp"
#include "uh/models/PlayerState.hpp"
#include "uh/models/ActiveRecording.hpp"

#include <QDebug>

namespace uh {

enum MessageType
{
    FighterKinds,
    FighterStatusKinds,
    StageKinds,
    MatchStart,
    MatchEnd,
    FighterState
};

// ----------------------------------------------------------------------------
Protocol::Protocol(tcp_socket socket, QObject* parent)
    : QThread(parent)
    , socket_(socket)
{
    // These signals are used to transfer data from the listener thread to the
    // main thread
    connect(this, SIGNAL(_receiveMatchStarted(ActiveRecording*)),
            this, SLOT(onReceiveMatchStarted(ActiveRecording*)));
    connect(this, SIGNAL(_receivePlayerState(unsigned int, int, unsigned int, float, unsigned int)),
            this, SLOT(onReceivePlayerState(unsigned int, int, unsigned int, float, unsigned int)));
    connect(this, SIGNAL(_receiveMatchEnded()),
            this, SLOT(onReceiveMatchEnded()));
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
        emit recordingEnded(recording_.data());
}

// ----------------------------------------------------------------------------
void Protocol::run()
{
    QVector<uint8_t> entryIDs;
    MappingInfo mappingInfo;

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
            uint8_t fighterID, len;
            char name[256];
            if (tcp_socket_read(&socket_, &fighterID, 1) != 1) break;
            if (tcp_socket_read(&socket_, &len, 1) != 1) break;
            if (tcp_socket_read(&socket_, name, len) != len) break;
            name[(int)len] = '\0';

            mappingInfo.fighterID.add(fighterID, name);
        }
        else if (msg == FighterStatusKinds)
        {
            uint8_t fighterID, statusID_l, statusID_h, len;
            char name[256];
            if (tcp_socket_read(&socket_, &fighterID, 1) != 1) break;
            if (tcp_socket_read(&socket_, &statusID_h, 1) != 1) break;
            if (tcp_socket_read(&socket_, &statusID_l, 1) != 1) break;
            if (tcp_socket_read(&socket_, &len, 1) != 1) break;
            if (tcp_socket_read(&socket_, name, len) != len) break;
            name[(int)len] = '\0';

            uint16_t statusID = (statusID_h << 8)
                              | (statusID_l << 0);

            if (fighterID == 255)
                mappingInfo.fighterStatus.addBaseEnumName(statusID, name);
            else
                mappingInfo.fighterStatus.addFighterSpecificEnumName(statusID, fighterID, name);
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

            uint16_t stageID = (stageID_h << 8)
                             | (stageID_l << 0);
            mappingInfo.stageID.add(stageID, name);
        }
        else if (msg == MatchStart)
        {
            uint8_t stageID_l, stageID_h, playerCount;
            if (tcp_socket_read(&socket_, &stageID_h, 1) != 1) break;
            if (tcp_socket_read(&socket_, &stageID_l, 1) != 1) break;
            if (tcp_socket_read(&socket_, &playerCount, 1) != 1) break;

            uint16_t stageID = (stageID_h << 8)
                             | (stageID_l << 0);

            QVector<uint8_t> fighterIDs(playerCount);
            QVector<QString> tags(playerCount);
            entryIDs.resize(playerCount);
            for (int i = 0; i < playerCount; ++i)
            {
                uint8_t id;
                if (tcp_socket_read(&socket_, &id, 1) != 1) goto fail;
                entryIDs[i] = id;
            }
            for (int i = 0; i < playerCount; ++i)
            {
                uint8_t id;
                if (tcp_socket_read(&socket_, &id, 1) != 1) goto fail;
                fighterIDs[i] = id;
            }
            for (int i = 0; i < playerCount; ++i)
            {
                uint8_t len;
                char tag[256];
                if (tcp_socket_read(&socket_, &len, 1) != 1) goto fail;
                if (tcp_socket_read(&socket_, tag, len) != len) goto fail;
                tag[(int)len] = '\0';
                tags[i] = tag;
            }

            emit _receiveMatchStarted(new ActiveRecording(
                MappingInfo(mappingInfo),
                std::move(fighterIDs),
                std::move(tags),
                stageID
            ));
            continue;

            fail: break;
        }
        else if (msg == MatchEnd)
        {
            emit _receiveMatchEnded();
        }
        else if (msg == FighterState)
        {
            uint8_t f0, f1, f2, f3, entryID, stocks, status_l, status_h, damage_l, damage_h;
            if (tcp_socket_read(&socket_, &f0, 1) != 1) break;
            if (tcp_socket_read(&socket_, &f1, 1) != 1) break;
            if (tcp_socket_read(&socket_, &f2, 1) != 1) break;
            if (tcp_socket_read(&socket_, &f3, 1) != 1) break;
            if (tcp_socket_read(&socket_, &entryID, 1) != 1) break;
            if (tcp_socket_read(&socket_, &stocks, 1) != 1) break;
            if (tcp_socket_read(&socket_, &status_h, 1) != 1) break;
            if (tcp_socket_read(&socket_, &status_l, 1) != 1) break;
            if (tcp_socket_read(&socket_, &damage_h, 1) != 1) break;
            if (tcp_socket_read(&socket_, &damage_l, 1) != 1) break;

            uint32_t frame = (f0 << 24)
                           | (f1 << 16)
                           | (f2 << 8)
                           | (f3 << 0);
            uint16_t status = (status_h << 8)
                            | (status_l << 0);
            float damage = (((uint16_t)damage_h << 8) | ((uint16_t)damage_l << 0)) * 0.01;

            // Map the entry ID to an index
            for (int i = 0; i != entryIDs.size(); ++i)
                if (entryIDs[i] == entryID)
                {
                    emit _receivePlayerState(frame, i, status, damage, stocks);
                    break;
                }
        }
    }

    tcp_socket_shutdown(&socket_);
    emit serverClosedConnection();
}

// ----------------------------------------------------------------------------
void Protocol::onReceiveMatchStarted(ActiveRecording* recording)
{
    // Handle case where match end is not sent (should never happen but you never know)
    if (recording_ != nullptr)
        emit recordingEnded(recording_.data());

    recording_ = recording;
    emit recordingStarted(recording_.data());
}

// ----------------------------------------------------------------------------
void Protocol::onReceivePlayerState(unsigned int frame, int playerID, unsigned int status, float damage, unsigned int stocks)
{
    if (recording_ == nullptr)
        return;

    recording_->addPlayerState(playerID, PlayerState(frame, status, damage, stocks));
}

// ----------------------------------------------------------------------------
void Protocol::onReceiveMatchEnded()
{
    if (recording_ == nullptr)
        return;

    emit recordingEnded(recording_.data());
    recording_.reset();
}

}
