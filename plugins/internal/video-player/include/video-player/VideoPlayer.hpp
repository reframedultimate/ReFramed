#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/String.hpp"
#include <QWidget>

extern "C" {
typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodec AVCodec;
typedef struct AVCodecParameters AVCodecParameters;
typedef struct AVStream AVStream;
}

class QPlainTextEdit;

class VideoPlayer
    : public QWidget
    , public rfcommon::RealtimePlugin
{
    Q_OBJECT

public:
    explicit VideoPlayer(RFPluginFactory* factory, QWidget* parent=nullptr);
    ~VideoPlayer();

    bool openFile(const QString& fileName);

private:
    void info(const QString& msg);
    void error(const QString& msg);

private:
    QWidget* createView() override { return this; }
    void destroyView(QWidget* view) override { (void)view; }

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::Session* training) override;
    void onProtocolTrainingResumed(rfcommon::Session* training) override;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override;
    void onProtocolTrainingEnded(rfcommon::Session* training) override;
    void onProtocolGameStarted(rfcommon::Session* game) override;
    void onProtocolGameResumed(rfcommon::Session* game) override;
    void onProtocolGameEnded(rfcommon::Session* game) override;

private:
    void onGameSessionLoaded(rfcommon::Session* game) override;
    void onGameSessionUnloaded(rfcommon::Session* game) override;
    void onTrainingSessionLoaded(rfcommon::Session* training) override;
    void onTrainingSessionUnloaded(rfcommon::Session* training) override;

    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override;
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override;

private:
    QPlainTextEdit* logWidget_;
    AVFormatContext* formatContext_ = nullptr;
};
