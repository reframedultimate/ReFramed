#include "uh/models/Protocol.hpp"
#include "uh/models/PlayerState.hpp"

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
    , mapping_(new MappingInfo)
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

    emit connectionClosed();
}

// ----------------------------------------------------------------------------
void Protocol::run()
{
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
            continue;

        if (msg == FighterKinds)
        {
            uint8_t fighterID, len;
            char name[256];
            if (tcp_socket_read(&socket_, &fighterID, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &len, 1) != 1) continue;
            if (tcp_socket_read(&socket_, name, len) != len) continue;
            name[(int)len] = '\0';

            mapping_->fighterID.add(fighterID, name);
        }
        else if (msg == StageKinds)
        {
            uint8_t stageID_l, stageID_h, len;
            char name[256];
            if (tcp_socket_read(&socket_, &stageID_h, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &stageID_l, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &len, 1) != 1) continue;
            if (tcp_socket_read(&socket_, name, len) != len) continue;
            name[(int)len] = '\0';

            uint16_t stageID = (stageID_h << 8)
                             | (stageID_l << 0);
            mapping_->stageID.add(stageID, name);
        }
        else if (msg == MatchStart)
        {
            uint8_t stageID_l, stageID_h, playerCount;
            if (tcp_socket_read(&socket_, &stageID_h, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &stageID_l, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &playerCount, 1) != 1) continue;

            uint16_t stageID = (stageID_h << 8)
                             | (stageID_l << 0);

            const QString* stageNamePtr = mapping_->stageID.map(stageID);
            QString stageName = stageNamePtr ? *stageNamePtr : "(Unknown Stage)";

            QVector<uint8_t> entryIDs(playerCount);
            QVector<uint8_t> fighterIDs(playerCount);
            QVector<QString> tags(playerCount);
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

            recording_ = new Recording(*mapping_);
            recording_->setGameInfo(GameInfo(stageID, QDateTime::currentDateTime()));
            emit dateChanged(recording_->gameInfo().timeStarted());
            emit stageChanged(stageName);

            emit playerCountChanged(playerCount);
            for (int i = 0; i < playerCount; ++i)
            {
                recording_->addPlayer(PlayerInfo(tags[i], fighterIDs[i], entryIDs[i]));

                const QString* fighterNamePtr = recording_->mappingInfo().fighterID.map(fighterIDs[i]);
                QString fighterName = fighterNamePtr ? *fighterNamePtr : "(Unknown character)";
                emit playerTagChanged(i, tags[i]);
                emit playerFighterChanged(i, fighterName);
            }

            emit matchStarted();
            fail:;
        }
        else if (msg == MatchEnd)
        {
            emit matchEnded(recording_);
        }
        else if (msg == FighterState)
        {
            uint8_t f0, f1, f2, f3, entryID, stocks, status_l, status_h, damage_l, damage_h;
            if (tcp_socket_read(&socket_, &f0, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &f1, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &f2, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &f3, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &entryID, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &stocks, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &status_h, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &status_l, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &damage_h, 1) != 1) continue;
            if (tcp_socket_read(&socket_, &damage_l, 1) != 1) continue;

            uint32_t frame = (f0 << 24)
                           | (f1 << 16)
                           | (f2 << 8)
                           | (f3 << 0);
            uint16_t status = (status_h << 8)
                            | (status_l << 0);
            float damage = (((uint16_t)damage_h << 8) | ((uint16_t)damage_l << 0)) * 0.01;

            // Map the entry ID to an index
            for (int i = 0; i != recording_->playerCount(); ++i)
                if (recording_->playerInfo(i).entryID() == entryID)
                {
                    recording_->addPlayerState(i, PlayerState(frame, status, damage, stocks));
                    emit playerStatusChanged(frame, i, status);
                    emit playerDamageChanged(frame, i, damage);
                    emit playerStockCountChanged(frame, i, stocks);
                    break;
                }
        }
    }
}

}
