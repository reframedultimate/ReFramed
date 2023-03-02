#pragma once

#include "rfcommon/Reference.hpp"

#include <QThread>

namespace rfcommon {
    class Log;
    class Plugin;
    class Session;
}

namespace rfapp {

class PluginManager;

class AutoAssociateVideoTask : public QThread
{
    Q_OBJECT

public:
    explicit AutoAssociateVideoTask(
            rfcommon::Session* session,
            const QString& vidDir,
            int frameOffsetCorrection,
            PluginManager* pluginManager,
            rfcommon::Log* log,
            QObject* parent=nullptr);
    ~AutoAssociateVideoTask();

signals:
    void success();
    void failure();

protected:
    void run() override;

private:
    PluginManager* pluginManager_;
    rfcommon::Plugin* videoPlugin_;
    rfcommon::Log* log_;
    rfcommon::Reference<rfcommon::Session> session_;
    QString vidDir_;
    const int frameCorrection_;
};

}
