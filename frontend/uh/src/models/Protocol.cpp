#include "uh/models/Protocol.hpp"
#include "uh/models/PlayerState.hpp"

#include "QDebug"

namespace uh {

enum MessageType
{
    FighterKinds,
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
}

// ----------------------------------------------------------------------------
QSharedDataPointer<Recording> Protocol::takeRecording()
{
    QSharedDataPointer<Recording> result;
    mutex_.lock();
    result = recording_;
    recording_ = nullptr;
    mutex_.unlock();
    return result;
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

            const QString* stageNamePtr = mappingInfo.stageID.map(stageID);
            QString stageName = stageNamePtr ? *stageNamePtr : "(Unknown Stage)";

            GameInfo gameInfo(stageID, QDateTime::currentDateTime());
            gameInfo.setFormat(GameInfo::FRIENDLIES);  // TODO: Hardcode this for now, should be set according to the UI later

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

            mutex_.lock();
            recording_ = new Recording(mappingInfo);
            recording_->setGameInfo(gameInfo);
            emit dateChanged(recording_->gameInfo().timeStarted());
            emit stageChanged(stageName);

            emit playerCountChanged(playerCount);
            for (int i = 0; i < playerCount; ++i)
            {
                recording_->addPlayer(PlayerInfo(tags[i], fighterIDs[i]));
            }
            mutex_.unlock();

            for (int i = 0; i < playerCount; ++i)
            {
                const QString* fighterNamePtr = mappingInfo.fighterID.map(fighterIDs[i]);
                QString fighterName = fighterNamePtr ? *fighterNamePtr : "(Unknown character)";
                emit playerTagChanged(i, tags[i]);
                emit playerFighterChanged(i, fighterName);
            }

            emit matchStarted();
            continue;

            fail: break;
        }
        else if (msg == MatchEnd)
        {
            emit matchEnded();
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
            mutex_.lock();
            for (int i = 0; i != recording_->playerCount(); ++i)
                if (entryIDs[i] == entryID)
                {
                    recording_->addPlayerState(i, PlayerState(frame, status, damage, stocks));
                    mutex_.unlock();
                    emit playerStatusChanged(frame, i, status);
                    emit playerDamageChanged(frame, i, damage);
                    emit playerStockCountChanged(frame, i, stocks);
                    goto noUnlock;
                }
            mutex_.unlock();

            noUnlock:;
        }
    }

    tcp_socket_shutdown(&socket_);
    emit connectionClosed();
}

}
